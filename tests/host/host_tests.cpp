#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "poly_test_host.h"

using poly::test::MidiEvent;
using poly::test::PolyTestHost;

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
