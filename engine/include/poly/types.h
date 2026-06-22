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
    float phraseLength = 0.0f; // beats; 0 = continuous (no phrase gating)
    float phraseGap = 0.0f;    // beats; silence between phrases
    float phraseOffset = 0.0f; // beats; phase offset for this lane's phrase cycle
    float mutationRate = 0.0f; // 0.0-1.0; per-step mutation probability each cycle
    float driftRate = 0.0f;    // steps per bar; pattern rotation rate from absolute PPQ
    bool active = true;
    std::array<EnvelopeAssign, kMaxEnvelopesPerLane> envelopes{};
    int envelopeCount = 0;
    ConstraintConfig constraints{};
};

// --- Macros ---

struct MacroValues {
    float complexity = 0.5f;
    float density = 0.5f;
    float syncopation = 0.0f;
    float swing = 0.0f;
    float tension = 0.0f;
    float humanize = 0.0f;
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
