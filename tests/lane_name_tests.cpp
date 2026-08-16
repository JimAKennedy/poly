// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include <cstring>
#include <string>

#include <gtest/gtest.h>

#include "poly/lane_name.h"

namespace {

// --- noteLabel(): the single-sourced GM taxonomy table -----------------------

TEST(LaneNameTest, NoteLabelCoreDrums) {
    EXPECT_STREQ("kick", poly::noteLabel(35));
    EXPECT_STREQ("kick", poly::noteLabel(36));
    EXPECT_STREQ("snare", poly::noteLabel(38));
    EXPECT_STREQ("snare", poly::noteLabel(40));
    EXPECT_STREQ("hat", poly::noteLabel(42));
    EXPECT_STREQ("hat", poly::noteLabel(44));
    EXPECT_STREQ("openhat", poly::noteLabel(46));
    EXPECT_STREQ("clap", poly::noteLabel(39));
    EXPECT_STREQ("rim", poly::noteLabel(37));
}

TEST(LaneNameTest, NoteLabelPercussionRange) {
    EXPECT_STREQ("tom", poly::noteLabel(41));
    EXPECT_STREQ("cymbal", poly::noteLabel(49));
    EXPECT_STREQ("cowbell", poly::noteLabel(56));
    EXPECT_STREQ("bongo", poly::noteLabel(60));
    EXPECT_STREQ("conga", poly::noteLabel(63));
    EXPECT_STREQ("shaker", poly::noteLabel(70));
    EXPECT_STREQ("clave", poly::noteLabel(75));
    EXPECT_STREQ("triangle", poly::noteLabel(81));
}

// Negative / boundary: notes outside the GM percussion table fall through to
// "perc" rather than crashing or returning nullptr.
TEST(LaneNameTest, NoteLabelUnknownFallsBackToPerc) {
    EXPECT_STREQ("perc", poly::noteLabel(0));
    EXPECT_STREQ("perc", poly::noteLabel(34));  // just below the table
    EXPECT_STREQ("perc", poly::noteLabel(82));  // just above the table
    EXPECT_STREQ("perc", poly::noteLabel(127)); // MIDI max
    EXPECT_STREQ("perc", poly::noteLabel(-1));  // out of MIDI range
    EXPECT_STREQ("perc", poly::noteLabel(9999));
}

TEST(LaneNameTest, NoteLabelNeverNull) {
    for (int note = -8; note <= 135; ++note) {
        EXPECT_NE(nullptr, poly::noteLabel(note)) << "note=" << note;
    }
}

// --- laneName(): human-readable display names --------------------------------

TEST(LaneNameTest, LaneNameCoreDrums) {
    EXPECT_STREQ("Kick", poly::laneName(36));
    EXPECT_STREQ("Snare", poly::laneName(38));
    EXPECT_STREQ("Hi-Hat", poly::laneName(42));
    EXPECT_STREQ("Open Hat", poly::laneName(46));
    EXPECT_STREQ("Clap", poly::laneName(39));
    EXPECT_STREQ("Rim", poly::laneName(37));
}

TEST(LaneNameTest, LaneNamePercussion) {
    EXPECT_STREQ("Tom", poly::laneName(41));
    EXPECT_STREQ("Cymbal", poly::laneName(49));
    EXPECT_STREQ("Cowbell", poly::laneName(56));
    EXPECT_STREQ("Conga", poly::laneName(63));
    EXPECT_STREQ("Clave", poly::laneName(75));
    EXPECT_STREQ("Triangle", poly::laneName(81));
}

TEST(LaneNameTest, LaneNameUnknownFallsBackToPerc) {
    EXPECT_STREQ("Perc", poly::laneName(0));
    EXPECT_STREQ("Perc", poly::laneName(127));
    EXPECT_STREQ("Perc", poly::laneName(-1));
}

TEST(LaneNameTest, LaneNameNeverNullOrEmpty) {
    for (int note = -8; note <= 135; ++note) {
        const char* name = poly::laneName(note);
        ASSERT_NE(nullptr, name) << "note=" << note;
        EXPECT_GT(std::strlen(name), 0u) << "note=" << note;
    }
}

// laneName() must stay a strict presentation layer over the single-sourced
// noteLabel() table: every note that maps to the same taxonomy label must map
// to the same display name. This is what keeps the mapping single-sourced.
TEST(LaneNameTest, LaneNameConsistentWithNoteLabel) {
    for (int note = 0; note <= 127; ++note) {
        const char* label = poly::noteLabel(note);
        const char* name = poly::laneName(note);
        // Two notes with the same taxonomy label share a display name.
        for (int other = 0; other <= 127; ++other) {
            if (std::strcmp(label, poly::noteLabel(other)) == 0) {
                EXPECT_STREQ(name, poly::laneName(other)) << "notes " << note << " and " << other << " share label '"
                                                          << label << "' but differ in display name";
            }
        }
    }
}

} // namespace
