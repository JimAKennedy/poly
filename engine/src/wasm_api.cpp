#include "poly/wasm_api.h"

#include <algorithm>
#include <array>
#include <cstdint>

#include "poly/constraint.h"
#include "poly/engine.h"
#include "poly/euclidean.h"
#include "poly/macro.h"
#include "poly/presets.h"
#include "poly/sanitize.h"
#include "poly/scene.h"
#include "poly/types.h"

namespace {

static constexpr int kFieldsPerEvent = 6;

struct Context {
    poly::Engine engine;
    poly::SceneState scenes{};
    poly::NoteEventBuffer eventBuffer{};
    std::array<double, poly::kMaxEventsPerBlock * kFieldsPerEvent> flatEvents{};
    // Scratch buffer used by readState() to hand out an interpolated GrooveState
    // view under SceneSelect::Morph. Owning it on the Context keeps returned
    // pointers (e.g. poly_lane_micro_timing_ptr) valid for the JS caller's
    // synchronous read window.
    poly::GrooveState interpBuf{};

    // Edits always target A or B directly (Morph selection routes to A) —
    // matches the VST plugin's behavior (processor.cpp:646).
    poly::GrooveState& state() { return scenes.select == poly::SceneSelect::B ? scenes.sceneB : scenes.sceneA; }
    const poly::GrooveState& state() const {
        return scenes.select == poly::SceneSelect::B ? scenes.sceneB : scenes.sceneA;
    }

    // Reads: under SceneSelect::Morph, materialize the interpolated snapshot of
    // A and B into interpBuf and return it. Otherwise route to the selected
    // scene. This makes every read accessor (lane fields, macros, patterns,
    // envelopes, seed) respect Morph, so the JS host's syncStateFromWasm() sees
    // the same interpolated lanes that poly_render emits — fixing the M044 S06
    // bug where the modal's audio path picked samples by scene A's lane notes
    // regardless of morph amount. Returns non-const because the pointer
    // accessors (micro-timing, accents, cells) publish raw float*/int* into
    // the buffer; JS never writes back through them.
    poly::GrooveState& readState() {
        if (scenes.select == poly::SceneSelect::Morph) {
            interpBuf = poly::interpolateGrooveState(scenes.sceneA, scenes.sceneB, scenes.morphAmount);
            return interpBuf;
        }
        return state();
    }
};

poly::SceneSelect sceneFromInt(int v) {
    switch (v) {
    case 1:
        return poly::SceneSelect::B;
    case 2:
        return poly::SceneSelect::Morph;
    default:
        return poly::SceneSelect::A;
    }
}

enum class LaneFieldInt {
    MidiNote,
    MidiChannel,
    HitCount,
    Rotation,
    BaseVelocity,
    GhostFloor,
    Active,
    Subdivision,
    CycleSteps,
    KotekanSource,
    Timeline,
    EnvelopeCount,
    CellCount,
};

enum class LaneFieldFloat {
    Probability,
    VelocitySpread,
    HumanizeMs,
    SwingAmount,
    NoteDuration,
    PhraseLength,
    PhraseGap,
    PhraseOffset,
    MutationRate,
    DriftRate,
    TimingOffsetMs,
    SyncopationOffset,
    TempoMultiplier,
    EmphasisProb,
};

} // namespace

