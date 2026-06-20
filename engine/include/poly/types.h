#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace poly {

struct TransportContext {
    double ppqStart = 0.0;
    double ppqEnd = 0.0;
    double tempo = 120.0;
    bool looping = false;
    bool jumped = false;
};

struct NoteEvent {
    double ppqPosition = 0.0;
    int16_t pitch = 0;
    float velocity = 0.0f;
    double duration = 0.0;
    int16_t channel = 0;
};

static constexpr size_t kMaxEventsPerBlock = 256;

struct NoteEventBuffer {
    std::array<NoteEvent, kMaxEventsPerBlock> events{};
    size_t count = 0;

    void clear() { count = 0; }

    bool push(const NoteEvent& e) {
        if (count >= kMaxEventsPerBlock) return false;
        events[count++] = e;
        return true;
    }
};

} // namespace poly
