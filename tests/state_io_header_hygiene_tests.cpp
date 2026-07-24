// M049 S08 (E9): header self-containment probe.
//
// Before the split, state_io_write_lane.h / state_io_read_lane.h called
// writeEnvelope/readEnvelope without including anything that declared them.
// They only compiled because state_io.h defined the envelope helpers first,
// closed the namespace, then included the lane headers mid-file, then
// reopened the namespace. The lane headers were unusable in isolation.
//
// This translation unit deliberately includes ONLY the lane headers (not
// state_io.h) and instantiates writeLaneConfig/readLaneConfig. If either
// header regressed to hidden dependencies on state_io.h's mid-file include
// ordering, this TU would fail to compile.
//
// Do NOT add "#include "poly/state_io.h"" here. The whole point of this
// test is to prove the lane headers stand alone.

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include "poly/state_io_read_lane.h"
#include "poly/state_io_write_lane.h"
#include "poly/types.h"

TEST(StateIoHeaderHygiene, LaneHeadersAreSelfContained) {
    poly::LaneConfig lane{};
    lane.id = 3;
    lane.midiNote = 42;
    lane.cycle = {.steps = 8, .subdivision = 8};
    lane.hitCount = 3;

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto p = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + size);
        return true;
    };
    ASSERT_TRUE(poly::writeLaneConfig(write, lane, poly::kCurrentStateVersion));
    EXPECT_GT(buffer.size(), 0u);

    size_t pos = 0;
    auto read = [&buffer, &pos](void* data, size_t size) -> bool {
        if (pos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + pos, size);
        pos += size;
        return true;
    };
    poly::LaneConfig restored{};
    ASSERT_TRUE(poly::readLaneConfig(read, restored, poly::kCurrentStateVersion));

    EXPECT_EQ(restored.id, lane.id);
    EXPECT_EQ(restored.midiNote, lane.midiNote);
    EXPECT_EQ(restored.cycle.steps, lane.cycle.steps);
    EXPECT_EQ(restored.hitCount, lane.hitCount);
}