extern "C" {

PolyContext poly_create() {
    auto* c = new Context(); // ownership-transfer
    // Both scenes seeded to the default init preset. Independent so that a
    // preset applied to one scene never spills into the other. Selection
    // starts at A to match legacy single-scene behavior.
    c->scenes.sceneA = poly::makeFactoryPreset(0);
    c->scenes.sceneB = poly::makeFactoryPreset(0);
    c->scenes.select = poly::SceneSelect::A;
    return c;
}

void poly_destroy(PolyContext ctx) {
    delete static_cast<Context*>(ctx);
}

int poly_preset_count() {
    return poly::kFactoryPresetCount;
}

const char* poly_preset_name(int index) {
    if (index < 0 || index >= poly::kFactoryPresetCount)
        return "";
    return poly::getFactoryPresetInfo(index).name;
}

const char* poly_preset_description(int index) {
    if (index < 0 || index >= poly::kFactoryPresetCount)
        return "";
    return poly::getFactoryPresetInfo(index).description;
}

void poly_load_preset(PolyContext ctx, int index) {
    auto* c = static_cast<Context*>(ctx);
    if (index < 0 || index >= poly::kFactoryPresetCount)
        return;
    c->state() = poly::makeFactoryPreset(index);
}

int poly_render(PolyContext ctx, double ppqStart, double ppqEnd, double tempo, double sampleRate, int blockSize,
                int playing, int looping, double loopStartPpq, double loopEndPpq, int jumped) {
    auto* c = static_cast<Context*>(ctx);
    c->eventBuffer.clear();

    poly::TransportContext tc{};
    tc.ppqStart = ppqStart;
    tc.ppqEnd = ppqEnd;
    tc.tempo = tempo;
    tc.sampleRate = sampleRate;
    tc.blockSize = blockSize;
    tc.playing = (playing != 0);
    tc.looping = (looping != 0);
    tc.jumped = (jumped != 0);
    tc.loopStartPpq = loopStartPpq;
    tc.loopEndPpq = loopEndPpq;

    // M043 S14 T02: when Morph is selected, render an interpolated snapshot of
    // both scenes rather than falling back to scene A. Mirrors the plugin's
    // process() path (processor.cpp:425-431) so the audible playback and the
    // eventual VST render agree on what Morph means.
    // M043 S15 T01: resolve macros + constraints before rendering, matching
    // processor.cpp:439. Preset-baked macros (complexity/density/etc.) only
    // affect output through resolveMacros; without this, moving a macro
    // slider or loading a preset with non-default macro values changes
    // nothing on the site's Play or Try It surfaces.
    // readState() already handles the Morph interpolation; poly_render used to
    // duplicate the branch, kept in sync manually. Route through readState() so
    // there is a single source of truth.
    const poly::GrooveState& base = c->readState();
    poly::GrooveState resolved = poly::resolveConstraints(base, poly::resolveMacros(base));
    c->engine.renderRange(tc, resolved, c->eventBuffer);

    for (size_t i = 0; i < c->eventBuffer.count; ++i) {
        const auto& e = c->eventBuffer.events[i];
        size_t base = i * kFieldsPerEvent;
        c->flatEvents[base + 0] = e.ppqPosition;
        c->flatEvents[base + 1] = static_cast<double>(e.pitch);
        c->flatEvents[base + 2] = static_cast<double>(e.velocity);
        c->flatEvents[base + 3] = e.duration;
        c->flatEvents[base + 4] = static_cast<double>(e.channel);
        c->flatEvents[base + 5] = static_cast<double>(e.laneIndex);
    }

    return static_cast<int>(c->eventBuffer.count);
}

int poly_event_count(PolyContext ctx) {
    return static_cast<int>(static_cast<Context*>(ctx)->eventBuffer.count);
}

double* poly_event_buffer(PolyContext ctx) {
    return static_cast<Context*>(ctx)->flatEvents.data();
}

int poly_active_lane_count(PolyContext ctx) {
    return static_cast<Context*>(ctx)->readState().activeLaneCount;
}

double poly_macro_value(PolyContext ctx, int index) {
    auto* c = static_cast<Context*>(ctx);
    const auto& s = c->readState();
    switch (index) {
    case 0:
        return static_cast<double>(s.macros.complexity);
    case 1:
        return static_cast<double>(s.macros.density);
    case 2:
        return static_cast<double>(s.macros.syncopation);
    case 3:
        return static_cast<double>(s.macros.swing);
    case 4:
        return static_cast<double>(s.macros.tension);
    case 5:
        return static_cast<double>(s.macros.humanize);
    default:
        return 0.0;
    }
}

void poly_set_macro(PolyContext ctx, int index, double value) {
    auto* c = static_cast<Context*>(ctx);
    auto v = static_cast<float>(value);
    switch (index) {
    case 0:
        c->state().macros.complexity = v;
        break;
    case 1:
        c->state().macros.density = v;
        break;
    case 2:
        c->state().macros.syncopation = v;
        break;
    case 3:
        c->state().macros.swing = v;
        break;
    case 4:
        c->state().macros.tension = v;
        break;
    case 5:
        c->state().macros.humanize = v;
        break;
    }
}

void poly_edit_lane_int(PolyContext ctx, int lane, int field, int value) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state().lanes[static_cast<size_t>(lane)];

    switch (static_cast<LaneFieldInt>(field)) {
    case LaneFieldInt::MidiNote:
        cfg.midiNote = static_cast<int16_t>(std::clamp(value, 0, 127));
        break;
    case LaneFieldInt::MidiChannel:
        cfg.midiChannel = static_cast<int16_t>(std::clamp(value, -1, 15));
        break;
    case LaneFieldInt::HitCount:
        cfg.hitCount = std::clamp(value, 0, cfg.cycle.steps);
        break;
    case LaneFieldInt::Rotation:
        cfg.rotation = ((value % cfg.cycle.steps) + cfg.cycle.steps) % cfg.cycle.steps;
        break;
    case LaneFieldInt::BaseVelocity:
        cfg.baseVelocity = static_cast<uint8_t>(std::clamp(value, 0, 127));
        break;
    case LaneFieldInt::GhostFloor:
        cfg.ghostFloor = static_cast<uint8_t>(std::clamp(value, 0, 127));
        break;
    case LaneFieldInt::Active:
        cfg.active = (value != 0);
        break;
    case LaneFieldInt::Subdivision:
        cfg.cycle.subdivision = std::clamp(value, 1, 16);
        break;
    case LaneFieldInt::CycleSteps:
        cfg.cycle.steps = std::clamp(value, 1, poly::kMaxSteps);
        cfg.hitCount = std::min(cfg.hitCount, cfg.cycle.steps);
        break;
    case LaneFieldInt::KotekanSource:
        cfg.kotekanSourceLane = std::clamp(value, -1, poly::kMaxLanes - 1);
        break;
    case LaneFieldInt::Timeline:
        cfg.timeline = (value != 0);
        break;
    case LaneFieldInt::EnvelopeCount:
        cfg.envelopeCount = std::clamp(value, 0, poly::kMaxEnvelopesPerLane);
        break;
    case LaneFieldInt::CellCount:
        cfg.cellCount = std::clamp(value, 0, poly::kMaxSteps);
        break;
    }
    poly::sanitizeGrooveState(c->state());
}

