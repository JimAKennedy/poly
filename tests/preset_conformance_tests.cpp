// M070 preset-conformance lint.
//
// This file is the dedicated home for the executable conformance assertions that
// enforce every M070 preset success criterion after the real
// resolveMacros/resolveConstraints pipeline (the same path the plugin and site
// consume via engine.cpp). The per-preset assertions were relocated here out of
// preset_tests.cpp (S04 T01) so the conformance lint lives in one auditable place
// and can grow negative RED-proof unit tests on its predicate helpers.
//
// Suite naming: every assertion lives under the PresetConformance suite so
// `ctest`/`--gtest_filter=PresetConformance.*` selects the whole lint.

#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include "poly/constraint.h"
#include "poly/engine.h"
#include "poly/euclidean.h"
#include "poly/macro.h"
#include "poly/presets.h"
#include "poly/state_io.h"

namespace {

// M070 S01: a "locked referent" lane is the single timekeeping/phase-reference
// lane whose pattern is frozen at preset-build time (D017 D2 mechanism, D020
// implementation). The lock is expressed through the existing engine timeline
// path: timeline=true with the resolved pattern baked into
// fixedPattern[0..fixedPatternLength), and every macro/mutation/probability
// perturbation neutralized so no runtime input can move the reference —
// probability==1.0, mutationRate==0, phraseLength==0 (no phrase gating). The
// baked pattern must be non-empty so the referent actually sounds a pulse.
// Byte-equality of the baked pattern to euclidean(hitCount,cycle.steps,rotation)
// and determinism are proven separately by the golden determinism tests (T03).
bool isLockedReferent(const poly::LaneConfig& cfg) {
    if (!cfg.timeline)
        return false;
    if (cfg.probability != 1.0f)
        return false;
    if (cfg.mutationRate != 0.0f)
        return false;
    if (cfg.phraseLength != 0.0f)
        return false;
    if (cfg.fixedPatternLength <= 0 || cfg.fixedPatternLength > poly::kMaxSteps)
        return false;
    for (int s = 0; s < cfg.fixedPatternLength; ++s) {
        if (cfg.fixedPattern[static_cast<size_t>(s)])
            return true; // non-empty baked pattern
    }
    return false;
}

// M070 S02 kotekan conformance: replicate the exact derivation the engine's
// buildLanePattern uses for a kotekan complement lane (engine.cpp region
// :kotekan) so the assertion tests the definition, not the full render path.
// A lane "kotekan-derives" when kotekanSourceLane names a valid, distinct lane
// and the source is not itself pointing back at this lane (the engine's
// mutual-reference guard). Returns true if the lane kotekan-derives; when it
// does, `out` holds the derived on/off pattern over [0, cfg.cycle.steps).
bool deriveKotekanPattern(const poly::GrooveState& state, int lane, std::array<bool, poly::kMaxSteps>& out) {
    out = {};
    const auto& cfg = state.lanes[static_cast<size_t>(lane)];
    if (cfg.timeline)
        return false;
    bool useKotekan =
        cfg.kotekanSourceLane >= 0 && cfg.kotekanSourceLane < state.activeLaneCount && cfg.kotekanSourceLane != lane;
    if (useKotekan) {
        const auto& src = state.lanes[static_cast<size_t>(cfg.kotekanSourceLane)];
        if (src.kotekanSourceLane == lane)
            useKotekan = false;
    }
    if (!useKotekan)
        return false;

    const auto& src = state.lanes[static_cast<size_t>(cfg.kotekanSourceLane)];
    std::array<bool, poly::kMaxSteps> srcPattern{};
    poly::euclidean(src.hitCount, src.cycle.steps, src.rotation, srcPattern);
    for (int step = 0; step < cfg.cycle.steps && step < src.cycle.steps; ++step)
        out[static_cast<size_t>(step)] = !srcPattern[static_cast<size_t>(step)];
    for (int step = src.cycle.steps; step < cfg.cycle.steps && step < poly::kMaxSteps; ++step)
        out[static_cast<size_t>(step)] = true;
    return true;
}

// M070 S04: the "metronomic traditions" are the authored-grid, non-swung
// idioms whose feel is defined by strict, even subdivision — Sub-Saharan
// African timeline ensembles, Balkan/Eastern-European aksak, Indian tala, and
// Balinese/Javanese gamelan. In Poly these map exactly to three PresetInfo
// categories. Applying swing (delaying every other subdivision by a fraction of
// a step) or humanize (random per-note timing jitter) to these traditions is a
// genre error: their groove comes from the interlock/colotomic grid landing
// dead-on the pulse, not from a laid-back or loose feel. The Experimental /
// Fusion category (e.g. Afro-Electronic Fusion) is intentionally NOT metronomic
// — its swing is the point — so it is excluded even though it borrows gamelan
// and clave material.
bool isMetronomicTradition(const char* category) {
    return std::strcmp(category, "African") == 0 || std::strcmp(category, "Balkan / Eastern European") == 0 ||
           std::strcmp(category, "Asian Traditions") == 0;
}

} // namespace

