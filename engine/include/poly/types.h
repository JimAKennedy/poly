// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace poly {

// --- Constants ---

static constexpr int kMaxLanes = 8;
static constexpr int kMaxSteps = 64;
static constexpr int kMaxEnvelopesPerLane = 4;
static constexpr int kMaxGlobalEnvelopes = 8;
static constexpr size_t kMaxEventsPerBlock = 256;

// --- Transport ---

// region:transport-context
struct TransportContext {
    double ppqStart = 0.0;
    double ppqEnd = 0.0;
    double tempo = 120.0;
    double sampleRate = 44100.0;
    int32_t blockSize = 512;
    bool playing = false;
    bool looping = false;
    bool jumped = false;
    // M046 S06 P9: true when this block is the first block after a natural loop
    // wrap (playhead snapped from ~loopEndPpq back to ~loopStartPpq while
    // looping). Callers that clear per-jump state (capture buffer, scene chain,
    // macro smoother) should gate on !wrappedLoop to preserve continuity across
    // repeats. Pending note-offs still flush unconditionally to avoid stuck notes.
    bool wrappedLoop = false;
    double loopStartPpq = 0.0;
    double loopEndPpq = 0.0;
    // M051 S02 E6: host time signature drives bar-unit math where "bar" is a
    // user-facing count — scene chain "advance every N bars," envelope
    // `periodBars`, MIDI capture "last N bars." Lane cycles remain
    // meter-independent: a steps=7/subdivision=8 lane always cycles every 7/8,
    // regardless of host meter (subdivision is notes-per-bar, and 4.0 PPQ stays
    // the reference bar for step math). Plugin populates from
    // Vst::ProcessContext::timeSigNumerator/Denominator under kTimeSigValid;
    // defaults to 4/4 for engine tests and hosts that don't publish it.
    int16_t timeSigNumerator = 4;
    int16_t timeSigDenominator = 4;

    // PPQ per bar = numerator * (4 / denominator). Poly's PPQ unit is one
    // quarter note, so a 7/8 bar is 7 * (4/8) = 3.5 PPQ.
    double ppqPerBar() const {
        return static_cast<double>(timeSigNumerator) * (4.0 / static_cast<double>(timeSigDenominator));
    }
};
// endregion:transport-context

// --- Note Output ---

// region:note-event
struct NoteEvent {
    double ppqPosition = 0.0;
    int16_t pitch = 0;
    float velocity = 0.0f;
    double duration = 0.0;
    int16_t channel = 0;
    int16_t laneIndex = 0;
};
// endregion:note-event

struct NoteEventBuffer {
    std::array<NoteEvent, kMaxEventsPerBlock> events{};
    size_t count = 0;
    size_t droppedCount = 0;

    void clear() {
        count = 0;
        droppedCount = 0;
    }

    bool push(const NoteEvent& e) {
        if (count >= kMaxEventsPerBlock) {
            ++droppedCount;
            return false;
        }
        events[count++] = e;
        return true;
    }
};

// --- Emission Classification (M045 S01 T01) ---
//
// The desk grid historically displayed the base Euclidean pattern only, so
// mutation-added, mutation-dropped, ghost-velocity, and probability-culled
// hits were invisible — the observed "Jazz Bop Ride snare is all over the
// place" bug. renderRange optionally emits one EmissionEvent per considered
// step so the UI can composite the truth on top of the base pattern.
// Base:  emitted, on-pattern, unmodified velocity
// Ghost: emitted, on-pattern, mutation floored velocity
// Add:   emitted, off-pattern position (mutation-added or fill-added)
// Drop:  NOT emitted, on-pattern (mutation drop or probability/activation cull)
// Silent (off-pattern non-hit) is not recorded — nothing to display.
// M002 S01 (EC06). The interlock style a kotekan pair uses. Defined by what the
// engine computes, not by appeal to the tradition: `src` is the source lane's
// resolved pattern, and the derived part is
//   NyogCag  pattern[s] = !src[s]        -- the strict complement
//   Telu     pattern[s] = !src[s % 3]    -- repeats on a three-pulse cell
//   Empat    pattern[s] = !src[s % 4]    -- repeats on a four-pulse cell
// The cell modes are then filled wherever neither part would strike, because
// theory-gamelan Rule 1 makes gaps in the composite errors and a periodic
// complement of an unrelated source does not preserve continuity on its own.
// Only the rhythmic dimension of telu and empat is modelled; the terms also
// carry pitch meaning, which theory-gamelan.mdx discloses and sources.
enum class KotekanMode : uint8_t {
    NyogCag = 0,
    Telu = 1,
    Empat = 2,
};

