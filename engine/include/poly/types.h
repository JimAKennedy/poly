#pragma once

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

struct TransportContext {
    double ppqStart = 0.0;
    double ppqEnd = 0.0;
    double tempo = 120.0;
    double sampleRate = 44100.0;
    int32_t blockSize = 512;
    bool playing = false;
    bool looping = false;
    bool jumped = false;
    double loopStartPpq = 0.0;
    double loopEndPpq = 0.0;
};

// --- Note Output ---

struct NoteEvent {
    double ppqPosition = 0.0;
    int16_t pitch = 0;
    float velocity = 0.0f;
    double duration = 0.0;
    int16_t channel = 0;
};

struct NoteEventBuffer {
    std::array<NoteEvent, kMaxEventsPerBlock> events{};
    size_t count = 0;

    void clear() { count = 0; }

    bool push(const NoteEvent& e) {
        if (count >= kMaxEventsPerBlock)
            return false;
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
    std::array<bool, kMaxSteps> steps{};
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

struct LaneConfig {
    int id = 0;
    Role role = Role::Custom;
    int16_t midiNote = 36;
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
    float phraseLength = 0.0f;                    // beats; 0 = continuous (no phrase gating)
    float phraseGap = 0.0f;                       // beats; silence between phrases
    float phraseOffset = 0.0f;                    // beats; phase offset for this lane's phrase cycle
    float mutationRate = 0.0f;                    // 0.0-1.0; per-step mutation probability each cycle
    float driftRate = 0.0f;                       // steps per bar; pattern rotation rate from absolute PPQ
    float timingOffsetMs = 0.0f;                  // ms; positive = late, negative = early; range [-20, +20]
    int kotekanSourceLane = -1;                   // -1=independent, 0-7=complement of source lane's pattern
    int cellCount = 0;                            // 0 = equal cells (standard Euclidean); >0 = additive/aksak
    std::array<int, kMaxSteps> cellSizes{};       // subdivision units per cell; sum = total cycle length
    bool timeline = false;                        // timeline mode: use fixedPattern, immune to macros
    std::array<bool, kMaxSteps> fixedPattern{};   // per-step on/off for timeline mode
    int fixedPatternLength = 0;                   // 0 = use cycle.steps; >0 = explicit length
    std::array<float, kMaxSteps> microTimingMs{}; // per-step timing offset in ms; range [-20, +20]
    bool active = true;
    std::array<EnvelopeAssign, kMaxEnvelopesPerLane> envelopes{};
    int envelopeCount = 0;
    ConstraintConfig constraints{};
};

// --- Additive cell helpers ---

struct AdditiveCellInfo {
    std::array<double, kMaxSteps> cumPpq{};
    double totalPpq = 0.0;
    int count = 0;
};

inline AdditiveCellInfo computeAdditiveCells(const LaneConfig& cfg) {
    AdditiveCellInfo info{};
    if (cfg.cellCount <= 0)
        return info;
    info.count = cfg.cellCount;
    double basePpq = 4.0 / cfg.cycle.subdivision;
    double accum = 0.0;
    for (int i = 0; i < cfg.cellCount && i < kMaxSteps; ++i) {
        info.cumPpq[i] = accum;
        accum += static_cast<double>(cfg.cellSizes[i]) * basePpq;
    }
    info.totalPpq = accum;
    return info;
}

// --- Macros ---

struct MacroValues {
    float complexity = 0.5f;
    float density = 0.5f;
    float syncopation = 0.0f;
    float swing = 0.0f;
    float tension = 0.0f;
    float humanize = 0.0f;
};

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

struct GrooveState {
    std::array<LaneConfig, kMaxLanes> lanes{};
    int activeLaneCount = 4;
    std::array<Envelope, kMaxGlobalEnvelopes> globalEnvelopes{};
    int globalEnvelopeCount = 0;
    MacroValues macros{};
    uint64_t seed = 0;
    int globalDensityCeiling = 0;
};

} // namespace poly