// M070 S01 referent-lock conformance: every factory preset must lock exactly
// one referent lane via the D2 mechanism (see isLockedReferent). Fails RED if a
// preset leaves its referent unlocked (count 0) or locks more than one lane
// (count >1) — giving the lint teeth against both under- and over-locking.
TEST(PresetConformance, EveryPresetHasExactlyOneLockedReferent) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto state = poly::makeFactoryPreset(i);
        auto info = poly::getFactoryPresetInfo(i);

        int lockedCount = 0;
        for (int lane = 0; lane < state.activeLaneCount; ++lane) {
            if (isLockedReferent(state.lanes[static_cast<size_t>(lane)]))
                ++lockedCount;
        }

        EXPECT_EQ(lockedCount, 1) << info.name << " (preset " << i
                                  << ") must have exactly one locked referent lane; found " << lockedCount;
    }
}

// M070 S02 conformance: every lane that derives its pattern from another lane
// via the kotekan mechanism (kotekanSourceLane) must render a non-empty
// pattern. The interlocking tradition (polos + sangsih) requires the complement
// to fill the source's gaps — if the source lane is fully saturated
// (hitCount >= cycle.steps), its euclidean pattern is all-true, so !srcPattern
// is all-false and the complement voice is completely SILENT. Fails RED on the
// current tree: Kotekan Interlock (preset 7) ships source lane 0 as E(3,3),
// fully saturated, so its sangsih complement lane 1 renders silent. Fixed GREEN
// in T02 by thinning the polos so the sangsih has gaps to interlock into.
TEST(PresetConformance, EveryKotekanLaneRendersNonEmpty) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto state = poly::makeFactoryPreset(i);
        auto info = poly::getFactoryPresetInfo(i);

        for (int lane = 0; lane < state.activeLaneCount; ++lane) {
            std::array<bool, poly::kMaxSteps> pattern{};
            if (!deriveKotekanPattern(state, lane, pattern))
                continue; // lane does not kotekan-derive

            bool nonEmpty = false;
            for (int step = 0; step < state.lanes[static_cast<size_t>(lane)].cycle.steps && step < poly::kMaxSteps;
                 ++step) {
                if (pattern[static_cast<size_t>(step)]) {
                    nonEmpty = true;
                    break;
                }
            }
            EXPECT_TRUE(nonEmpty) << info.name << " (preset " << i << ") kotekan lane " << lane
                                  << " derives an all-silent pattern (source lane "
                                  << state.lanes[static_cast<size_t>(lane)].kotekanSourceLane << " is saturated)";
        }
    }
}

