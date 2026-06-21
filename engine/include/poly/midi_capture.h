#pragma once

#include <array>
#include <cstddef>

#include "poly/types.h"

namespace poly {

class MidiCaptureBuffer {
public:
    static constexpr size_t kCapacity = 2048;
    static constexpr int kDefaultCaptureBars = 8;
    static constexpr double kBeatsPerBar = 4.0;

    void push(const NoteEvent& event);
    void clear();

    size_t copyRaw(NoteEvent* out, size_t maxOut) const;
    size_t extract(double fromPpq, double toPpq, NoteEvent* out, size_t maxOut) const;
    size_t extractLastBars(int bars, NoteEvent* out, size_t maxOut) const;

    size_t count() const { return count_; }
    bool empty() const { return count_ == 0; }
    double newestPpq() const;

private:
    std::array<NoteEvent, kCapacity> events_{};
    size_t writePos_ = 0;
    size_t count_ = 0;
};

} // namespace poly