enum class EmissionKind : uint8_t {
    Base = 0,
    Ghost = 1,
    Add = 2,
    Drop = 3,
};

struct EmissionEvent {
    double ppqPosition = 0.0;
    // Post-timing-shift onset (swing, syncopation, per-step micro-timing,
    // humanize, and the lane timing offset applied) — the ppq the audible note
    // actually fires at. Equals ppqPosition for Drop emissions, which never
    // schedule a note and so have no shifted onset. Consumed by the desk
    // "played" timeline so a syncopated/delayed hit is shown where it sounds,
    // not where its grid step sits.
    double shiftedPpqPosition = 0.0;
    int16_t cycleStep = 0;
    int16_t laneIndex = 0;
    uint8_t kind = 0; // EmissionKind
};

static constexpr size_t kMaxEmissionsPerBlock = 512;

struct EmissionEventBuffer {
    std::array<EmissionEvent, kMaxEmissionsPerBlock> events{};
    size_t count = 0;
    size_t droppedCount = 0;

    void clear() {
        count = 0;
        droppedCount = 0;
    }

    bool push(const EmissionEvent& e) {
        if (count >= kMaxEmissionsPerBlock) {
            ++droppedCount;
            return false;
        }
        events[count++] = e;
        return true;
    }
};

// --- Lane Model ---

enum class Role : uint8_t { AnchorPulse, Backbeat, Shimmer, Accent, Ghost, Ornament, Fill, Custom };

struct Cycle {
    int steps = 4;
    int subdivision = 4; // 1=whole, 2=half, 4=quarter, 8=eighth, 16=sixteenth
};

struct AccentMask {
    std::array<float, kMaxSteps> steps{};
};

// --- Envelopes ---

enum class EnvTarget : uint8_t {
    Velocity,
    Density,
    Probability,
    AccentBias,
    NoteLength,
    TimingLooseness,
    ActivationWeight,
    FillLikelihood
};

enum class Shape : uint8_t { Ramp, Sine, Triangle, Curve, StepList };

static constexpr int kMaxStepListEntries = 16;

struct Envelope {
    EnvTarget target = EnvTarget::Velocity;
    float periodBars = 4.0f;
    Shape shape = Shape::Sine;
    float depth = 1.0f;
    float phaseOffset = 0.0f;
    float curvature = 0.0f;
    std::array<float, kMaxStepListEntries> stepValues{};
    int stepCount = 0;
};

struct EnvelopeAssign {
    Envelope envelope{};
    bool active = true;
};

// --- Constraints ---

struct ConstraintConfig {
    AccentMask anchorSteps{};
    bool backbeatProtect = false;
    int densityMin = 0;
    int densityMax = kMaxSteps;
};

// --- Lane Config ---

