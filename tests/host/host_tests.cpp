#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "poly/scene.h"
#include "poly/state_io.h"
#include "poly_test_host.h"

using poly::test::MidiEvent;
using poly::test::PolyTestHost;

namespace {

// Deserializes a saveState() byte buffer via readSceneState so tests can assert against
// individual fields (noteMap, macros, etc.) without depending on raw byte layout.
poly::SceneState deserializeSceneState(const std::vector<uint8_t>& bytes) {
    poly::SceneState scene{};
    size_t cursor = 0;
    auto read = [&](void* dst, size_t size) -> bool {
        if (cursor + size > bytes.size())
            return false;
        std::memcpy(dst, bytes.data() + cursor, size);
        cursor += size;
        return true;
    };
    if (!poly::readSceneState(read, scene))
        return {};
    return scene;
}

// Any non-identity map suffices to distinguish the "edited" state from the default map.
// Chosen to touch every byte so a partial write couldn't accidentally masquerade as success.
std::array<int16_t, 128> makeEditedNoteMap() {
    std::array<int16_t, 128> map{};
    for (int i = 0; i < 128; ++i)
        map[static_cast<size_t>(i)] = static_cast<int16_t>((i + 42) % 128);
    return map;
}

std::array<int16_t, 128> makeDefaultNoteMap() {
    std::array<int16_t, 128> map{};
    for (int i = 0; i < 128; ++i)
        map[static_cast<size_t>(i)] = static_cast<int16_t>(i);
    return map;
}

} // namespace

// --- T01: Basic harness sanity ---

TEST(HostTests, SetupAndTeardown) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    host.teardown();
}

TEST(HostTests, SetupMultipleSampleRates) {
    for (double sr : {44100.0, 48000.0, 96000.0}) {
        PolyTestHost host;
        ASSERT_TRUE(host.setup(sr, 512)) << "Failed at sample rate " << sr;
        host.teardown();
    }
}

// --- T02: Transport scenario tests ---

TEST(HostTests, CleanStart_EmitsEvents) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    host.playBars(8, 120.0);

    auto noteOns = host.noteOnEvents();
    EXPECT_GT(noteOns.size(), 0u) << "Default patch should produce note-on events";

    std::set<int16_t> pitches;
    for (const auto& e : noteOns)
        pitches.insert(e.pitch);
    EXPECT_GE(pitches.size(), 2u) << "Default 4-lane patch should produce events on multiple pitches";

    host.teardown();
}

TEST(HostTests, StopFlush_NoHangingNotes) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    host.playBars(2, 120.0);

    double lastPpq = 2.0 * 4.0;
    host.stopAndFlush(lastPpq, 120.0);

    std::map<int16_t, int> balance;
    for (const auto& e : host.events()) {
        if (e.type == MidiEvent::NoteOn)
            balance[e.pitch]++;
        else if (e.type == MidiEvent::NoteOff)
            balance[e.pitch]--;
    }
    for (const auto& [pitch, count] : balance) {
        EXPECT_LE(count, 0) << "Hanging note-on for pitch " << pitch << " (balance=" << count << ")";
    }

    host.teardown();
}

// --- Save/load state regression tests (M046 P1 + P2) ---
//
// These pin two release-blocking bugs in the plugin state-serialization path:
//
//   P1 (processor.cpp:383-415): the stopped-path early return drains pending edits into
//   sceneState_ but never republishes stateSnapshot_. If getState is called while the
//   stateSnapshot_ still holds a pre-edit value (from the last playing block), the DAW
//   saves stale state — the user's post-stop edit is silently lost.
//
//   P2 (processor.cpp:838-839): when snapshotReady_ is false, getState falls through to
//   an unsynchronized read of sceneState_ from the message thread. Racy in production;
//   in a single-threaded test it appears "correct", so P2 is covered indirectly by the
//   cold-processor round-trip test — which will break if T03 removes the fallback
//   without publishing an initial snapshot from setActive(true).

