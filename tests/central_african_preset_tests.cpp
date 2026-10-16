// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// central-africa M002/S01 (CA15): each Central African factory preset plays
// Arom's marks exactly.
//
// The fixtures are the grids Arom transcribes in *African Polyphony and
// Polyrhythm* (1991), Book VI, Chapter 3, as given in the owner's research note
// (Part C, examples 1–11). Each preset is resolved through the production
// pipeline — resolveConstraints(base, resolveMacros(base)) — and rendered by
// the engine itself, so the test checks what Poly emits rather than a mirror of
// the pattern builder. A lane that is shorter than the formula's period (a
// 3-step pulsation lane against 24 positions) is checked tiled across the
// period, as it plays.

#include <cmath>
#include <cstring>
#include <ostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "poly/constraint.h"
#include "poly/engine.h"
#include "poly/macro.h"
#include "poly/presets.h"

namespace {

struct Part {
    int lane;
    const char* row; // 'x' = mark, '.' = none; '|' and spaces are for reading
};

struct Formula {
    const char* preset;
    int period;     // positions in the formula's period
    double stepPpq; // quarter notes per position
    std::vector<Part> parts;
    const char* count; // Arom's (count) row: parts marking each position
};

// gtest prints a parameter that fails; name the preset rather than dump bytes.
void PrintTo(const Formula& f, std::ostream* os) {
    *os << f.preset;
}

std::vector<bool> marks(const char* row) {
    std::vector<bool> out;
    for (const char* c = row; *c; ++c)
        if (*c == 'x' || *c == '.')
            out.push_back(*c == 'x');
    return out;
}

std::vector<int> counts(const char* row) {
    std::vector<int> out;
    for (const char* c = row; *c; ++c)
        if (*c == '.')
            out.push_back(0);
        else if (*c >= '1' && *c <= '9')
            out.push_back(*c - '0');
    return out;
}

int presetIndexByName(const char* name) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i)
        if (std::strcmp(poly::getFactoryPresetInfo(i).name, name) == 0)
            return i;
    return -1;
}

// The positions within one period at which `lane` sounds.
std::vector<bool> renderedMarks(const poly::GrooveState& base, int lane, int period, double stepPpq) {
    const poly::GrooveState resolved = poly::resolveConstraints(base, poly::resolveMacros(base));
    poly::Engine engine;
    poly::NoteEventBuffer notes;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = period * stepPpq;
    tc.tempo = 120.0;
    tc.playing = true;
    engine.renderRange(tc, resolved, notes, nullptr);
    std::vector<bool> out(static_cast<size_t>(period), false);
    for (size_t i = 0; i < notes.count; ++i) {
        const poly::NoteEvent& e = notes.events[i];
        if (e.laneIndex != lane)
            continue;
        const long pos = std::lround(e.ppqPosition / stepPpq);
        if (pos >= 0 && pos < period)
            out[static_cast<size_t>(pos)] = true;
    }
    return out;
}

std::string show(const std::vector<bool>& v) {
    std::string s;
    for (bool b : v)
        s += b ? 'x' : '.';
    return s;
}

const std::vector<Formula>& formulae() {
    static const std::vector<Formula> all = {
        // Ngbolu, exx. 104–105, p. 288: 8 ternary pulsations, periods 24 : 12 : 3.
        {"Aka: Ngbolu",
         24,
         0.5,
         {
             {0, ". . x | . . x | . . x | . . x | . . x | . . x | . . x | . . x"}, // dikpakpa
             {1, "x . x | . x . | . . x | . x . | x . x | . x . | . . x | . x ."}, // ngue
             {2, "x . x | . x . | x x . | x . x | . x . | x . x | x . x | . x ."}, // diketo
         },
         "2 . 3 | . 2 1 | 1 1 2 | 1 1 2 | 1 1 2 | 1 1 2 | 1 . 3 | . 2 1"},
    };
    return all;
}

class CentralAfricanFormula : public ::testing::TestWithParam<Formula> {};

// A slip in typing a fixture should fail here, before any engine is involved.
TEST_P(CentralAfricanFormula, FixtureRowsSumToAromsCountRow) {
    const Formula& f = GetParam();
    const std::vector<int> want = counts(f.count);
    ASSERT_EQ(static_cast<int>(want.size()), f.period) << f.preset << ": count row length";
    std::vector<int> got(static_cast<size_t>(f.period), 0);
    for (const Part& p : f.parts) {
        const std::vector<bool> m = marks(p.row);
        ASSERT_EQ(static_cast<int>(m.size()), f.period) << f.preset << " lane " << p.lane << ": row length";
        for (int i = 0; i < f.period; ++i)
            got[static_cast<size_t>(i)] += m[static_cast<size_t>(i)] ? 1 : 0;
    }
    EXPECT_EQ(got, want) << f.preset;
}

TEST_P(CentralAfricanFormula, EveryLaneSoundsExactlyAromsMarks) {
    const Formula& f = GetParam();
    const int index = presetIndexByName(f.preset);
    ASSERT_GE(index, 0) << "no factory preset named " << f.preset;
    const poly::GrooveState state = poly::makeFactoryPreset(index);
    ASSERT_EQ(state.activeLaneCount, static_cast<int>(f.parts.size())) << f.preset << ": one lane per part";

    std::vector<int> sounding(static_cast<size_t>(f.period), 0);
    for (const Part& p : f.parts) {
        const std::vector<bool> want = marks(p.row);
        const std::vector<bool> got = renderedMarks(state, p.lane, f.period, f.stepPpq);
        EXPECT_EQ(show(got), show(want)) << f.preset << " lane " << p.lane;
        for (int i = 0; i < f.period; ++i)
            sounding[static_cast<size_t>(i)] += got[static_cast<size_t>(i)] ? 1 : 0;
    }
    EXPECT_EQ(sounding, counts(f.count)) << f.preset << ": per-position count against Arom's (count) row";
}

std::string formulaName(const ::testing::TestParamInfo<Formula>& info) {
    std::string out;
    for (const char* c = info.param.preset; *c; ++c)
        if ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9'))
            out += *c;
    return out;
}

INSTANTIATE_TEST_SUITE_P(Arom1991, CentralAfricanFormula, ::testing::ValuesIn(formulae()), formulaName);

} // namespace
