#include <cstddef>
#include <cstdint>
#include <cstring>

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
    (void)poly::readGrooveState(read, groove);

    pos = 0;
    poly::SceneState scene{};
    (void)poly::readSceneState(read, scene);

    return 0;
}