// M070 S02 conformance: no non-timeline lane may ship fully saturated
// (hitCount >= cycle.steps) unless the density is a deliberate, musically
// motivated choice recorded in the allowlist below. A saturated lane plays
// every step, flattening any groove the lane was meant to carry, and — when it
// is a kotekan source — silences its complement (see
// EveryKotekanLaneRendersNonEmpty). Timeline lanes are exempt: their density is
// governed by an explicit fixedPattern, not hitCount.
//
// Allowlist entries are {presetIndex, lane}. Each entry MUST carry a one-line
// comment justifying why saturation is intentional for that lane.
TEST(PresetConformance, NoUnintendedSaturation) {
    struct SaturationException {
        int presetIndex;
        int lane;
    };
    // Justified intentional-saturation entries: lanes whose full density is the
    // point of the tradition, not a groove-flattening bug. Each entry names why.
    static const std::vector<SaturationException> kIntentionalSaturation = {
        // Four on the Floor: straight-8th hi-hat is the genre's continuous
        // timekeeper, not a rhythmic figure that needs gaps.
        {0, 2},
        // Reich Phasing: lane 2 is the steady pulse the two phasing voices drift
        // against — a constant grid is the whole point of the piece.
        {6, 2},
        // Afrobeat 12/8: lane 1 is the four-on-the-floor kick — its every-beat
        // pulse defines the groove.
        {9, 1},
        // Afrobeat 12/8: lane 3 is a continuous shaker stream (idiomatic
        // 12th-note timekeeping).
        {9, 3},
        // Carnatic Tala: lane 1 strikes once per additive anga [4+2+2]; a stroke
        // on each of the 3 cells is the tala skeleton, not saturation.
        {12, 1},
        // Reich Phase Process: lane 2 is the steady pulse reference (same
        // rationale as preset 6).
        {27, 2},
        // Afro-Electronic Fusion: lane 1 is the four-on-the-floor techno kick —
        // its every-beat pulse defines the genre.
        {40, 1},
    };

    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto state = poly::makeFactoryPreset(i);
        auto info = poly::getFactoryPresetInfo(i);

        for (int lane = 0; lane < state.activeLaneCount; ++lane) {
            const auto& cfg = state.lanes[static_cast<size_t>(lane)];
            if (cfg.timeline)
                continue; // density governed by fixedPattern, not hitCount
            std::array<bool, poly::kMaxSteps> kotekanPattern{};
            if (deriveKotekanPattern(state, lane, kotekanPattern))
                continue; // kotekan-derived: density comes from the source lane's
                          // complement, not this lane's own hitCount (which is
                          // unused). Non-emptiness is covered by
                          // EveryKotekanLaneRendersNonEmpty.
            if (cfg.hitCount < cfg.cycle.steps)
                continue; // not saturated

            bool allowed = false;
            for (const auto& ex : kIntentionalSaturation) {
                if (ex.presetIndex == i && ex.lane == lane) {
                    allowed = true;
                    break;
                }
            }
            EXPECT_TRUE(allowed) << info.name << " (preset " << i << ") lane " << lane << " is saturated (hitCount "
                                 << cfg.hitCount << " >= cycle.steps " << cfg.cycle.steps
                                 << "); de-saturate it or add a justified allowlist entry";
        }
    }
}

// M070 S02 conformance (T07): a kotekan complement lane derives its pattern as
// !euclidean(source.hitCount, source.cycle.steps, source.rotation). This asserts,
// per kotekan pair, both halves of the interlock invariant at definition time:
// (1) the source (polos) is NOT saturated — hitCount < cycle.steps — so it leaves
// gaps, and (2) the derived complement (sangsih) renders a non-empty pattern.
// Complements EveryKotekanLaneRendersNonEmpty by naming the root cause (a saturated
// source) directly rather than only its silent symptom.
TEST(PresetConformance, KotekanSourceNotSaturatedComplementNonEmpty) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto state = poly::makeFactoryPreset(i);
        auto info = poly::getFactoryPresetInfo(i);

        for (int lane = 0; lane < state.activeLaneCount; ++lane) {
            std::array<bool, poly::kMaxSteps> pattern{};
            if (!deriveKotekanPattern(state, lane, pattern))
                continue; // lane does not kotekan-derive

            const auto& cfg = state.lanes[static_cast<size_t>(lane)];
            const auto& src = state.lanes[static_cast<size_t>(cfg.kotekanSourceLane)];

            EXPECT_LT(src.hitCount, src.cycle.steps)
                << info.name << " (preset " << i << ") kotekan source lane " << cfg.kotekanSourceLane
                << " is saturated (hitCount " << src.hitCount << " >= cycle.steps " << src.cycle.steps
                << "); thin the source so complement lane " << lane << " has gaps to interlock into";

            bool nonEmpty = false;
            for (int step = 0; step < cfg.cycle.steps && step < poly::kMaxSteps; ++step) {
                if (pattern[static_cast<size_t>(step)]) {
                    nonEmpty = true;
                    break;
                }
            }
            EXPECT_TRUE(nonEmpty) << info.name << " (preset " << i << ") kotekan complement lane " << lane
                                  << " derives an all-silent pattern from source lane " << cfg.kotekanSourceLane;
        }
    }
}

