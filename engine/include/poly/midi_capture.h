#pragma once

#include <array>
#include <cstddef>

#include "poly/types.h"

namespace poly {

class MidiCaptureBuffer {
public:
    // M032 S02 T01: Sized to hold a genuinely dense 32-bar pattern without event
    // loss. The worst case is 8 lanes (kMaxLanes) each firing a note on every
    // sixteenth (subdivision clamps to 16) => 32 bars * 16 steps/bar * 8 lanes =
    // 4096 note events resident in the ring during the capture window. 8192 keeps
    // a 2x margin so extractLastBars(32) never evicts the head of the window. The
    // std::array below stays pre-allocated at construction, so raising this does
    // not introduce any runtime allocation on the RT capture path.
    static constexpr size_t kCapacity = 8192;
    static constexpr int kDefaultCaptureBars = 8;
    // M051 S02: kBeatsPerBar is the default when the caller doesn't know the
    // host meter (engine tests, back-compat). Real plugin call sites pass the
    // live TransportContext::ppqPerBar() so "last N bars" respects host 3/4, 7/8, etc.
    static constexpr double kBeatsPerBar = 4.0;

    void push(const NoteEvent& event);
    void clear();

    size_t copyRaw(NoteEvent* out, size_t maxOut) const;
    size_t extract(double fromPpq, double toPpq, NoteEvent* out, size_t maxOut) const;
    size_t extractLastBars(int bars, NoteEvent* out, size_t maxOut, double ppqPerBar = kBeatsPerBar) const;

    size_t count() const { return count_; }
    bool empty() const { return count_ == 0; }
    double newestPpq() const;

private:
    std::array<NoteEvent, kCapacity> events_{};
    size_t writePos_ = 0;
    size_t count_ = 0;
};

} // namespace poly
