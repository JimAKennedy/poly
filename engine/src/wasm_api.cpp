#include "poly/wasm_api.h"

#include <algorithm>
#include <array>
#include <cstdint>

#include "poly/engine.h"
#include "poly/euclidean.h"
#include "poly/presets.h"
#include "poly/sanitize.h"
#include "poly/types.h"

namespace {

static constexpr int kFieldsPerEvent = 6;

struct Context {
    poly::Engine engine;
    poly::GrooveState state{};
    poly::NoteEventBuffer eventBuffer{};
    std::array<double, poly::kMaxEventsPerBlock * kFieldsPerEvent> flatEvents{};
};

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
    auto* c = new Context();
    c->state = poly::makeFactoryPreset(0);
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
    c->state = poly::makeFactoryPreset(index);
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

    c->engine.renderRange(tc, c->state, c->eventBuffer);

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
    return static_cast<Context*>(ctx)->state.activeLaneCount;
}

double poly_macro_value(PolyContext ctx, int index) {
    auto* c = static_cast<Context*>(ctx);
    switch (index) {
    case 0:
        return static_cast<double>(c->state.macros.complexity);
    case 1:
        return static_cast<double>(c->state.macros.density);
    case 2:
        return static_cast<double>(c->state.macros.syncopation);
    case 3:
        return static_cast<double>(c->state.macros.swing);
    case 4:
        return static_cast<double>(c->state.macros.tension);
    case 5:
        return static_cast<double>(c->state.macros.humanize);
    default:
        return 0.0;
    }
}

void poly_set_macro(PolyContext ctx, int index, double value) {
    auto* c = static_cast<Context*>(ctx);
    auto v = static_cast<float>(value);
    switch (index) {
    case 0:
        c->state.macros.complexity = v;
        break;
    case 1:
        c->state.macros.density = v;
        break;
    case 2:
        c->state.macros.syncopation = v;
        break;
    case 3:
        c->state.macros.swing = v;
        break;
    case 4:
        c->state.macros.tension = v;
        break;
    case 5:
        c->state.macros.humanize = v;
        break;
    }
}

void poly_edit_lane_int(PolyContext ctx, int lane, int field, int value) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state.lanes[static_cast<size_t>(lane)];

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
    poly::sanitizeGrooveState(c->state);
}

void poly_edit_lane_float(PolyContext ctx, int lane, int field, double value) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state.lanes[static_cast<size_t>(lane)];
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
    poly::sanitizeGrooveState(c->state);
}

void poly_action_set_euclid(PolyContext ctx, int lane, int steps, int hits, int rotation) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state.lanes[static_cast<size_t>(lane)];
    if (steps > 0)
        cfg.cycle.steps = std::clamp(steps, 1, poly::kMaxSteps);
    if (hits >= 0)
        cfg.hitCount = std::clamp(hits, 0, cfg.cycle.steps);
    cfg.rotation = ((rotation % cfg.cycle.steps) + cfg.cycle.steps) % cfg.cycle.steps;
    poly::sanitizeGrooveState(c->state);
}

void poly_action_toggle_step(PolyContext ctx, int lane, int step) {
    auto* c = static_cast<Context*>(ctx);
    if (lane < 0 || lane >= poly::kMaxLanes)
        return;
    auto& cfg = c->state.lanes[static_cast<size_t>(lane)];
    if (!cfg.timeline || step < 0 || step >= poly::kMaxSteps)
        return;
    cfg.fixedPattern[static_cast<size_t>(step)] = !cfg.fixedPattern[static_cast<size_t>(step)];
}

void poly_action_apply_preset(PolyContext ctx, int index) {
    poly_load_preset(ctx, index);
}

uint64_t poly_seed(PolyContext ctx) {
    return static_cast<Context*>(ctx)->state.seed;
}

void poly_set_seed(PolyContext ctx, uint64_t seed) {
    static_cast<Context*>(ctx)->state.seed = seed;
}

} // extern "C"