// M070 S02 conformance (T07): fixed-structure traditions (talas and interlocking
// kotekan) must ship a neutral-or-denser default complexity (>= 0.5). resolveMacros
// (macro.cpp region :apply) treats complexity < 0.5 as a thinning gesture — it lerps
// every non-timeline lane's hitCount toward 1 and rotation toward 0 — which perturbs
// the authored, fixed structural figures the tradition is defined by. The referent
// lane is timeline-locked and macro-immune, but the supporting structural lanes
// (theka/dugun/tigun, jegogan/reyong) are not, so a sub-neutral default silently
// dissolves the tala skeleton / interlock density before a note plays.
//
// Scope is an explicit list, not a category: colotomic gamelan presets are excluded
// because their non-referent lanes each carry a single stroke (hitCount 1) that
// complexity < 0.5 cannot thin further, and aksak/folk presets (Aksak, Rachenitsa,
// Kopanitsa) are out of this slice's scope and conformant per prior review.
TEST(PresetConformance, StructuralPresetsShipNeutralComplexity) {
    struct StructuralPreset {
        int index;
        const char* label;
    };
    static const std::vector<StructuralPreset> kStructuralPresets = {
        {12, "Carnatic Tala"},
        {21, "Balinese Kotekan"},
        {23, "Tintal Groove"},
        {24, "Rupak Tal"},
    };

    for (const auto& sp : kStructuralPresets) {
        auto state = poly::makeFactoryPreset(sp.index);
        EXPECT_GE(state.macros.complexity, 0.5f)
            << sp.label << " (preset " << sp.index << ") ships complexity " << state.macros.complexity
            << " < 0.5; resolveMacros thins its non-referent structural lanes below neutral, "
               "perturbing the fixed tala/interlock structure";
    }
}

// M070 S02 conformance: funk and jazz presets ("Jazz / Funk / Soul" category)
// depend on the backbeat landing squarely on beats 2 and 4 (and "the One").
// resolveMacros rotates every non-timeline lane by round(syncopation * steps/2)
// (macro.cpp region :apply), so a nonzero shipped-default syncopation macro
// silently shifts the Backbeat lane off the downbeat even though the pocket is
// already authored per-lane via rotation — making the default syncopation both
// redundant and genre-breaking. This asserts that applying a preset's SHIPPED
// default macros leaves every non-timeline Backbeat lane's Euclidean pattern
// unchanged. The timeline-locked referent is macro-immune (resolveMacros skips
// timeline lanes) and never carries the Backbeat role here, so no funk/jazz
// backbeat is exempt. Latin/breaks/IDM presets are intentionally out of scope:
// their syncopation is the point of the genre, not a defect.
TEST(PresetConformance, BackbeatSurvivesSyncopationDefault) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto info = poly::getFactoryPresetInfo(i);
        if (std::strcmp(info.category, "Jazz / Funk / Soul") != 0)
            continue;

        auto base = poly::makeFactoryPreset(i);
        auto resolved = poly::resolveMacros(base);

        for (int lane = 0; lane < base.activeLaneCount; ++lane) {
            const auto& bcfg = base.lanes[static_cast<size_t>(lane)];
            if (bcfg.role != poly::Role::Backbeat)
                continue;
            if (bcfg.timeline)
                continue; // locked referent is macro-immune (resolveMacros skips it)

            std::array<bool, poly::kMaxSteps> want{};
            std::array<bool, poly::kMaxSteps> got{};
            poly::euclidean(bcfg.hitCount, bcfg.cycle.steps, bcfg.rotation, want);
            const auto& rcfg = resolved.lanes[static_cast<size_t>(lane)];
            poly::euclidean(rcfg.hitCount, rcfg.cycle.steps, rcfg.rotation, got);

            for (int step = 0; step < bcfg.cycle.steps && step < poly::kMaxSteps; ++step) {
                EXPECT_EQ(want[static_cast<size_t>(step)], got[static_cast<size_t>(step)])
                    << info.name << " (preset " << i << ") backbeat lane " << lane
                    << " is displaced by its default macros at step " << step
                    << "; the 2/4 backbeat must survive macro resolution intact";
            }
        }
    }
}