void poly_edit_lane_float(PolyContext ctx, int lane, int field, double value) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state().lanes[static_cast<size_t>(lane)];
    auto v = static_cast<float>(value);

    switch (static_cast<LaneFieldFloat>(field)) {
    case LaneFieldFloat::Probability:
        cfg.probability = std::clamp(v, 0.0f, 1.0f);
        break;
    case LaneFieldFloat::VelocitySpread:
        cfg.velocitySpread = std::clamp(v, 0.0f, 1.0f);
        break;
    case LaneFieldFloat::HumanizeMs:
        cfg.humanizeMs = std::clamp(v, 0.0f, 50.0f);
        break;
    case LaneFieldFloat::SwingAmount:
        cfg.swingAmount = std::clamp(v, 0.0f, 1.0f);
        break;
    case LaneFieldFloat::NoteDuration:
        cfg.noteDuration = std::clamp(v, 0.0f, 4.0f);
        break;
    case LaneFieldFloat::PhraseLength:
        cfg.phraseLength = std::clamp(v, 0.0f, 64.0f);
        break;
    case LaneFieldFloat::PhraseGap:
        cfg.phraseGap = std::clamp(v, 0.0f, 64.0f);
        break;
    case LaneFieldFloat::PhraseOffset:
        cfg.phraseOffset = std::clamp(v, 0.0f, 64.0f);
        break;
    case LaneFieldFloat::MutationRate:
        cfg.mutationRate = std::clamp(v, 0.0f, 1.0f);
        break;
    case LaneFieldFloat::DriftRate:
        cfg.driftRate = std::clamp(v, -4.0f, 4.0f);
        break;
    case LaneFieldFloat::TimingOffsetMs:
        cfg.timingOffsetMs = std::clamp(v, -20.0f, 20.0f);
        break;
    case LaneFieldFloat::SyncopationOffset:
        cfg.syncopationOffset = std::clamp(v, 0.0f, 1.0f);
        break;
    case LaneFieldFloat::TempoMultiplier:
        cfg.tempoMultiplier = std::clamp(v, 0.25f, 4.0f);
        break;
    case LaneFieldFloat::EmphasisProb:
        cfg.emphasisProb = std::clamp(v, 0.0f, 1.0f);
        break;
    }
    poly::sanitizeGrooveState(c->state());
}

