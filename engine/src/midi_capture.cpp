#include "poly/midi_capture.h"

#include <algorithm>

namespace poly {

void MidiCaptureBuffer::push(const NoteEvent& event) {
    events_[writePos_] = event;
    writePos_ = (writePos_ + 1) % kCapacity;
    if (count_ < kCapacity)
        ++count_;
}

void MidiCaptureBuffer::clear() {
    writePos_ = 0;
    count_ = 0;
}

double MidiCaptureBuffer::newestPpq() const {
    if (count_ == 0)
        return 0.0;
    size_t newest = (writePos_ == 0) ? kCapacity - 1 : writePos_ - 1;
    return events_[newest].ppqPosition;
}

size_t MidiCaptureBuffer::copyRaw(NoteEvent* out, size_t maxOut) const {
    if (count_ == 0 || !out || maxOut == 0)
        return 0;
    size_t start = (count_ < kCapacity) ? 0 : writePos_;
    size_t n = std::min(count_, maxOut);
    for (size_t i = 0; i < n; ++i) {
        out[i] = events_[(start + i) % kCapacity];
    }
    return n;
}

size_t MidiCaptureBuffer::extract(double fromPpq, double toPpq, NoteEvent* out, size_t maxOut) const {
    if (count_ == 0 || !out || maxOut == 0)
        return 0;

    size_t start = (count_ < kCapacity) ? 0 : writePos_;
    size_t outCount = 0;

    for (size_t i = 0; i < count_ && outCount < maxOut; ++i) {
        size_t idx = (start + i) % kCapacity;
        if (events_[idx].ppqPosition >= fromPpq && events_[idx].ppqPosition < toPpq) {
            out[outCount++] = events_[idx];
        }
    }

    std::sort(out, out + outCount,
              [](const NoteEvent& a, const NoteEvent& b) { return a.ppqPosition < b.ppqPosition; });

    return outCount;
}

size_t MidiCaptureBuffer::extractLastBars(int bars, NoteEvent* out, size_t maxOut) const {
    if (count_ == 0)
        return 0;
    double newest = newestPpq();
    double fromPpq = newest - bars * kBeatsPerBar;
    return extract(fromPpq, newest + 1.0, out, maxOut);
}

} // namespace poly