// M070 S03 conformance (T03): the Syncopation macro rotates every non-timeline
// lane by round(syncopation * steps/2) (macro.cpp region :apply). Raising the
// macro therefore shifts Backbeat lanes off beats 2 and 4 — the genre-undo that
// backbeatProtect fixes by activating on every non-timeline Backbeat lane
// (presets.cpp protectBackbeatLanes) and restoring the authored rotation in
// resolveConstraints (constraint.cpp). This locks the fix at the PRESET level:
// it drives real factory-preset data through the same resolveMacros ->
// resolveConstraints pipeline the plugin/site consume (engine.cpp runs
// resolveConstraints after resolveMacros), with syncopation forced to 1.0 for
// maximum displacement, and asserts every non-timeline Backbeat lane retains its
// authored rotation. The timeline-locked referent is macro-immune and never
// carries the Backbeat role, so it is correctly skipped.
TEST(PresetConformance, BackbeatSurvivesSyncopationRotation) {
    int backbeatLanesChecked = 0;

    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto base = poly::makeFactoryPreset(i);
        auto info = poly::getFactoryPresetInfo(i);

        base.macros.syncopation = 1.0f; // maximum syncopation displacement

        auto macroResolved = poly::resolveMacros(base);
        auto constrained = poly::resolveConstraints(base, macroResolved);

        for (int lane = 0; lane < base.activeLaneCount; ++lane) {
            const auto& bcfg = base.lanes[static_cast<size_t>(lane)];
            if (bcfg.role != poly::Role::Backbeat)
                continue;
            if (bcfg.timeline)
                continue; // locked referent is macro-immune (resolveMacros skips it)

            ++backbeatLanesChecked;
            EXPECT_EQ(constrained.lanes[static_cast<size_t>(lane)].rotation, bcfg.rotation)
                << info.name << " (preset " << i << ") backbeat lane " << lane << " lost its authored rotation "
                << bcfg.rotation << " after syncopation=1.0 macro+constraint resolution (got "
                << constrained.lanes[static_cast<size_t>(lane)].rotation
                << "); backbeatProtect must restore rotation so the 2/4 backbeat survives";
        }
    }

    EXPECT_GT(backbeatLanesChecked, 0) << "expected at least one non-timeline Backbeat lane across factory presets; "
                                          "none found means the assertion is vacuously passing";
}

// M070 S04 conformance (T02, NEW): no metronomic-tradition preset may carry
// swing or humanize on any lane after resolveMacros. resolveMacros (macro.cpp
// region :apply) sets every non-timeline lane's swingAmount to
// clamp(base.swingAmount + macros.swing) and humanizeMs to
// clamp(base.humanizeMs + macros.humanize * 25). Because both terms are
// non-negative, a lane resolves to a nonzero value iff the preset ships either a
// nonzero per-lane swingAmount/humanizeMs OR a nonzero macros.swing/humanize —
// so this single post-macro assertion catches both sources. The metronomic
// traditions (Sub-Saharan African, Balkan, Indian tala, gamelan) are defined by
// a dead-on even grid; any swing/humanize there is a genre error (see
// isMetronomicTradition). Fails RED on the current tree (see tests/s04-swing-red.log)
// until presets.cpp zeroes the offending swing/humanize at the source; GREEN after.
// Timeline lanes are macro-immune (resolveMacros skips them) but are still checked
// here — an authored referent must not carry swing/humanize either.
TEST(PresetConformance, NoSwingHumanizeOnMetronomicTraditions) {
    int metronomicPresetsChecked = 0;

    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        auto info = poly::getFactoryPresetInfo(i);
        if (!isMetronomicTradition(info.category))
            continue;

        ++metronomicPresetsChecked;
        auto base = poly::makeFactoryPreset(i);
        auto resolved = poly::resolveMacros(base);

        for (int lane = 0; lane < resolved.activeLaneCount; ++lane) {
            const auto& cfg = resolved.lanes[static_cast<size_t>(lane)];
            EXPECT_EQ(cfg.swingAmount, 0.0f)
                << info.name << " (preset " << i << ", " << info.category << ") lane " << lane
                << " resolves to swingAmount " << cfg.swingAmount
                << " after resolveMacros; metronomic traditions must stay dead-on the grid (zero the per-lane "
                   "swingAmount and/or macros.swing at the source in presets.cpp)";
            EXPECT_EQ(cfg.humanizeMs, 0.0f)
                << info.name << " (preset " << i << ", " << info.category << ") lane " << lane
                << " resolves to humanizeMs " << cfg.humanizeMs
                << " after resolveMacros; metronomic traditions must stay dead-on the grid (zero the per-lane "
                   "humanizeMs and/or macros.humanize at the source in presets.cpp)";
        }
    }

    EXPECT_GT(metronomicPresetsChecked, 0) << "expected at least one metronomic-tradition preset across the factory "
                                              "set; none found means the assertion is vacuously passing";
}