int poly_lane_int(PolyContext ctx, int lane, int field) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return 0;
    const auto& cfg = c->readState().lanes[static_cast<size_t>(lane)];

    switch (static_cast<LaneFieldInt>(field)) {
    case LaneFieldInt::MidiNote:
        return cfg.midiNote;
    case LaneFieldInt::MidiChannel:
        return cfg.midiChannel;
    case LaneFieldInt::HitCount:
        return cfg.hitCount;
    case LaneFieldInt::Rotation:
        return cfg.rotation;
    case LaneFieldInt::BaseVelocity:
        return cfg.baseVelocity;
    case LaneFieldInt::GhostFloor:
        return cfg.ghostFloor;
    case LaneFieldInt::Active:
        return cfg.active ? 1 : 0;
    case LaneFieldInt::Subdivision:
        return cfg.cycle.subdivision;
    case LaneFieldInt::CycleSteps:
        return cfg.cycle.steps;
    case LaneFieldInt::KotekanSource:
        return cfg.kotekanSourceLane;
    case LaneFieldInt::Timeline:
        return cfg.timeline ? 1 : 0;
    case LaneFieldInt::EnvelopeCount:
        return cfg.envelopeCount;
    case LaneFieldInt::CellCount:
        return cfg.cellCount;
    }
    return 0;
}

double poly_lane_float(PolyContext ctx, int lane, int field) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return 0.0;
    const auto& cfg = c->readState().lanes[static_cast<size_t>(lane)];

    switch (static_cast<LaneFieldFloat>(field)) {
    case LaneFieldFloat::Probability:
        return static_cast<double>(cfg.probability);
    case LaneFieldFloat::VelocitySpread:
        return static_cast<double>(cfg.velocitySpread);
    case LaneFieldFloat::HumanizeMs:
        return static_cast<double>(cfg.humanizeMs);
    case LaneFieldFloat::SwingAmount:
        return static_cast<double>(cfg.swingAmount);
    case LaneFieldFloat::NoteDuration:
        return static_cast<double>(cfg.noteDuration);
    case LaneFieldFloat::PhraseLength:
        return static_cast<double>(cfg.phraseLength);
    case LaneFieldFloat::PhraseGap:
        return static_cast<double>(cfg.phraseGap);
    case LaneFieldFloat::PhraseOffset:
        return static_cast<double>(cfg.phraseOffset);
    case LaneFieldFloat::MutationRate:
        return static_cast<double>(cfg.mutationRate);
    case LaneFieldFloat::DriftRate:
        return static_cast<double>(cfg.driftRate);
    case LaneFieldFloat::TimingOffsetMs:
        return static_cast<double>(cfg.timingOffsetMs);
    case LaneFieldFloat::SyncopationOffset:
        return static_cast<double>(cfg.syncopationOffset);
    case LaneFieldFloat::TempoMultiplier:
        return static_cast<double>(cfg.tempoMultiplier);
    case LaneFieldFloat::EmphasisProb:
        return static_cast<double>(cfg.emphasisProb);
    }
    return 0.0;
}

int poly_lane_pattern(PolyContext ctx, int lane, int* out, int maxLen) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes || !out || maxLen <= 0)
        return 0;
    const auto& cfg = c->readState().lanes[static_cast<size_t>(lane)];
    std::array<bool, poly::kMaxSteps> pattern{};
    poly::euclidean(cfg.hitCount, cfg.cycle.steps, cfg.rotation, pattern);
    int len = std::min(cfg.cycle.steps, maxLen);
    for (int i = 0; i < len; ++i)
        out[i] = pattern[static_cast<size_t>(i)] ? 1 : 0;
    return len;
}

float* poly_lane_micro_timing_ptr(PolyContext ctx, int lane) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return nullptr;
    return c->readState().lanes[static_cast<size_t>(lane)].microTimingMs.data();
}

float* poly_lane_accents_ptr(PolyContext ctx, int lane) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return nullptr;
    return c->readState().lanes[static_cast<size_t>(lane)].accents.steps.data();
}

int* poly_lane_cells_ptr(PolyContext ctx, int lane) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return nullptr;
    return c->readState().lanes[static_cast<size_t>(lane)].cellSizes.data();
}

