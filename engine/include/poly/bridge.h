#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace poly {

int32_t ppqToSampleOffset(double ppqPosition, double ppqBlockStart,
                          double tempo, double sampleRate, int32_t blockSize);

struct PendingNoteOff {
    double ppqOff = 0.0;
    int16_t pitch = 0;
    int16_t channel = 0;
};

class PendingNoteOffBuffer {
public:
    static constexpr size_t kCapacity = 512;

    bool push(const PendingNoteOff& noff);
    size_t flushDue(double ppqStart, double ppqEnd,
                    PendingNoteOff* out, size_t maxOut);
    void clear();
    size_t count() const { return count_; }

private:
    std::array<PendingNoteOff, kCapacity> buf_{};
    size_t count_ = 0;
};

} // namespace poly