// region:lane-config
struct LaneConfig {
    int id = 0;
    Role role = Role::Custom;
    int16_t midiNote = 36;
    int16_t midiChannel = -1; // -1 = auto (lane index); 0-15 = explicit MIDI channel
    Cycle cycle{};
    int hitCount = 4;
    int rotation = 0;
    float probability = 1.0f;
    uint8_t baseVelocity = 100;
    AccentMask accents{};
    float emphasisProb = 0.5f;
    uint8_t ghostFloor = 30;
    float velocitySpread = 0.05f;
    float humanizeMs = 0.0f;
    float swingAmount = 0.0f;
    float noteDuration = 0.0f;
    float phraseLength = 0.0f;      // beats; 0 = continuous (no phrase gating)
    float phraseGap = 0.0f;         // beats; silence between phrases
    float phraseOffset = 0.0f;      // beats; phase offset for this lane's phrase cycle
    float mutationRate = 0.0f;      // 0.0-1.0; per-step mutation probability each cycle
    float driftRate = 0.0f;         // steps per bar; pattern rotation rate from absolute PPQ
    float timingOffsetMs = 0.0f;    // ms; positive = late, negative = early; range [-20, +20]
    float syncopationOffset = 0.0f; // 0.0-1.0; pushes even (strong-beat) steps late
    float tempoMultiplier = 1.0f;   // 0.25-4.0; per-lane tempo scaling (Nancarrow-style)
    int kotekanSourceLane = -1;     // -1=independent, 0-7=complement of source lane's pattern
    // M002 S01 (EC06). The interlock style, and how many structural points the
    // pair strikes together. Both are state-only: the per-lane VST3 parameter
    // family is full (kParamsPerLane == 16, kKotekanSource occupies slot 15),
    // and a style choice does not want an automation lane. Defaults reproduce
    // the pre-M002 strict complement exactly.
    KotekanMode kotekanMode = KotekanMode::NyogCag;
    int kotekanOverlap = 0;                 // structural steps struck by both parts; 0 = strict
    int fillEveryNBars = 0;                 // 0 = no bar-gated fill; N>0 = play off-pattern fill on bars whose
                                            // absolute bar index is a multiple of N (deterministic, PPQ-derived)
    int cellCount = 0;                      // 0 = equal cells (standard Euclidean); >0 = additive/aksak
    std::array<int, kMaxSteps> cellSizes{}; // subdivision units per cell; sum = total cycle length
    // M003 S01 (EC08). Non-isochronous subdivision: each entry is a step's
    // duration as a multiple of the base step. profileCount == 0 takes the
    // existing branch untouched, so every pre-M003 patch is byte-identical.
    // The profile is normalised so the cycle keeps the length it would have had
    // evenly -- it states distribution, never length -- and takes precedence
    // over cellSizes, which are structure rather than feel.
    std::array<float, kMaxSteps> subdivisionProfile{};
    int profileCount = 0;
    // M003 S02 (EC09). A grouping over the lane's existing steps, for feel.
    // Distinct from cellCount/cellSizes, which replace the steps with one per
    // cell: a lane with swingCellSizes {2,2,3} still has seven steps, and swing
    // displaces within each cell rather than across the bar. 0 = no grouping,
    // which leaves every swung lane keying off (cycleStep % 2) as before.
    int swingCellCount = 0;
    std::array<int, kMaxSteps> swingCellSizes{};
    bool timeline = false;                      // timeline mode: use fixedPattern, immune to macros
    std::array<bool, kMaxSteps> fixedPattern{}; // per-step on/off for timeline mode
    // timeline mode pattern length: 0 = use cycle.steps; >0 = explicit length that governs both editable slot count
    // AND playback cycle wrap (see prepareLaneContext in engine.cpp; enforced M049 S02 / E2 fix).
    int fixedPatternLength = 0;
    std::array<float, kMaxSteps> microTimingMs{}; // per-step timing offset in ms; range [-20, +20]
    // M034 S03: per-lane seed lock. laneSeed is the preserved RNG seed a locked
    // lane derives from; seedLocked pins it so a global reroll (GrooveState::seed
    // change) leaves this lane's output byte-identical while other lanes re-roll.
    // The WebUI captures the current global seed into laneSeed when the user locks
    // the lane. Defaults (seedLocked=false, laneSeed=0) make laneEffectiveSeed
    // return the global seed unchanged, so pre-change output is byte-identical.
    uint64_t laneSeed = 0;
    bool seedLocked = false;
    bool active = true;
    std::array<EnvelopeAssign, kMaxEnvelopesPerLane> envelopes{};
    int envelopeCount = 0;
    ConstraintConfig constraints{};
};
// endregion:lane-config

// Effective RNG seed for a lane. A locked lane (seedLocked=true) derives every
// deterministicRand roll from its preserved laneSeed, so its output stays
// byte-identical across a global reroll; an unlocked lane uses the global seed.
// With the defaults (seedLocked=false) this returns globalSeed unchanged, so
// output is byte-identical to the pre-lock RNG path (determinism tests stay
// green). laneId is still XOR-mixed inside deterministicRand, so two locked
// lanes sharing a laneSeed still diverge by lane index.
inline uint64_t laneEffectiveSeed(const LaneConfig& cfg, uint64_t globalSeed) {
    return cfg.seedLocked ? cfg.laneSeed : globalSeed;
}

// --- Additive cell helpers ---

struct AdditiveCellInfo {
    std::array<double, kMaxSteps> cumPpq{};
    double totalPpq = 0.0;
    int count = 0;
};