int poly_lane_fixed_pattern(PolyContext ctx, int lane, int* out, int maxLen) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes || !out || maxLen <= 0)
        return 0;
    const auto& cfg = c->readState().lanes[static_cast<size_t>(lane)];
    int len = cfg.fixedPatternLength > 0 ? cfg.fixedPatternLength : cfg.cycle.steps;
    len = std::min(len, maxLen);
    for (int i = 0; i < len; ++i)
        out[i] = cfg.fixedPattern[static_cast<size_t>(i)] ? 1 : 0;
    return len;
}

int poly_lane_envelope_count(PolyContext ctx, int lane) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return 0;
    return c->readState().lanes[static_cast<size_t>(lane)].envelopeCount;
}

int poly_lane_envelope_active(PolyContext ctx, int lane, int index) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes || index < 0 || index >= poly::kMaxEnvelopesPerLane)
        return 0;
    return c->readState().lanes[static_cast<size_t>(lane)].envelopes[static_cast<size_t>(index)].active ? 1 : 0;
}

int poly_lane_envelope_target(PolyContext ctx, int lane, int index) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes || index < 0 || index >= poly::kMaxEnvelopesPerLane)
        return 0;
    return static_cast<int>(
        c->readState().lanes[static_cast<size_t>(lane)].envelopes[static_cast<size_t>(index)].envelope.target);
}

float poly_lane_envelope_period(PolyContext ctx, int lane, int index) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes || index < 0 || index >= poly::kMaxEnvelopesPerLane)
        return 0.0f;
    return c->readState().lanes[static_cast<size_t>(lane)].envelopes[static_cast<size_t>(index)].envelope.periodBars;
}

float poly_lane_envelope_depth(PolyContext ctx, int lane, int index) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes || index < 0 || index >= poly::kMaxEnvelopesPerLane)
        return 0.0f;
    return c->readState().lanes[static_cast<size_t>(lane)].envelopes[static_cast<size_t>(index)].envelope.depth;
}

void poly_action_set_euclid(PolyContext ctx, int lane, int steps, int hits, int rotation) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state().lanes[static_cast<size_t>(lane)];
    if (steps > 0)
        cfg.cycle.steps = std::clamp(steps, 1, poly::kMaxSteps);
    if (hits >= 0)
        cfg.hitCount = std::clamp(hits, 0, cfg.cycle.steps);
    cfg.rotation = ((rotation % cfg.cycle.steps) + cfg.cycle.steps) % cfg.cycle.steps;
    poly::sanitizeGrooveState(c->state());
}

void poly_action_toggle_step(PolyContext ctx, int lane, int step) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state().lanes[static_cast<size_t>(lane)];
    if (!cfg.timeline || step < 0 || step >= poly::kMaxSteps)
        return;
    cfg.fixedPattern[static_cast<size_t>(step)] = !cfg.fixedPattern[static_cast<size_t>(step)];
}

void poly_action_apply_preset(PolyContext ctx, int index) {
    poly_load_preset(ctx, index);
}

// M043 S14 T01: scene-aware preset apply. Writes the preset to the requested
// scene (0 = A, 1 = B) without disturbing the other scene or the current
// selection. `targetScene` values other than 0/1 fall back to the currently
// selected scene, and Morph selection is treated as A (Morph is a render-time
// blend, not a writable scene).
void poly_action_apply_preset_to_scene(PolyContext ctx, int index, int targetScene) {
    auto* c = static_cast<Context*>(ctx);
    if (index < 0 || index >= poly::kFactoryPresetCount)
        return;
    poly::GrooveState& dest = (targetScene == 1)   ? c->scenes.sceneB
                              : (targetScene == 0) ? c->scenes.sceneA
                                                   : c->state();
    dest = poly::makeFactoryPreset(index);
}

// M043 S14 T01: JS-side scene selection reaches the engine so subsequent
// edits, renders and preset loads target the correct scene. Accepts the
// SceneSelect ordinal (0 = A, 1 = B, 2 = Morph).
void poly_action_select_scene(PolyContext ctx, int sceneSelect) {
    auto* c = static_cast<Context*>(ctx);
    c->scenes.select = sceneFromInt(sceneSelect);
}

int poly_scene_select(PolyContext ctx) {
    return static_cast<int>(static_cast<Context*>(ctx)->scenes.select);
}