// ---------------------------------------------------------------------------
// Negative RED-proof unit tests (T03)
//
// The per-preset assertions above all pass on the conformed factory set, which
// on its own cannot distinguish "the lint has teeth" from "the lint is a no-op".
// These tests feed synthetic bad GrooveStates/LaneConfigs directly to the lint's
// predicate helpers and assert they FLAG the violation — proving each conformance
// assertion would fail RED if a future edit reintroduced the defect.
// ---------------------------------------------------------------------------

namespace {

// A fully-conformant locked referent lane: timeline-baked, macro/mutation/
// probability/phrase perturbations all neutralized, and a non-empty baked
// pattern. Each negative test below mutates exactly one field off this baseline
// to prove isLockedReferent rejects that single violation.
poly::LaneConfig makeLockedReferentLane() {
    poly::LaneConfig cfg;
    cfg.timeline = true;
    cfg.probability = 1.0f;
    cfg.mutationRate = 0.0f;
    cfg.phraseLength = 0.0f;
    cfg.fixedPatternLength = 4;
    cfg.fixedPattern[0] = true; // non-empty baked pulse
    return cfg;
}

} // namespace

// M070 S01 negative RED-proof (T03): isLockedReferent must REJECT a lane that
// fails any single lock condition. A conformant referent is the positive control;
// each single-field mutation (non-timeline, live probability, live mutation, live
// phrase gating, absent/empty baked pattern) must flag as unlocked. This proves
// EveryPresetHasExactlyOneLockedReferent has teeth: a preset that forgets to
// neutralize even one perturbation cannot slip past the lint.
TEST(PresetConformance, IsLockedReferentRejectsUnlockedLane) {
    // Positive control: the fully-neutralized referent is accepted.
    EXPECT_TRUE(isLockedReferent(makeLockedReferentLane())) << "a fully-conformant locked referent must be accepted";

    {
        auto cfg = makeLockedReferentLane();
        cfg.timeline = false; // not a baked timeline referent
        EXPECT_FALSE(isLockedReferent(cfg)) << "a non-timeline lane must not count as locked";
    }
    {
        auto cfg = makeLockedReferentLane();
        cfg.probability = 0.8f; // probability perturbation left live
        EXPECT_FALSE(isLockedReferent(cfg)) << "probability != 1.0 must not count as locked";
    }
    {
        auto cfg = makeLockedReferentLane();
        cfg.mutationRate = 0.1f; // per-cycle mutation left live
        EXPECT_FALSE(isLockedReferent(cfg)) << "mutationRate != 0 must not count as locked";
    }
    {
        auto cfg = makeLockedReferentLane();
        cfg.phraseLength = 4.0f; // phrase gating left live
        EXPECT_FALSE(isLockedReferent(cfg)) << "phraseLength != 0 must not count as locked";
    }
    {
        auto cfg = makeLockedReferentLane();
        cfg.fixedPatternLength = 0; // no baked-pattern length
        EXPECT_FALSE(isLockedReferent(cfg)) << "fixedPatternLength <= 0 must not count as locked";
    }
    {
        auto cfg = makeLockedReferentLane();
        cfg.fixedPattern = {}; // baked pattern all-silent
        EXPECT_FALSE(isLockedReferent(cfg)) << "an all-silent baked pattern must not count as locked (no pulse sounds)";
    }
}