inline AdditiveCellInfo computeAdditiveCells(const LaneConfig& cfg) {
    AdditiveCellInfo info{};
    double basePpq = 4.0 / cfg.cycle.subdivision;

    // M003 S01 (EC08). A subdivision profile wins over integer cells: both
    // reaching here needs a stated winner, and combining two non-isochronies
    // silently is what nobody can reason about later.
    if (cfg.profileCount > 0) {
        int count = cfg.profileCount < kMaxSteps ? cfg.profileCount : kMaxSteps;
        double sum = 0.0;
        for (int i = 0; i < count; ++i)
            sum += static_cast<double>(cfg.subdivisionProfile[i]);
        // A profile summing to nothing is not a feel, and dividing by it would
        // put infinities into the timing path.
        if (sum <= 0.0)
            return info;
        // Normalise so the cycle occupies the length it would have evenly.
        double scale = sum / static_cast<double>(count);
        info.count = count;
        double accum = 0.0;
        for (int i = 0; i < count; ++i) {
            info.cumPpq[i] = accum;
            accum += (static_cast<double>(cfg.subdivisionProfile[i]) / scale) * basePpq;
        }
        info.totalPpq = accum;
        return info;
    }

    if (cfg.cellCount <= 0)
        return info;
    info.count = cfg.cellCount;
    double accum = 0.0;
    for (int i = 0; i < cfg.cellCount && i < kMaxSteps; ++i) {
        info.cumPpq[i] = accum;
        accum += static_cast<double>(cfg.cellSizes[i]) * basePpq;
    }
    info.totalPpq = accum;
    return info;
}

// --- Swing cell grouping (M003 S02, EC09) ---

struct SwingCellInfo {
    bool valid = false;
    int cell = 0;
    int positionInCell = 0;
    int cellSize = 0;
    // Issue #157 specifies the displaced pulse as "the final subdivision of
    // each 2- or 3-group", not the odd-parity one. On 2+2+3 the two readings
    // coincide -- both give steps 1, 3, 5 -- so an implementation of the wrong
    // one looks correct against the rachenitsa and diverges on 2+3+2.
    [[nodiscard]] bool isCellTail() const { return valid && positionInCell == cellSize - 1; }
};

// Map a step to the cell it falls in and its position within that cell.
// Returns an invalid result -- never a guess -- when the lane declares no
// grouping, when the sizes do not account for exactly stepsInCycle steps, or
// when the step is out of range. A grouping that does not add up is a
// configuration error, not a licence to read past the end of the array.
inline SwingCellInfo swingCellFor(const LaneConfig& cfg, int step, int stepsInCycle) {
    SwingCellInfo info{};
    if (cfg.swingCellCount <= 0 || cfg.swingCellCount > kMaxSteps)
        return info;
    if (step < 0 || stepsInCycle <= 0 || step >= stepsInCycle)
        return info;

    int total = 0;
    for (int c = 0; c < cfg.swingCellCount; ++c) {
        if (cfg.swingCellSizes[static_cast<size_t>(c)] <= 0)
            return info;
        total += cfg.swingCellSizes[static_cast<size_t>(c)];
    }
    if (total != stepsInCycle)
        return info;

    int remaining = step;
    for (int c = 0; c < cfg.swingCellCount; ++c) {
        const int size = cfg.swingCellSizes[static_cast<size_t>(c)];
        if (remaining < size) {
            info.valid = true;
            info.cell = c;
            info.positionInCell = remaining;
            info.cellSize = size;
            return info;
        }
        remaining -= size;
    }
    return info;
}

// --- Macros ---

// region:macro-values
struct MacroValues {
    float complexity = 0.5f;
    float density = 0.5f;
    float syncopation = 0.0f;
    float swing = 0.0f;
    float tension = 0.0f;
    float humanize = 0.0f;
};
// endregion:macro-values

// --- Note Map (global output pitch remapping) ---

struct NoteMap {
    std::array<int16_t, 128> map{};

    NoteMap() {
        for (int i = 0; i < 128; ++i)
            map[static_cast<size_t>(i)] = static_cast<int16_t>(i);
    }

    int16_t apply(int16_t note) const {
        if (note >= 0 && note < 128)
            return map[static_cast<size_t>(note)];
        return note;
    }

    void reset() {
        for (int i = 0; i < 128; ++i)
            map[static_cast<size_t>(i)] = static_cast<int16_t>(i);
    }
};

// --- Groove State (full serializable patch) ---

// region:groove-state
struct GrooveState {
    std::array<LaneConfig, kMaxLanes> lanes{};
    int activeLaneCount = 4;
    std::array<Envelope, kMaxGlobalEnvelopes> globalEnvelopes{};
    int globalEnvelopeCount = 0;
    MacroValues macros{};
    uint64_t seed = 0;
    int globalDensityCeiling = 0;
    // Transient momentary control (NOT serialized): when true, the current
    // render pass forces every bar to render as a fill bar for lanes,
    // independent of each lane's fillEveryNBars. The plugin pulses this for a
    // single render from a manual-fill trigger, then clears it.
    bool fillManualTrigger = false;
};
// endregion:groove-state

} // namespace poly