TEST(HostTests, SaveAfterStop_PreservesEdits) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    // Play a bar so the playing-path publishes stateSnapshot_ (snapshotReady_ = true)
    // holding the pre-edit noteMap. This mirrors the real "user hits play, then makes
    // an edit, then stops" sequence.
    host.playBars(1.0, 120.0);
    host.stopAndFlush(4.0, 120.0);

    // Simulate a controller-driven edit arriving after stop (e.g. user drags a note in
    // the note-map view while the transport is idle).
    const auto editedMap = makeEditedNoteMap();
    host.injectNoteMap(editedMap);

    // One stopped block so process() drains pendingNoteMap_ into sceneState_.
    // On current HEAD this drain happens (line 348-350 runs before the early return)
    // but the stopped-path early return at line 383-415 does NOT republish
    // stateSnapshot_ — so stateSnapshot_ still holds the pre-edit map.
    host.processBlock(4.0, 120.0, false);

    auto bytes = host.saveState();
    ASSERT_FALSE(bytes.empty()) << "saveState() should produce a non-empty buffer";

    auto scene = deserializeSceneState(bytes);

    // Fail-loud diagnostic: report how many map slots survived unchanged so the red
    // output tells the reader exactly how the bug manifested.
    int matched = 0;
    for (int i = 0; i < 128; ++i)
        if (scene.noteMap.map[static_cast<size_t>(i)] == editedMap[static_cast<size_t>(i)])
            ++matched;

    EXPECT_EQ(matched, 128) << "Save-after-stop dropped the noteMap edit: " << matched
                            << "/128 slots match the edited map. This is P1 in the M046 review — "
                               "the stopped-path early return in PolyProcessor::process() drains "
                               "pending edits into sceneState_ but never republishes stateSnapshot_, "
                               "so getState() returns the pre-edit snapshot.";

    host.teardown();
}

TEST(HostTests, GetStateFromColdProcessor_RoundTrips) {
    // Cold processor: setup only, no playBars, no notify(), no processBlock().
    // On current HEAD this passes via the P2 torn-state fallback because there is no
    // concurrent writer. Once T03 removes the fallback, this test guards the invariant
    // that setActive(true) must publish an initial stateSnapshot_ so cold getState()
    // still round-trips instead of returning kResultFalse or reading uninitialized state.
    PolyTestHost hostA;
    ASSERT_TRUE(hostA.setup(44100.0, 512));

    auto bytes = hostA.saveState();
    ASSERT_FALSE(bytes.empty()) << "Cold saveState() should produce a non-empty buffer";

    auto sceneA = deserializeSceneState(bytes);
    const auto defaultMap = makeDefaultNoteMap();
    for (int i = 0; i < 128; ++i) {
        ASSERT_EQ(sceneA.noteMap.map[static_cast<size_t>(i)], defaultMap[static_cast<size_t>(i)])
            << "Cold state should serialize the identity noteMap; slot " << i << " diverged";
    }

    hostA.teardown();

    PolyTestHost hostB;
    ASSERT_TRUE(hostB.setup(44100.0, 512));
    ASSERT_TRUE(hostB.loadState(bytes)) << "loadState() should accept a valid saveState() buffer";

    // One block so the setState → pendingState_ → sceneState_ handoff completes.
    hostB.processBlock(0.0, 120.0, false);

    auto bytes2 = hostB.saveState();
    ASSERT_FALSE(bytes2.empty()) << "Post-load saveState() should produce a non-empty buffer";
    auto sceneB = deserializeSceneState(bytes2);
    for (int i = 0; i < 128; ++i) {
        EXPECT_EQ(sceneB.noteMap.map[static_cast<size_t>(i)], defaultMap[static_cast<size_t>(i)])
            << "Round-tripped noteMap diverges from default at slot " << i;
    }

    hostB.teardown();
}

TEST(HostTests, Determinism_SameInputSameOutput) {
    auto runPass = []() -> std::vector<MidiEvent> {
        PolyTestHost host;
        host.setup(44100.0, 512);
        host.playBars(4, 120.0);
        auto result = host.noteOnEvents();
        host.teardown();
        return result;
    };

    auto pass1 = runPass();
    auto pass2 = runPass();

    ASSERT_FALSE(pass1.empty());
    ASSERT_EQ(pass1.size(), pass2.size()) << "Event counts must match across runs";
    for (size_t i = 0; i < pass1.size(); ++i) {
        EXPECT_EQ(pass1[i].pitch, pass2[i].pitch) << "Pitch mismatch at event " << i;
        EXPECT_FLOAT_EQ(pass1[i].velocity, pass2[i].velocity) << "Velocity mismatch at event " << i;
        EXPECT_DOUBLE_EQ(pass1[i].ppqPosition, pass2[i].ppqPosition) << "PPQ mismatch at event " << i;
    }
}

