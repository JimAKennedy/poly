#include "poly/bridge.h"

#include <algorithm>
#include <cmath>

namespace poly {

int32_t ppqToSampleOffset(double ppqPosition, double ppqBlockStart,
                          double tempo, double sampleRate, int32_t blockSize) {
    if (tempo <= 0.0) return 0;
    double beatsOffset = ppqPosition - ppqBlockStart;
    double seconds = beatsOffset * 60.0 / tempo;
    int32_t sample = static_cast<int32_t>(std::round(seconds * sampleRate));
    return std::clamp(sample, int32_t{0}, blockSize > 0 ? blockSize - 1 : 0);
}

bool PendingNoteOffBuffer::push(const PendingNoteOff& noff) {
    if (count_ >= kCapacity) return false;
    buf_[count_++] = noff;
    return true;
}

size_t PendingNoteOffBuffer::flushDue(double ppqStart, double ppqEnd,
                                      PendingNoteOff* out, size_t maxOut) {
    size_t flushed = 0;
    size_t i = 0;
    while (i < count_ && flushed < maxOut) {
        if (buf_[i].ppqOff >= ppqStart && buf_[i].ppqOff < ppqEnd) {
            out[flushed++] = buf_[i];
            buf_[i] = buf_[--count_];
        } else {
            ++i;
        }
    }
    return flushed;
}

void PendingNoteOffBuffer::clear() {
    count_ = 0;
}

} // namespace poly