// M070 S01 negative RED-proof (T03): the exactly-one-locked-referent count that
// EveryPresetHasExactlyOneLockedReferent asserts must flag both under-locking
// (count 0) and over-locking (count > 1). Drives synthetic GrooveStates through
// the same per-lane counting logic to prove the assertion is not vacuously
// satisfiable — a preset with a free-running referent, or two locked lanes, is
// detected rather than silently accepted.
TEST(PresetConformance, ReferentCountDetectsUnlockedPreset) {
    auto countLocked = [](const poly::GrooveState& s) {
        int n = 0;
        for (int lane = 0; lane < s.activeLaneCount; ++lane) {
            if (isLockedReferent(s.lanes[static_cast<size_t>(lane)]))
                ++n;
        }
        return n;
    };

    // Under-locked: every lane free-running (default LaneConfig has timeline=false).
    {
        poly::GrooveState s;
        s.activeLaneCount = 3;
        for (int lane = 0; lane < 3; ++lane)
            s.lanes[static_cast<size_t>(lane)] = poly::LaneConfig{};
        EXPECT_EQ(countLocked(s), 0) << "a preset with no locked referent must count 0, tripping the != 1 assertion";
    }

    // Conformant: exactly one locked referent.
    {
        poly::GrooveState s;
        s.activeLaneCount = 3;
        s.lanes[0] = makeLockedReferentLane();
        s.lanes[1] = poly::LaneConfig{};
        s.lanes[2] = poly::LaneConfig{};
        EXPECT_EQ(countLocked(s), 1) << "exactly one locked referent must count 1 (conformant control)";
    }

    // Over-locked: two locked referents.
    {
        poly::GrooveState s;
        s.activeLaneCount = 3;
        s.lanes[0] = makeLockedReferentLane();
        s.lanes[1] = makeLockedReferentLane();
        s.lanes[2] = poly::LaneConfig{};
        EXPECT_EQ(countLocked(s), 2) << "a preset with two locked referents must count 2, tripping the != 1 assertion";
    }
}

// M070 S03 negative RED-proof (T03): prove the syncopation genre-undo is REAL so
// BackbeatSurvivesSyncopationDefault/Rotation are non-vacuous. resolveMacros
// (macro.cpp region :apply) rotates every non-timeline lane by
// round(syncopation * steps/2); at syncopation=1.0 an authored backbeat is
// displaced by half a cycle. This drives a synthetic non-timeline Backbeat lane
// through resolveMacros ALONE — no resolveConstraints / backbeatProtect — and
// asserts the rotation IS displaced, establishing the exact defect the
// constraint-level protection must undo. If resolveMacros ever stopped moving the
// backbeat, the survival assertions would pass trivially; this test fails first.
TEST(PresetConformance, SyncopationDefaultDisplacesBackbeatWhenUnprotected) {
    poly::GrooveState s;
    s.activeLaneCount = 1;

    poly::LaneConfig cfg;
    cfg.role = poly::Role::Backbeat;
    cfg.timeline = false;                    // macro-eligible (timeline lanes are skipped)
    cfg.cycle.steps = 8;                     // even cycle so steps/2 is exact
    cfg.hitCount = 2;                        // authored 2/4 backbeat
    cfg.rotation = 1;                        // authored placement
    cfg.constraints.backbeatProtect = false; // unprotected: no restoration
    s.lanes[0] = cfg;

    s.macros.syncopation = 1.0f; // maximum displacement

    auto resolved = poly::resolveMacros(s);

    EXPECT_NE(resolved.lanes[0].rotation, cfg.rotation)
        << "resolveMacros must displace an unprotected backbeat lane at syncopation=1.0; if it does not, "
           "BackbeatSurvives* asserts nothing";

    // The displacement is exactly the half-cycle syncopation rotation applied on
    // top of the (complexity=0.5 passthrough) authored rotation.
    const int expected = (cfg.rotation + (cfg.cycle.steps / 2)) % cfg.cycle.steps;
    EXPECT_EQ(resolved.lanes[0].rotation, expected)
        << "expected syncopation=1.0 to add round(steps/2) to the authored rotation";
}