// M043 S14 T02: morph amount push/pull. The plugin exposes this as
// kSceneMorph (processor.cpp:484); the web bridge routes `scene.morph` edits
// here so Morph-mode playback and dumps interpolate against the current UI
// value, not a stale default.
void poly_set_morph(PolyContext ctx, double amount) {
    auto* c = static_cast<Context*>(ctx);
    c->scenes.morphAmount = std::clamp(static_cast<float>(amount), 0.0f, 1.0f);
}

double poly_morph_amount(PolyContext ctx) {
    return static_cast<double>(static_cast<Context*>(ctx)->scenes.morphAmount);
}

// M043 S14 T02: snapshot the full scene state (both grooves + select + morph
// amount + noteMap + chain config) from src into dst. Used by the JS host to
// seed the Play scratch context from engineCtx so playback renders both
// scenes under Morph while keeping its own fresh Engine RNG state (Engine
// lives on Context, not on scenes, so a fresh Context always starts
// deterministically). Chain runtime state stays with each Context's own
// Engine and is not copied here.
void poly_copy_scenes(PolyContext dst, PolyContext src) {
    if (!dst || !src)
        return;
    auto* d = static_cast<Context*>(dst);
    auto* s = static_cast<Context*>(src);
    d->scenes = s->scenes;
}

void poly_action_set_fixed_step(PolyContext ctx, int lane, int step, int on) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state().lanes[static_cast<size_t>(lane)];
    if (step < 0 || step >= poly::kMaxSteps)
        return;
    cfg.fixedPattern[static_cast<size_t>(step)] = (on != 0);
}

void poly_action_set_micro_timing(PolyContext ctx, int lane, int step, float ms) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state().lanes[static_cast<size_t>(lane)];
    if (step < 0 || step >= poly::kMaxSteps)
        return;
    cfg.microTimingMs[static_cast<size_t>(step)] = std::clamp(ms, -20.0f, 20.0f);
}

void poly_action_set_accent(PolyContext ctx, int lane, int step, float value) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state().lanes[static_cast<size_t>(lane)];
    if (step < 0 || step >= poly::kMaxSteps)
        return;
    cfg.accents.steps[static_cast<size_t>(step)] = std::clamp(value, 0.0f, 1.0f);
}

void poly_action_set_cells(PolyContext ctx, int lane, const int* cells, int count) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state().lanes[static_cast<size_t>(lane)];
    cfg.cellCount = std::clamp(count, 0, poly::kMaxSteps);
    for (int i = 0; i < cfg.cellCount; ++i)
        cfg.cellSizes[static_cast<size_t>(i)] = std::clamp(cells[i], 1, 4);
    poly::sanitizeGrooveState(c->state());
}

void poly_action_clear_cells(PolyContext ctx, int lane) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    c->state().lanes[static_cast<size_t>(lane)].cellCount = 0;
    poly::sanitizeGrooveState(c->state());
}

void poly_action_set_envelope(PolyContext ctx, int lane, int index, int target, float period, float depth, int active) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state().lanes[static_cast<size_t>(lane)];
    if (index < 0 || index >= poly::kMaxEnvelopesPerLane)
        return;
    auto& ea = cfg.envelopes[static_cast<size_t>(index)];
    ea.envelope.target = static_cast<poly::EnvTarget>(std::clamp(target, 0, 7));
    ea.envelope.periodBars = std::max(0.25f, period);
    ea.envelope.depth = std::clamp(depth, 0.0f, 1.0f);
    ea.active = (active != 0);
    if (index >= cfg.envelopeCount)
        cfg.envelopeCount = index + 1;
}

void poly_action_remove_envelope(PolyContext ctx, int lane, int index) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state().lanes[static_cast<size_t>(lane)];
    if (index < 0 || index >= cfg.envelopeCount)
        return;
    for (int i = index; i < cfg.envelopeCount - 1; ++i)
        cfg.envelopes[static_cast<size_t>(i)] = cfg.envelopes[static_cast<size_t>(i + 1)];
    cfg.envelopes[static_cast<size_t>(cfg.envelopeCount - 1)] = {};
    cfg.envelopeCount--;
}

uint64_t poly_seed(PolyContext ctx) {
    return static_cast<Context*>(ctx)->readState().seed;
}

void poly_set_seed(PolyContext ctx, uint64_t seed) {
    static_cast<Context*>(ctx)->state().seed = seed;
}

} // extern "C"
