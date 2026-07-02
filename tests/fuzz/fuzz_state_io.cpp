#include <cstddef>
#include <cstdint>
#include <cstring>

#include "poly/engine.h"
#include "poly/state_io.h"
#include "poly/types.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    size_t pos = 0;
    auto read = [&](void* dst, size_t len) -> bool {
        if (pos + len > size)
            return false;
        std::memcpy(dst, data + pos, len);
        pos += len;
        return true;
    };

    poly::GrooveState groove{};
    if (poly::readGrooveState(read, groove)) {
        poly::Engine engine;
        poly::NoteEventBuffer buf;
        poly::TransportContext tc{};
        tc.playing = true;
        tc.ppqStart = 0.0;
        tc.ppqEnd = 4.0;
        tc.tempo = 120.0;
        tc.sampleRate = 44100.0;
        tc.blockSize = 512;
        engine.renderRange(tc, groove, buf);
    }

    pos = 0;
    poly::SceneState scene{};
    (void)poly::readSceneState(read, scene);

    return 0;
}