TEST(HostTests, TransportJump_EmitsNoteOffs) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    double tempo = 120.0;
    double step = host.ppqPerBlock(tempo);

    double ppq = 0.0;
    for (int i = 0; i < 8; ++i) {
        host.processBlock(ppq, tempo, true);
        ppq += step;
    }

    size_t eventsBefore = host.events().size();

    double jumpTarget = ppq + 32.0;
    host.processBlock(jumpTarget, tempo, true);

    bool hasNoteOffs = false;
    for (size_t i = eventsBefore; i < host.events().size(); ++i) {
        if (host.events()[i].type == MidiEvent::NoteOff) {
            hasNoteOffs = true;
            break;
        }
    }
    EXPECT_TRUE(hasNoteOffs) << "Transport jump should flush pending note-offs";

    host.teardown();
}

TEST(HostTests, BufferSizeInvariance) {
    auto runWithBlockSize = [](int blockSize) -> std::vector<MidiEvent> {
        PolyTestHost host;
        host.setup(44100.0, blockSize);
        host.playBars(4, 120.0);
        auto events = host.noteOnEvents();
        host.teardown();
        std::vector<MidiEvent> filtered;
        for (const auto& e : events)
            if (e.ppqPosition < 15.0)
                filtered.push_back(e);
        return filtered;
    };

    auto events64 = runWithBlockSize(64);
    auto events512 = runWithBlockSize(512);
    auto events4096 = runWithBlockSize(4096);

    ASSERT_FALSE(events64.empty());
    ASSERT_EQ(events64.size(), events512.size())
        << "Block size 64 vs 512: event count mismatch (" << events64.size() << " vs " << events512.size() << ")";
    ASSERT_EQ(events512.size(), events4096.size())
        << "Block size 512 vs 4096: event count mismatch (" << events512.size() << " vs " << events4096.size() << ")";

    for (size_t i = 0; i < events64.size(); ++i) {
        EXPECT_EQ(events64[i].pitch, events512[i].pitch) << "Pitch mismatch at event " << i << " (64 vs 512)";
        EXPECT_NEAR(events64[i].ppqPosition, events512[i].ppqPosition, 1e-6)
            << "PPQ mismatch at event " << i << " (64 vs 512)";
        EXPECT_FLOAT_EQ(events64[i].velocity, events512[i].velocity)
            << "Velocity mismatch at event " << i << " (64 vs 512)";
    }
}

// --- T03: Golden MIDI comparison at processor level ---

static std::string serializeNoteOns(const std::vector<MidiEvent>& events) {
    std::ostringstream oss;
    oss << "# processor golden: bars=4 tempo=120 blockSize=512 sampleRate=44100\n";
    oss << "# ppq_position  pitch  velocity  channel\n";
    for (const auto& e : events) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "%.6f  %3d  %.6f  %d\n", e.ppqPosition, e.pitch, e.velocity, e.channel);
        oss << buf;
    }
    return oss.str();
}

TEST(GoldenProcessor, DefaultPatch4Bars) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    host.playBars(4, 120.0);

    auto noteOns = host.noteOnEvents();
    ASSERT_FALSE(noteOns.empty()) << "Default patch should produce events";

    std::string actual = serializeNoteOns(noteOns);

    std::string goldenPath = GOLDEN_DIR "/processor_default_4bars.txt";
    std::ifstream goldenFile(goldenPath);

    if (!goldenFile.is_open()) {
        std::ofstream out(goldenPath);
        ASSERT_TRUE(out.is_open()) << "Could not create golden file at " << goldenPath;
        out << actual;
        out.close();
        GTEST_SKIP() << "Golden file created at " << goldenPath << " (" << noteOns.size() << " events). "
                     << "Re-run to verify determinism.";
    }

    std::string expected((std::istreambuf_iterator<char>(goldenFile)), std::istreambuf_iterator<char>());

    EXPECT_EQ(actual, expected) << "Processor output diverges from golden file.\n"
                                << "Note-on count: " << noteOns.size() << "\n"
                                << "To regenerate: rm " << goldenPath << " && re-run tests";

    host.teardown();
}
