// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "plugids.h"
#include "poly/bridge.h"
#include "poly/euclidean.h"
#include "poly/offline_render.h" // M032 S03 T02: export-tempo mirror integration test
#include "poly/params_def.h"
#include "poly/scene.h"
#include "poly/state_io.h"
#include "poly/types.h"
#include "poly_test_host.h"
#include "ui_snapshot.h" // M073: full UISnapshot for the emission-ring test

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

// --- IPC snapshot-pointer lifecycle regression (M046 S02 P3) ---
//
// The processor sends a raw pointer to its own UISnapshot field to the controller via an
// IMessage on IConnectionPoint::connect. The controller caches that pointer in uiSnapshot_
// and dereferences it from the UI thread. When the DAW tears the connection down, the
// controller MUST null the cached pointer — otherwise it dangles across reconnects and
// (worse) would point into another process's address space in a bridged/distributed host.
// See docs/reviews/2026-07-16-product-review.md P3.

TEST(HostTests, InitializeWithConnect_UISnapshotPopulated) {
    // Guardrail — proves the connect wiring works so DisconnectNullsSnapshotPointer's
    // "was non-null, now null" assertion is meaningful. If this ever regresses, the P3 fix's
    // disconnect assertion is testing nothing.
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    EXPECT_NE(host.controllerUiSnapshot(), nullptr)
        << "connect() should have populated the controller's uiSnapshot_ via UISnapshotPtr notify";
    host.teardown();
}

TEST(HostTests, DisconnectNullsSnapshotPointer) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    ASSERT_NE(host.controllerUiSnapshot(), nullptr) << "fixture broken: connect never populated snapshot";
    host.disconnectComponents();
    EXPECT_EQ(host.controllerUiSnapshot(), nullptr)
        << "P3 (M046 S02): PolyControllerBase::disconnect must null uiSnapshot_ so the raw "
           "pointer cannot dangle across reconnects";
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

// P1-regression (T05): pluginval on PR #80 failed 221/255 Plugin state restoration
// sub-tests because T03's setActive(true) initial publish leaves snapshotReady_ = true,
// and the playing-path guard at processor.cpp:483-486 only publishes when
// snapshotReady_ is false. So the first scene edit during playback drains into
// sceneState_ but never reaches stateSnapshot_. getState() returns the stale default.
// The fix is to publish unconditionally in the playing path (matching T02's stopped path).

TEST(HostTests, PlayingProcessWithParamChange_GetStateReflectsLatest) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    // setActive(true) has published the default sceneState_ into stateSnapshot_
    // and left snapshotReady_ = true. This is the pluginval trap.

    // Simulate the pluginval sequence: host queues a param change (here, a noteMap
    // edit via the same notify path a real controller uses), then process() runs.
    const auto editedMap = makeEditedNoteMap();
    host.injectNoteMap(editedMap);

    // One playing block. process() drains pendingNoteMap_ into sceneState_ before the
    // snapshot guard runs. On the buggy HEAD (guarded on !snapshotReady_), the guard
    // sees ready=true and skips — stateSnapshot_ stays at the default from setActive.
    host.processBlock(0.0, 120.0, true);

    auto bytes = host.saveState();
    ASSERT_FALSE(bytes.empty()) << "saveState() should produce a non-empty buffer";

    auto scene = deserializeSceneState(bytes);

    int matched = 0;
    for (int i = 0; i < 128; ++i)
        if (scene.noteMap.map[static_cast<size_t>(i)] == editedMap[static_cast<size_t>(i)])
            ++matched;

    EXPECT_EQ(matched, 128) << "Playing-path snapshot regression: " << matched
                            << "/128 slots match. The guard at processor.cpp:483-486 only publishes "
                               "stateSnapshot_ when snapshotReady_ is false, but T03's setActive(true) "
                               "initial publish leaves it true — so the first real scene edit during "
                               "playback drains into sceneState_ but never reaches stateSnapshot_. "
                               "This is the fingerprint pluginval saw on PR #80.";

    host.teardown();
}

// --- M046 S01a T01: controller-side pluginval-fingerprint tests ---
//
// The T05 hotfix (63b5dd6) fixed the playing-path snapshot guard but pluginval
// on PR #80 stayed red 224/255 with the same "Param not restored" fingerprint.
// Reason: pluginval also exercises the path where a param is set via
// IEditController::setParamNormalized WITHOUT a process() call between the
// mutation and the state save. In that path the controller's cache holds the
// new value but the processor's sceneState_ never sees it — the standard VST3
// automation flow is host → IParameterChanges → process() → sceneState_, and
// pluginval bypasses process() for its round-trip test.
//
// These two tests split the space:
//   CONTROL (green today): a param change flushed through process() DOES survive
//   the save→restore cycle. Guards the working path against regressions.
//
//   RED (red today, fingerprint match): a controller-only param mutation is
//   LOST in the save→restore cycle. Reproduces pluginval's failure locally
//   in <1s, closing the local-test gap that let 63b5dd6 pass locally then go
//   red on CI.

TEST(HostTests, ControllerParamChangeThroughProcess_ReflectsInGetState) {
    // Working path: the DAW-standard automation flow.
    // A param change queued onto IParameterChanges and drained by process()
    // reaches sceneState_ AND (via T05's unconditional playing-path publish)
    // stateSnapshot_, so save→restore round-trips through the controller.
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    const uint32_t paramId = poly::ParamIDs::laneParam(0, poly::ParamIDs::kBaseVelocity);
    const double target = 0.42;
    const double defaultNorm = 100.0 / 127.0; // matches kLaneParamDefs default

    // Standard DAW automation contract: host mirrors the automated value into both the
    // processor (via IParameterChanges) AND the controller (via setParamNormalized). Our
    // injectParamChangeThroughProcess only touches the processor side; add the controller
    // side explicitly so this test matches production host behaviour and doesn't drift
    // out of sync with itself after the S01a fix serializes controller state too.
    host.injectParamChangeThroughProcess(paramId, target);
    host.setParamOnController(paramId, target);
    host.processBlock(0.0, 120.0, true);

    auto bytes = host.saveFullPluginState();
    ASSERT_FALSE(bytes.empty());

    // Mutate to force restore work — controller now holds a different value.
    host.setParamOnController(paramId, 0.99);
    ASSERT_NEAR(host.getParamOnController(paramId), 0.99, 1e-6);

    ASSERT_TRUE(host.loadFullPluginState(bytes));

    EXPECT_NEAR(host.getParamOnController(paramId), target, 0.1)
        << "Working path regression: param " << paramId << " through IParameterChanges did not round-trip. "
        << "Default=" << defaultNorm << " target=" << target << " actual=" << host.getParamOnController(paramId);

    host.teardown();
}

TEST(HostTests, SetParamOnController_NoProcess_ThenGetState_ReflectsChange) {
    // Broken path (pluginval fingerprint): a param change set only on the
    // controller — no IParameterChanges, no process() flush — is lost on
    // save→restore. Expected RED on current HEAD (post-T05, pre-S01a fix).
    //
    // On green (post-fix), the controller-cached value survives the round-trip
    // because either (a) the processor's getState reads controller state too,
    // or (b) the controller's own getState/setState covers scene params.
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    const uint32_t paramId = poly::ParamIDs::laneParam(0, poly::ParamIDs::kBaseVelocity);
    const double target = 0.42;
    const double defaultNorm = 100.0 / 127.0; // matches kLaneParamDefs default

    // Confirm we start at the default — sanity check.
    ASSERT_NEAR(host.getParamOnController(paramId), defaultNorm, 1e-6);

    // Set on controller ONLY. No IParameterChanges, no process().
    ASSERT_TRUE(host.setParamOnController(paramId, target));
    ASSERT_NEAR(host.getParamOnController(paramId), target, 1e-6)
        << "setParamNormalized should update controller cache";

    // saveFullPluginState mirrors what a JUCE-based host (pluginval, most DAWs) does:
    // it captures both processor and controller state. Without controller state serialized
    // via controller->getState, controller-only edits (no process() flush) are lost.
    auto bytes = host.saveFullPluginState();
    ASSERT_FALSE(bytes.empty());

    // Mutate further, then restore.
    ASSERT_TRUE(host.setParamOnController(paramId, 0.99));
    ASSERT_TRUE(host.loadFullPluginState(bytes));

    const double restored = host.getParamOnController(paramId);
    EXPECT_NEAR(restored, target, 0.1)
        << "Pluginval fingerprint match: controller-only param set was LOST on save→restore. "
        << "Set target=" << target << ", processor never saw it, saveState wrote default=" << defaultNorm
        << ", loadState + setComponentState pushed default back to controller cache=" << restored
        << ". This is the same class of bug pluginval reports as 'Param not restored on setStateInformation'.";

    host.teardown();
}

// --- M051 S06: preset-label persistence across project reload ---
//
// Regression for the UAT finding "plugin keeps opening with Init instead of the
// last-used preset". The cosmetic preset label lived only on the WebUIView
// (runtime, reset to "Init" on reload) and was never serialized. It now lives on
// the controller as v3 state fields (one per scene). This proves the SELECTED
// scene's label survives the controller getState/setState round-trip a DAW does
// on project save/reload.
TEST(HostTests, PresetLabel_SurvivesFullPluginStateRoundTrip) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    // Default before any preset is applied (scene A selected).
    EXPECT_EQ(host.controllerPresetLabel(), "Init");

    // Apply a preset label (as applyPreset does) and save.
    ASSERT_TRUE(host.setControllerPresetLabel("Tresillo Fire"));
    ASSERT_EQ(host.controllerPresetLabel(), "Tresillo Fire");

    auto bytes = host.saveFullPluginState();
    ASSERT_FALSE(bytes.empty());

    // Mutate the label to force restore to do real work, then reload.
    ASSERT_TRUE(host.setControllerPresetLabel("Something Else"));
    ASSERT_EQ(host.controllerPresetLabel(), "Something Else");

    ASSERT_TRUE(host.loadFullPluginState(bytes));

    EXPECT_EQ(host.controllerPresetLabel(), "Tresillo Fire")
        << "Preset label was lost on save->reload — the header would revert to a stale/Init "
           "value instead of the last-applied preset. v3 controller state must round-trip it.";

    host.teardown();
}

// --- M051 S06: per-scene preset labels (A and B independent, Morph blank) ---
//
// Regression for the UAT finding that a single per-plugin label is misleading:
// switching between scenes A and B showed the other scene's preset name. Labels
// are now tracked per scene. This proves A and B are independent, that the
// accessor tracks the selected scene, that Morph shows a blank label (an
// interpolation is not "a preset"), and that BOTH survive project reload.
TEST(HostTests, PresetLabel_PerSceneIndependentAndSurvivesReload) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    // Label scene A, then switch to B and label it differently.
    ASSERT_TRUE(host.setControllerScene(poly::SceneSelect::A));
    ASSERT_TRUE(host.setControllerPresetLabel("Tresillo Fire"));
    ASSERT_TRUE(host.setControllerScene(poly::SceneSelect::B));
    ASSERT_TRUE(host.setControllerPresetLabel("Gamelan Drift"));

    // Each scene reports its OWN label — no bleed-through.
    ASSERT_TRUE(host.setControllerScene(poly::SceneSelect::A));
    EXPECT_EQ(host.controllerPresetLabel(), "Tresillo Fire");
    ASSERT_TRUE(host.setControllerScene(poly::SceneSelect::B));
    EXPECT_EQ(host.controllerPresetLabel(), "Gamelan Drift");

    // Morph is a blend of A and B, not a preset — label is blank.
    ASSERT_TRUE(host.setControllerScene(poly::SceneSelect::Morph));
    EXPECT_EQ(host.controllerPresetLabel(), "")
        << "During Morph the header must not claim to be showing a single preset.";

    auto bytes = host.saveFullPluginState();
    ASSERT_FALSE(bytes.empty());

    // Corrupt both labels, then reload and confirm each scene's label restored.
    ASSERT_TRUE(host.setControllerScene(poly::SceneSelect::A));
    ASSERT_TRUE(host.setControllerPresetLabel("Wrong A"));
    ASSERT_TRUE(host.setControllerScene(poly::SceneSelect::B));
    ASSERT_TRUE(host.setControllerPresetLabel("Wrong B"));

    ASSERT_TRUE(host.loadFullPluginState(bytes));

    ASSERT_TRUE(host.setControllerScene(poly::SceneSelect::A));
    EXPECT_EQ(host.controllerPresetLabel(), "Tresillo Fire") << "Scene A label lost on reload.";
    ASSERT_TRUE(host.setControllerScene(poly::SceneSelect::B));
    EXPECT_EQ(host.controllerPresetLabel(), "Gamelan Drift") << "Scene B label lost on reload.";

    host.teardown();
}

// --- M046 S01a T02: pluginval-mimic corpus sweep ---
//
// Broad guard against the whole class of controller/processor param-sync bugs the
// PR #80 pluginval failure exposed. For each scene-affecting param the controller
// registers with kCanAutomate, set a distinct non-default value on the CONTROLLER
// side only (no IParameterChanges, no process()), take a full DAW-realistic
// save/restore round trip, and assert the value survives. Any bug that lets a
// controller-only param drift on save/restore will now fail here in <2s instead
// of surfacing on CI ~15 minutes after push.

namespace {

// Enumerates the automatable scene-affecting param IDs the controller exposes,
// mirroring the registerLaneParams + addParam calls in controller_base.cpp. Kept
// intentionally hand-listed (not reflection-based) so a param registration change
// forces a review of what the pluginval-mimic corpus covers.
std::vector<uint32_t> makePluginvalMimicParamIds() {
    std::vector<uint32_t> ids;
    for (int lane = 0; lane < poly::kMaxLanes; ++lane) {
        for (int offset = 0; offset < poly::ParamIDs::kLaneParamCount; ++offset)
            ids.push_back(poly::ParamIDs::laneParam(lane, offset));
    }
    for (int lane = 0; lane < poly::kMaxLanes; ++lane) {
        for (int offset = 0; offset < poly::ParamIDs::kCoreParamsPerLane; ++offset)
            ids.push_back(poly::ParamIDs::laneCoreParam(lane, offset));
    }
    ids.push_back(poly::ParamIDs::kMacroComplexity);
    ids.push_back(poly::ParamIDs::kMacroDensity);
    ids.push_back(poly::ParamIDs::kMacroSyncopation);
    ids.push_back(poly::ParamIDs::kMacroSwing);
    ids.push_back(poly::ParamIDs::kMacroTension);
    ids.push_back(poly::ParamIDs::kMacroHumanize);
    ids.push_back(poly::ParamIDs::kActiveLaneCount);
    ids.push_back(poly::ParamIDs::kSeed);
    ids.push_back(poly::ParamIDs::kSceneSelect);
    ids.push_back(poly::ParamIDs::kSceneMorph);
    ids.push_back(poly::ParamIDs::kChainEnabled);
    ids.push_back(poly::ParamIDs::kChainMode);
    ids.push_back(poly::ParamIDs::kChainEntryCount);
    for (int e = 0; e < poly::kMaxChainEntries; ++e) {
        ids.push_back(poly::ParamIDs::chainEntryParam(e, poly::ParamIDs::kChainEntryScene));
        ids.push_back(poly::ParamIDs::chainEntryParam(e, poly::ParamIDs::kChainEntryBars));
    }
    ids.push_back(poly::ParamIDs::kExportTrigger);
    ids.push_back(poly::ParamIDs::kCaptureLength);
    return ids;
}

// Distinct non-default normalized value, deterministic per index, wrapped clear
// of the (0.0, 0.1) and (0.9, 1.0) tolerance-adjacent edges pluginval avoids.
double pickTargetValue(size_t index) {
    double base = 0.37 + static_cast<double>(index) * 0.017;
    double frac = base - std::floor(base);
    return 0.05 + 0.9 * frac;
}

} // namespace

TEST(HostTests, PluginvalMimic_SaveRestoreParamRoundTrip) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    const auto ids = makePluginvalMimicParamIds();
    ASSERT_GT(ids.size(), 200u) << "Corpus sanity: expected ~250 params, got " << ids.size();

    struct Failure {
        uint32_t paramId;
        double target;
        double restored;
        const char* reason;
    };
    std::vector<Failure> failures;

    for (size_t i = 0; i < ids.size(); ++i) {
        const uint32_t id = ids[i];
        const double target = pickTargetValue(i);
        const double mutation = pickTargetValue(i + 137);

        if (!host.setParamOnController(id, target)) {
            failures.push_back({id, target, -1.0, "setParamNormalized failed"});
            continue;
        }
        // Refresh the processor's stateSnapshot_ (one-shot per publish today, drained by
        // getState). No IParameterChanges are queued for this call, so the controller-only
        // edit above stays out of sceneState_ — that's exactly the pluginval-fingerprint gap.
        host.processBlock(0.0, 120.0, false);
        auto bytes = host.saveFullPluginState();
        if (bytes.empty()) {
            failures.push_back({id, target, -1.0, "saveFullPluginState returned empty"});
            continue;
        }
        host.setParamOnController(id, mutation);
        if (!host.loadFullPluginState(bytes)) {
            failures.push_back({id, target, -1.0, "loadFullPluginState failed"});
            continue;
        }
        double restored = host.getParamOnController(id);
        if (std::abs(restored - target) > 0.1) {
            failures.push_back({id, target, restored, "not restored (>0.1 diff)"});
        }
    }

    if (!failures.empty()) {
        std::ostringstream oss;
        oss << "Pluginval-mimic reported " << failures.size() << "/" << ids.size()
            << " params NOT restored on setStateInformation. Same class of bug pluginval reports on CI.\n";
        constexpr size_t kMaxShown = 25;
        for (size_t i = 0; i < failures.size() && i < kMaxShown; ++i) {
            const auto& f = failures[i];
            oss << "  param=" << f.paramId << " target=" << f.target << " restored=" << f.restored << " (" << f.reason
                << ")\n";
        }
        if (failures.size() > kMaxShown)
            oss << "  ... (" << (failures.size() - kMaxShown) << " more)\n";
        ADD_FAILURE() << oss.str();
    }

    host.teardown();
}

// --- M046 S05 P7: SceneSelect-aware controller publish ---
//
// Broken path: on current HEAD, PolyControllerBase::setComponentState hardcodes
// `cachedState_.sceneA` at controller_base.cpp:225, so a preset that ships with
// select=SceneSelect::B is loaded into cachedState_ correctly but publishes
// Scene A values through setParamNormalized. Result: the UI shows the wrong scene
// on load. The fix is to route the publish through activeScene() (already used
// everywhere else in this file).

TEST(HostTests, SetComponentStateWithSceneB_ControllerReflectsSceneB) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    poly::SceneState scene{};
    scene.select = poly::SceneSelect::B;
    scene.morphAmount = 0.0f;
    // Deliberately distinct sceneA vs sceneB values across several publish sites
    // (macros, lane params, core params) so a hardcoded `.sceneA` read shows up
    // on multiple assertions rather than a single fluke.
    scene.sceneA.macros.complexity = 0.10f;
    scene.sceneA.macros.density = 0.20f;
    scene.sceneA.lanes[0].baseVelocity = 30;
    scene.sceneA.lanes[0].cycle.steps = 4;

    scene.sceneB.macros.complexity = 0.80f;
    scene.sceneB.macros.density = 0.90f;
    scene.sceneB.lanes[0].baseVelocity = 110;
    scene.sceneB.lanes[0].cycle.steps = 12;

    ASSERT_TRUE(host.feedComponentState(scene));

    const double complexity = host.getParamOnController(poly::ParamIDs::kMacroComplexity);
    const double density = host.getParamOnController(poly::ParamIDs::kMacroDensity);
    const double baseVel = host.getParamOnController(poly::ParamIDs::laneParam(0, poly::ParamIDs::kBaseVelocity));
    const double steps = host.getParamOnController(poly::ParamIDs::laneCoreParam(0, poly::ParamIDs::kCoreSteps));

    EXPECT_NEAR(complexity, 0.80, 1e-4) << "P7 fingerprint: select=B but controller published sceneA.complexity=0.10; "
                                        << "expected sceneB.complexity=0.80. See controller_base.cpp:225.";
    EXPECT_NEAR(density, 0.90, 1e-4) << "sceneB.density=0.90 was expected; sceneA.density=0.20 leaked through.";
    EXPECT_NEAR(baseVel, 110.0 / 127.0, 1e-4)
        << "sceneB.lanes[0].baseVelocity=110 was expected; sceneA.lanes[0].baseVelocity=30 leaked through.";
    EXPECT_NEAR(steps, (12 - 1) / 63.0, 1e-4)
        << "sceneB.lanes[0].cycle.steps=12 was expected; sceneA value=4 leaked through.";

    host.teardown();
}

TEST(HostTests, SetComponentStateWithSceneA_ControllerReflectsSceneA) {
    // Guard against a naive "always read sceneB" fix — the opposite regression.
    // Passes on current HEAD (sceneA IS what's hardcoded); MUST also pass after
    // the T02 fix flips to activeScene().
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    poly::SceneState scene{};
    scene.select = poly::SceneSelect::A;
    scene.sceneA.macros.complexity = 0.15f;
    scene.sceneB.macros.complexity = 0.85f;
    scene.sceneA.lanes[0].baseVelocity = 40;
    scene.sceneB.lanes[0].baseVelocity = 100;

    ASSERT_TRUE(host.feedComponentState(scene));

    EXPECT_NEAR(host.getParamOnController(poly::ParamIDs::kMacroComplexity), 0.15, 1e-4);
    EXPECT_NEAR(host.getParamOnController(poly::ParamIDs::laneParam(0, poly::ParamIDs::kBaseVelocity)), 40.0 / 127.0,
                1e-4);

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

// --- M046 S04 P5/P6: note-off integrity regressions ---
//
// P5 (engine/src/bridge.cpp:29): PendingNoteOffBuffer::flushDue treats entries with
// ppqOff < ppqStart as "future work" instead of "late". They sit forever, and any
// tempo ramp / transport jump / block-boundary rounding that skips past a scheduled
// off produces a stuck note. Fix (T02): drop the lower bound; late offs emit at
// block sample 0 as best-effort late offs.
//
// P6 (plugin/source/processor.cpp:194): emitMidiOutput calls pendingNoteOffs_.push()
// but ignores the bool return. When the buffer hits kCapacity (512) the push silently
// fails — the note-on IS emitted, but no off is ever scheduled. Fix (T03): check the
// return; on overflow bump noteOffDrops_ AND emit an immediate best-effort off in the
// same block, so a full buffer means "short notes" instead of "stuck notes".

TEST(HostTests, PendingStragglerBelowPpqStart_EmittedInNextBlock) {
    // Poke a straggler directly into pendingNoteOffs_ with ppqOff = 0.5, then run one
    // playing block at [2.0, 2.0 + ppqPerBlock). On HEAD the straggler stays parked
    // forever because flushDue's guard requires ppqOff >= ppqStart. Post-T02 the guard
    // is removed and the straggler surfaces as an off at sample 0 of the block.
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    const int16_t kPitch = 36;
    const int16_t kChannel = 0;
    host.injectPendingNoteOff(0.5, kPitch, kChannel);
    host.clearEvents();

    host.processBlock(2.0, 120.0, true);

    bool foundStragglerOff = false;
    for (const auto& e : host.events()) {
        if (e.type == MidiEvent::NoteOff && e.pitch == kPitch && e.channel == kChannel) {
            foundStragglerOff = true;
            break;
        }
    }

    EXPECT_TRUE(foundStragglerOff) << "P5 (M046 S04): a note-off scheduled at ppqOff=0.5 was never "
                                      "emitted after processing a block at ppqStart=2.0. flushDue's "
                                      "`ppqOff >= ppqStart` lower bound leaves late offs stuck in the "
                                      "buffer. Fix removes the lower bound so stragglers emit at block "
                                      "sample 0.";

    host.teardown();
}

TEST(HostTests, NoteOffOverflow_DropCounterIncrementsAndImmediateOffEmitted) {
    // Prefill pendingNoteOffs_ to kCapacity with far-future entries so the ring never
    // drains during the test window. Then drive a playing block that generates note-ons.
    // The scheduling code path at processor.cpp:194 (pendingNoteOffs_.push) will silently
    // fail for every generated note-on. On HEAD: drops=0, no immediate off, stuck notes.
    // Post-T03: drops>0, an immediate off for each dropped push, note-off count matches
    // dropped count.
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    const int16_t kFillerPitch = 100;
    for (size_t i = 0; i < poly::PendingNoteOffBuffer::kCapacity; ++i) {
        host.injectPendingNoteOff(1e6, kFillerPitch, 0); // far future — never drains in-window
    }
    ASSERT_EQ(host.noteOffDrops(), 0u) << "drop counter must be zero at start";
    host.clearEvents();

    // Play long enough to guarantee at least one note-on gets generated.
    host.playBars(1.0, 120.0);

    const auto onCount = host.noteOnEvents().size();
    ASSERT_GT(onCount, 0u) << "default patch should produce note-ons in 1 bar";

    EXPECT_GT(host.noteOffDrops(), 0u) << "P6 (M046 S04): buffer full but pendingNoteOffs_.push() bool "
                                          "return is discarded at processor.cpp:194 — the failed push "
                                          "is silent. Fix bumps noteOffDrops_ so overflow is accounted.";

    // Immediate off invariant: every dropped push produces a best-effort off in the
    // same block. On HEAD there are no note-offs at all for the newly-generated pitches,
    // only stragglers from the filler pitch (which never drain because ppqOff=1e6).
    size_t immediateOffsForOnPitches = 0;
    for (const auto& on : host.noteOnEvents()) {
        for (const auto& off : host.noteOffEvents()) {
            if (off.pitch == on.pitch && off.channel == on.channel) {
                ++immediateOffsForOnPitches;
                break;
            }
        }
    }
    EXPECT_GE(immediateOffsForOnPitches, host.noteOffDrops())
        << "P6 (M046 S04): the fix must also emit an immediate best-effort off per dropped push so "
           "the DAW hears a short note rather than a stuck one. Currently: "
        << immediateOffsForOnPitches << " immediate offs vs " << host.noteOffDrops() << " drops.";

    host.teardown();
}

// --- M046 S03 P4: UI-to-audio handshake TOCTOU regressions ---
//
// Invariant under test (target for S03 T02): for every UI-to-audio handshake site,
// notify-issued == applied + drops. Two rapid writes with no processBlock() between
// them therefore MUST show drops == 1 (writer detected the previous unconsumed publish
// and accounted for the overwrite).
//
// On current HEAD both tests fail: the writer silently stomps pending without
// incrementing the drop counter. That is the P4 silent-loss defect this slice
// eliminates.

TEST(HostTests, NotifyBurstBetweenProcess_LandsBothOrDropsCleanly) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    // Two distinct note maps issued via notify() back-to-back, no process() between.
    std::array<int16_t, 128> mapA{};
    std::array<int16_t, 128> mapB{};
    for (int i = 0; i < 128; ++i) {
        mapA[static_cast<size_t>(i)] = static_cast<int16_t>((i + 30) % 128);
        mapB[static_cast<size_t>(i)] = static_cast<int16_t>((i + 90) % 128);
    }

    auto baseline = host.handshakeDrops();
    ASSERT_EQ(baseline.noteMap, 0u) << "handshake drop counter is not zeroed at setup";

    host.injectNoteMap(mapA);
    host.injectNoteMap(mapB); // second write with no process() between — should be counted

    // Drain the pending map (whichever version survives) into sceneState_.
    host.processBlock(0.0, 120.0, false);

    // Invariant: issued=2, applied=1 (only one survives sceneState_), therefore drops=1.
    // On HEAD: drops=0 (silent loss). Post-S03-T02: drops=1.
    auto after = host.handshakeDrops();
    EXPECT_EQ(after.noteMap, 1u) << "P4 silent-loss: two rapid NoteMapUpdate notifies with no process() between "
                                    "must count as one apply + one drop; got drops="
                                 << after.noteMap;

    host.teardown();
}

TEST(HostTests, SetStateBurst_TearOrLoss) {
    // Prepare two distinct serialized scene states so we have two clearly-different
    // payloads to feed into setState back-to-back.
    std::vector<uint8_t> bytesA;
    std::vector<uint8_t> bytesB;
    {
        PolyTestHost prep;
        ASSERT_TRUE(prep.setup(44100.0, 512));
        std::array<int16_t, 128> mapA{};
        for (int i = 0; i < 128; ++i)
            mapA[static_cast<size_t>(i)] = static_cast<int16_t>((i + 20) % 128);
        prep.injectNoteMap(mapA);
        prep.processBlock(0.0, 120.0, false);
        bytesA = prep.saveState();
        prep.teardown();
    }
    {
        PolyTestHost prep;
        ASSERT_TRUE(prep.setup(44100.0, 512));
        std::array<int16_t, 128> mapB{};
        for (int i = 0; i < 128; ++i)
            mapB[static_cast<size_t>(i)] = static_cast<int16_t>((i + 100) % 128);
        prep.injectNoteMap(mapB);
        prep.processBlock(0.0, 120.0, false);
        bytesB = prep.saveState();
        prep.teardown();
    }
    ASSERT_FALSE(bytesA.empty());
    ASSERT_FALSE(bytesB.empty());
    ASSERT_NE(bytesA, bytesB) << "prep-run states are identical — burst assertion cannot distinguish them";

    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    auto baseline = host.handshakeDrops();
    ASSERT_EQ(baseline.state, 0u) << "handshake drop counter is not zeroed at setup";

    // Two rapid setState calls, no process() between — writer overwrites pendingState_
    // before the audio thread drains it.
    ASSERT_TRUE(host.loadState(bytesA));
    ASSERT_TRUE(host.loadState(bytesB));

    // Drain pendingState_ into sceneState_.
    host.processBlock(0.0, 120.0, false);

    // Invariant: issued=2, applied=1 (only one setState survives), therefore drops=1.
    // On HEAD: drops=0. Post-S03-T02: drops=1.
    auto after = host.handshakeDrops();
    EXPECT_EQ(after.state, 1u) << "P4 silent-loss: two rapid setState calls with no process() between must count as "
                                  "one apply + one drop; got drops="
                               << after.state;

    host.teardown();
}

// --- M046 S03 T03: Threaded stress — the sketch DoD gate ---
//
// Runs a writer thread hammering all seven host→RT handshakes with distinct
// increment-only payloads while the main thread runs processBlock() in a tight
// loop. After both threads quiesce, drains any lingering publishes then asserts:
//
//   1. Silent-loss invariant: issued == applied + drops for every handshake.
//   2. Contention progress: applied > 0 for every non-state handshake (reader is
//      not starved).
//   3. Torn-read spot check: the final applied noteMap is bit-exactly identical
//      to some issued writeId's payload (proves no torn slot copy landed).
//
// Torn-read encoding: writer sets map[i] = static_cast<int16_t>(((writeId * 31) ^ i) & 0x7FFF).
// If any field of the applied map was written by a different writeId (i.e. the
// reader saw a mid-flight publish), the check on map[0]->reconstruct fails.
//
// Runtime target < 5s: N=10000 iterations per handshake × 7 handshakes with 512-sample
// blocks at 44.1kHz produces ~5MB of payload traffic on the writer side. Empirically
// completes in < 2s on M-series Macs, giving 2.5× headroom below the ctest cap.
//
// TSan-guarded twin (M046 S03 T03 + M050): the same body under __has_feature(thread_sanitizer)
// is where TSan gets to detect any residual data race. In non-TSan builds the invariant
// checks alone stand as the safety net.

namespace stress {

constexpr int kStressIterations = 100000;

int16_t encodeNoteMapField(uint32_t writeId, int fieldIdx) {
    return static_cast<int16_t>(((writeId * 31u) ^ static_cast<uint32_t>(fieldIdx)) & 0x7FFFu);
}

std::array<int16_t, 128> makeNoteMap(uint32_t writeId) {
    std::array<int16_t, 128> map{};
    for (int i = 0; i < 128; ++i)
        map[static_cast<size_t>(i)] = encodeNoteMapField(writeId, i);
    return map;
}

std::array<int, poly::kMaxSteps> makeCellSizes(uint32_t writeId) {
    std::array<int, poly::kMaxSteps> sizes{};
    for (int i = 0; i < poly::kMaxSteps; ++i)
        sizes[static_cast<size_t>(i)] = static_cast<int>((writeId + i) % 8u) + 1;
    return sizes;
}

std::array<bool, poly::kMaxSteps> makeTimeline(uint32_t writeId) {
    std::array<bool, poly::kMaxSteps> pattern{};
    for (int i = 0; i < poly::kMaxSteps; ++i)
        pattern[static_cast<size_t>(i)] = ((writeId + static_cast<uint32_t>(i)) & 1u) != 0u;
    return pattern;
}

std::array<float, poly::kMaxSteps> makeMicroTiming(uint32_t writeId) {
    std::array<float, poly::kMaxSteps> timing{};
    for (int i = 0; i < poly::kMaxSteps; ++i)
        timing[static_cast<size_t>(i)] = static_cast<float>(((writeId + static_cast<uint32_t>(i)) % 40u)) - 20.0f;
    return timing;
}

std::array<float, poly::kMaxSteps> makeAccentMask(uint32_t writeId) {
    std::array<float, poly::kMaxSteps> accents{};
    for (int i = 0; i < poly::kMaxSteps; ++i)
        accents[static_cast<size_t>(i)] = static_cast<float>((writeId + static_cast<uint32_t>(i)) % 128u) / 127.0f;
    return accents;
}

poly::Envelope makeEnvelope(uint32_t writeId) {
    poly::Envelope env{};
    env.periodBars = 1.0f + static_cast<float>(writeId % 16u);
    env.depth = static_cast<float>(writeId % 100u) / 100.0f;
    env.phaseOffset = static_cast<float>(writeId % 360u);
    env.curvature = static_cast<float>((writeId + 1u) % 100u) / 100.0f;
    for (int i = 0; i < poly::kMaxStepListEntries; ++i)
        env.stepValues[static_cast<size_t>(i)] = static_cast<float>((writeId + static_cast<uint32_t>(i)) % 100u);
    env.stepCount = static_cast<int>(writeId % 8u);
    return env;
}

} // namespace stress

TEST(HostTests, HandshakeStress_NoTearNoLoss) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    // Prepare one setState payload we replay on every iteration — cheaper than
    // regenerating per-call. The setState handshake carries a full SceneState so
    // per-iteration payload variation isn't necessary to exercise the exchange.
    std::vector<uint8_t> stateBytes;
    {
        PolyTestHost prep;
        ASSERT_TRUE(prep.setup(44100.0, 512));
        prep.injectNoteMap(stress::makeNoteMap(1u));
        prep.processBlock(0.0, 120.0, false);
        stateBytes = prep.saveState();
        prep.teardown();
    }
    ASSERT_FALSE(stateBytes.empty());

    std::atomic<bool> writerDone{false};
    std::atomic<uint32_t> lastNoteMapWriteId{0};

    struct Issued {
        uint64_t state = 0;
        uint64_t noteMap = 0;
        uint64_t cellSizes = 0;
        uint64_t timeline = 0;
        uint64_t microTiming = 0;
        uint64_t envelope = 0;
        uint64_t accentMask = 0;
    } issued{};

    const auto tStart = std::chrono::steady_clock::now();

    std::thread writer([&] {
        constexpr int kLane = 0;
        constexpr int kEnvSlot = 0;
        for (uint32_t i = 1; i <= static_cast<uint32_t>(stress::kStressIterations); ++i) {
            host.injectNoteMap(stress::makeNoteMap(i));
            lastNoteMapWriteId.store(i, std::memory_order_relaxed);
            ++issued.noteMap;

            host.injectCellSizes(kLane, stress::makeCellSizes(i));
            ++issued.cellSizes;

            host.injectTimelinePattern(kLane, stress::makeTimeline(i), static_cast<int>(i % poly::kMaxSteps));
            ++issued.timeline;

            host.injectMicroTiming(kLane, stress::makeMicroTiming(i));
            ++issued.microTiming;

            host.injectEnvelope(kLane, kEnvSlot, (i & 1u) != 0u, stress::makeEnvelope(i));
            ++issued.envelope;

            host.injectAccentMask(kLane, stress::makeAccentMask(i));
            ++issued.accentMask;

            // setState is expensive (allocates MemoryStream, serializes SceneState).
            // Fire it 1-in-100 so it still stresses the exchange without dominating
            // wall-clock. Result: ~100 setState issues per stress run.
            if ((i % 100u) == 0u) {
                host.loadState(stateBytes);
                ++issued.state;
            }
        }
        writerDone.store(true, std::memory_order_release);
    });

    // Reader loop — processBlock() drains all seven slots each iteration. Keep going
    // a bit after writerDone so any late-published slots get drained too.
    double ppq = 0.0;
    int postDrainBlocks = 0;
    while (!writerDone.load(std::memory_order_acquire) || postDrainBlocks < 200) {
        host.processBlock(ppq, 120.0, false);
        ppq += host.ppqPerBlock(120.0);
        if (writerDone.load(std::memory_order_acquire))
            ++postDrainBlocks;
    }
    writer.join();

    const auto tEnd = std::chrono::steady_clock::now();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();

    // Assertion 1 — silent-loss invariant: issued == applied + drops.
    auto drops = host.handshakeDrops();
    auto applied = host.handshakeApplied();

    EXPECT_EQ(issued.noteMap, applied.noteMap + drops.noteMap)
        << "noteMap silent-loss (issued=" << issued.noteMap << " applied=" << applied.noteMap
        << " drops=" << drops.noteMap << ")";
    EXPECT_EQ(issued.cellSizes, applied.cellSizes + drops.cellSizes)
        << "cellSizes silent-loss (issued=" << issued.cellSizes << " applied=" << applied.cellSizes
        << " drops=" << drops.cellSizes << ")";
    EXPECT_EQ(issued.timeline, applied.timeline + drops.timeline)
        << "timeline silent-loss (issued=" << issued.timeline << " applied=" << applied.timeline
        << " drops=" << drops.timeline << ")";
    EXPECT_EQ(issued.microTiming, applied.microTiming + drops.microTiming)
        << "microTiming silent-loss (issued=" << issued.microTiming << " applied=" << applied.microTiming
        << " drops=" << drops.microTiming << ")";
    EXPECT_EQ(issued.envelope, applied.envelope + drops.envelope)
        << "envelope silent-loss (issued=" << issued.envelope << " applied=" << applied.envelope
        << " drops=" << drops.envelope << ")";
    EXPECT_EQ(issued.accentMask, applied.accentMask + drops.accentMask)
        << "accentMask silent-loss (issued=" << issued.accentMask << " applied=" << applied.accentMask
        << " drops=" << drops.accentMask << ")";
    EXPECT_EQ(issued.state, applied.state + drops.state)
        << "state silent-loss (issued=" << issued.state << " applied=" << applied.state << " drops=" << drops.state
        << ")";

    // Assertion 2 — reader not starved: at least one apply for every hot handshake.
    // Not all writes need to survive drops; but zero applies would mean the reader
    // never won any race, which itself is a red flag.
    EXPECT_GT(applied.noteMap, 0u) << "reader starved: applied.noteMap == 0";
    EXPECT_GT(applied.cellSizes, 0u) << "reader starved: applied.cellSizes == 0";
    EXPECT_GT(applied.timeline, 0u) << "reader starved: applied.timeline == 0";
    EXPECT_GT(applied.microTiming, 0u) << "reader starved: applied.microTiming == 0";
    EXPECT_GT(applied.envelope, 0u) << "reader starved: applied.envelope == 0";
    EXPECT_GT(applied.accentMask, 0u) << "reader starved: applied.accentMask == 0";

    // Assertion 3 — torn-read spot check on the final noteMap. Any bit-mismatch
    // between the observed map and the deterministic encoding for its own map[0]
    // means the reader saw fields written by different writeIds — the classic
    // torn-slot symptom.
    auto finalState = deserializeSceneState(host.saveState());
    const auto& finalMap = finalState.noteMap.map;
    const uint32_t observedField0 = static_cast<uint32_t>(finalMap[0]) & 0x7FFFu;
    // Recover writeId candidates from field 0: encodeNoteMapField(writeId, 0) == (writeId * 31) & 0x7FFF.
    // For any 32-bit writeId whose (writeId * 31) low 15 bits equal observedField0, we accept it as
    // the candidate. Instead of full recovery, we just check consistency: given map[0] we derive
    // the "shape" (writeId * 31) and verify map[i] XORs consistently.
    const uint32_t writeIdTimes31 = observedField0; // low 15 bits of writeId * 31
    for (int i = 1; i < 128; ++i) {
        const uint32_t expected = (writeIdTimes31 ^ static_cast<uint32_t>(i)) & 0x7FFFu;
        const uint32_t actual = static_cast<uint32_t>(finalMap[static_cast<size_t>(i)]) & 0x7FFFu;
        ASSERT_EQ(expected, actual) << "torn-read: final noteMap[" << i << "]=" << actual << " but map[0] implies "
                                    << expected << " (writeIdTimes31=" << writeIdTimes31 << ")";
    }

    std::cerr << "[HandshakeStress] elapsed=" << elapsedMs << "ms iters=" << stress::kStressIterations
              << " issued.noteMap=" << issued.noteMap << " applied.noteMap=" << applied.noteMap
              << " drops.noteMap=" << drops.noteMap << " (last writeId=" << lastNoteMapWriteId.load() << ")\n";

    host.teardown();
}

// TSan-guarded twin for M050's ThreadSanitizer job. Same body as
// HandshakeStress_NoTearNoLoss but at a reduced iteration count (TSan adds
// 5-15× runtime + memory overhead — a full 100k run would time out ctest).
// The invariant assertions here are redundant with the main test; the point is
// that TSan gets to instrument the reader↔writer race and flag any residual
// data race the invariant checks can't observe (e.g. same-slot writer↔reader
// overlap during in-flight commit that happens to produce a self-consistent
// but still torn payload).
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define POLY_TSAN_ACTIVE 1
#endif
#endif

#ifdef POLY_TSAN_ACTIVE
TEST(HostTests, HandshakeStress_TSanClean) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    constexpr int kTSanIterations = 5000;
    std::atomic<bool> writerDone{false};
    uint64_t issuedNoteMap = 0;

    std::thread writer([&] {
        for (uint32_t i = 1; i <= static_cast<uint32_t>(kTSanIterations); ++i) {
            host.injectNoteMap(stress::makeNoteMap(i));
            host.injectCellSizes(0, stress::makeCellSizes(i));
            host.injectTimelinePattern(0, stress::makeTimeline(i), static_cast<int>(i % poly::kMaxSteps));
            host.injectMicroTiming(0, stress::makeMicroTiming(i));
            host.injectEnvelope(0, 0, (i & 1u) != 0u, stress::makeEnvelope(i));
            host.injectAccentMask(0, stress::makeAccentMask(i));
            ++issuedNoteMap;
        }
        writerDone.store(true, std::memory_order_release);
    });

    double ppq = 0.0;
    int postDrainBlocks = 0;
    while (!writerDone.load(std::memory_order_acquire) || postDrainBlocks < 100) {
        host.processBlock(ppq, 120.0, false);
        ppq += host.ppqPerBlock(120.0);
        if (writerDone.load(std::memory_order_acquire))
            ++postDrainBlocks;
    }
    writer.join();

    auto drops = host.handshakeDrops();
    auto applied = host.handshakeApplied();
    EXPECT_EQ(issuedNoteMap, applied.noteMap + drops.noteMap);

    host.teardown();
}
#endif

// --- M046 S06 P9: loop-wrap MIDI-capture preservation (RED on HEAD) ---
//
// Root cause: PolyProcessor::updateTransportContext at processor.cpp:104 sets
// `tc_.jumped = |ppqStart − expectedNextPpq_| > 0.001` — a fixed threshold that
// treats any transport discontinuity as a jump. When Cubase (or any host) loops
// back from loopEnd to loopStart, `expectedNextPpq_ ≈ loopEnd` while next
// `ppqStart = loopStart`, and the delta far exceeds 0.001. `handleTransportJump`
// then clears `captureBuffer_` at processor.cpp:141, dropping every captured
// note across the wrap — the P9 defect. This is silent: no counter fires.
//
// Test 1: seed the capture buffer, drive one wrap block, and assert the seeded
// notes survive. RED today because handleTransportJump wipes them.
// Test 2 (guard): a real project-time jump (user drags playhead) SHOULD still
// clear — proves the fix doesn't over-broaden the guard and let stragglers
// linger across genuine jumps.
TEST(HostTests, LoopWrap_CaptureBufferPreservedAcrossPasses) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    constexpr double kTempo = 120.0;
    constexpr double kLoopStart = 0.0;
    constexpr double kLoopEnd = 4.0;

    // Warmup: prime expectedNextPpq_ inside the loop range. First block sees
    // expectedNextPpq_ = -1 → jumped stays false; the next-block ppqStart
    // becomes the reference for wrap detection.
    host.processBlock(3.5, kTempo, /*playing=*/true, /*looping=*/true, kLoopStart, kLoopEnd);

    // Seed synthetic captured notes with distinct pitches so any post-wrap
    // engine adds don't collide accidentally. The warmup block may itself have
    // emitted engine-generated notes into the capture buffer, so track the
    // seeded delta relative to whatever's already there.
    const size_t preSeedCount = host.capturedNoteCount();
    constexpr size_t kSeeded = 10;
    for (size_t i = 0; i < kSeeded; ++i) {
        poly::NoteEvent note{};
        note.ppqPosition = 3.5 + 0.01 * static_cast<double>(i);
        note.pitch = static_cast<int16_t>(96 + i);
        note.velocity = 0.75f;
        note.duration = 0.05;
        note.channel = 0;
        note.laneIndex = 0;
        host.pushCapturedNote(note);
    }
    const size_t postSeedCount = host.capturedNoteCount();
    ASSERT_EQ(postSeedCount, preSeedCount + kSeeded) << "seed helper should populate captureBuffer_";

    // Loop wrap: ppqStart snaps back to loopStart. expectedNextPpq_ ≈ 3.6, so
    // `|0.0 - 3.6| = 3.6 >> 0.001` trips the jump detector on HEAD → capture
    // buffer cleared. After S06 T02, wrap-aware detection preserves it.
    host.processBlock(kLoopStart, kTempo, /*playing=*/true, /*looping=*/true, kLoopStart, kLoopEnd);

    // Allow for engine-generated adds in the wrap block; assert the seeded
    // notes survived. RED on HEAD (buffer cleared → count << postSeedCount).
    EXPECT_GE(host.capturedNoteCount(), postSeedCount)
        << "P9 regression: loop wrap cleared captureBuffer_ (expected >= " << postSeedCount << ", got "
        << host.capturedNoteCount() << ")";

    host.teardown();
}

TEST(HostTests, NonWrapJump_CaptureBufferCleared) {
    // Guard against over-fix: a genuine project-time jump (not a loop wrap)
    // MUST still clear the capture buffer so stragglers don't leak across a
    // scrub. Passes on HEAD and must keep passing after T02.
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    constexpr double kTempo = 120.0;

    host.processBlock(0.0, kTempo, /*playing=*/true);
    host.processBlock(0.1, kTempo, /*playing=*/true);

    constexpr size_t kSeeded = 5;
    for (size_t i = 0; i < kSeeded; ++i) {
        poly::NoteEvent note{};
        note.ppqPosition = 0.1 + 0.01 * static_cast<double>(i);
        note.pitch = static_cast<int16_t>(60 + i);
        note.velocity = 0.6f;
        note.duration = 0.05;
        host.pushCapturedNote(note);
    }
    ASSERT_EQ(host.capturedNoteCount(), kSeeded);

    // Real jump: playhead scrubbed forward to bar 16, no looping active. Trips
    // jumped=true with wrappedLoop=false — must still clear.
    host.processBlock(64.0, kTempo, /*playing=*/true, /*looping=*/false);

    EXPECT_LT(host.capturedNoteCount(), kSeeded)
        << "non-wrap jump must not preserve capture (count=" << host.capturedNoteCount() << ")";

    host.teardown();
}

// --- M046 S07 T01: export trigger + event-sort regressions (RED on HEAD) ---
//
// P10: kExportTrigger is a momentary edge — 0->1 tells process() to snapshot the
// capture buffer. The consume path clears exportTriggered_ locally but never
// bounces the param back to 0 via outputParameterChanges. A dedupe-aware host
// (Cubase, JUCE) that caches the last written value never observes the fall,
// so a second Export click sees "value unchanged, skip" and no MIDI export
// fires. T02 fixes both consume sites (stopped path ~461, playing path ~516).

TEST(HostTests, ExportTrigger_ResetsAfterCommit_StoppedPath) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    host.injectParamChangeThroughProcess(poly::ParamIDs::kExportTrigger, 1.0);
    host.processBlock(0.0, 120.0, /*playing=*/false);

    auto v = host.lastOutputParamValue(poly::ParamIDs::kExportTrigger);
    ASSERT_TRUE(v.has_value()) << "M046 S07 P10 fingerprint (stopped path): kExportTrigger not bounced back "
                                  "via outputParameterChanges. Dedupe-aware host never sees the 1->0 fall, "
                                  "so a second Export click does nothing. T02 must add the bounce at both "
                                  "consume sites (stopped-path ~461, playing-path ~516).";
    EXPECT_DOUBLE_EQ(*v, 0.0);

    host.teardown();
}

TEST(HostTests, ExportTrigger_ResetsAfterCommit_PlayingPath) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    host.injectParamChangeThroughProcess(poly::ParamIDs::kExportTrigger, 1.0);
    host.processBlock(0.0, 120.0, /*playing=*/true);

    auto v = host.lastOutputParamValue(poly::ParamIDs::kExportTrigger);
    ASSERT_TRUE(v.has_value()) << "M046 S07 P10 fingerprint (playing path): kExportTrigger not bounced back "
                                  "via outputParameterChanges. Same defect as the stopped-path variant — "
                                  "T02 must fix both consume sites.";
    EXPECT_DOUBLE_EQ(*v, 0.0);

    host.teardown();
}

// --- M032 S03 T02: offline export tempo mirrors the host tempo ---
//
// renderCurrentPatternSmf (web_ui_view.cpp) used to hardcode 120.0 for the SMF
// tempo meta because the UI/message thread had no host-tempo source (MEM054).
// T01 published ctx.tempo into UISnapshot.tempoBpm every process() block; T02
// makes the offline export read it. This pins the integration end-to-end: drive
// the processor at a non-default tempo, confirm the controller's UISnapshot (the
// exact surface renderCurrentPatternSmf reads) carries that tempo, then render
// through the same primitive + snapshot fields the export path uses and assert
// the SMF tempo meta encodes the host tempo, not the old 120 BPM fallback.

TEST(HostTests, ExportTempoMirrorsHostTempo) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    constexpr double kHostTempo = 150.0; // 60,000,000 / 150 = 400000 us/qtr, clean
    constexpr uint32_t kExpectedUsPerQuarter = 400000u;
    constexpr uint32_t kDefaultUsPerQuarter = 500000u; // 120 BPM — the old hardcoded fallback

    // One process() block at the host tempo publishes ctx.tempo into the snapshot
    // (processor.cpp: uiSnapshot_.tempoBpm.store(tc_.tempo, ...) every block).
    host.processBlock(0.0, kHostTempo, /*playing=*/true);

    auto* snap = host.controllerUiSnapshot();
    ASSERT_NE(snap, nullptr) << "controller must hold the processor's UISnapshot after connect";
    const double snapTempo = snap->tempoBpm.load(std::memory_order_relaxed);
    EXPECT_DOUBLE_EQ(snapTempo, kHostTempo)
        << "UISnapshot.tempoBpm must mirror the host tempo — this is the surface renderCurrentPatternSmf reads.";

    // Render through the same primitive + snapshot fields renderCurrentPatternSmf
    // uses. A default scene still emits the conductor track carrying the tempo meta.
    const int bars = snap->captureBars.load(std::memory_order_relaxed);
    const int tsNum = snap->timeSigNumerator.load(std::memory_order_relaxed);
    const int tsDen = snap->timeSigDenominator.load(std::memory_order_relaxed);
    poly::SceneState scene{};
    const auto smf = poly::renderPatternToSMF(scene, bars, snapTempo, tsNum, tsDen, /*laneFilter=*/-1);

    // Conductor track 0 opens with delta 0 + FF 51 03, so usPerQuarter is bytes 26..28
    // (same offset offline_render_tests::extractTempoMeta reads).
    ASSERT_GE(smf.size(), 29u) << "conductor track tempo meta must be present";
    const uint32_t usPerQuarter =
        (static_cast<uint32_t>(smf[26]) << 16) | (static_cast<uint32_t>(smf[27]) << 8) | static_cast<uint32_t>(smf[28]);

    EXPECT_EQ(usPerQuarter, kExpectedUsPerQuarter)
        << "Exported SMF tempo meta must encode the host tempo (" << kHostTempo << " BPM = " << kExpectedUsPerQuarter
        << " us/qtr), got " << usPerQuarter << ".";
    EXPECT_NE(usPerQuarter, kDefaultUsPerQuarter)
        << "Regression guard: export must not fall back to the hardcoded 120 BPM (500000 us/qtr) "
           "when the host runs at a different tempo.";

    host.teardown();
}

// P12: emitMidiOutput forwards flushDue output straight to addEvent in the order
// flushDue returns entries. flushDue uses swap-remove compaction (bridge.cpp:35),
// so the emitted sequence isn't monotonic in sampleOffset — some hosts (JUCE-based
// wrappers and older Bitwig builds) reject or reorder unsorted events, which can
// drop MIDI notes. T03 fixes this with a stable_sort over a stack-allocated scratch
// array inside emitMidiOutput.
//
// Reproduction: inject 4 stragglers whose ppqOff values sit inside a single block
// window (tempo=120, SR=44100, block=512 => block spans PPQ [0, 0.02322)). Insertion
// order below is designed so flushDue's swap-remove yields sample offsets
// [441, 220, 331, 110] — clearly non-monotonic. After T03 the sequence is
// [110, 220, 331, 441] and the assertion passes.

TEST(HostTests, OutputEvents_SortedBySampleOffset) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    // ppqOff -> expected sampleOffset at tempo=120, SR=44100:
    //   0.020 -> round(0.020 * 22050) = 441
    //   0.005 -> round(0.005 * 22050) = 110
    //   0.015 -> round(0.015 * 22050) = 331
    //   0.010 -> round(0.010 * 22050) = 220
    host.injectPendingNoteOff(0.020, 60, 0);
    host.injectPendingNoteOff(0.005, 61, 0);
    host.injectPendingNoteOff(0.015, 62, 0);
    host.injectPendingNoteOff(0.010, 63, 0);

    host.processBlock(0.0, 120.0, /*playing=*/true);

    auto offs = host.noteOffEvents();
    ASSERT_EQ(offs.size(), 4u) << "flushDue should emit all 4 injected stragglers inside this block window "
                                  "(size="
                               << offs.size() << ")";

    int32_t prevOffSample = -1;
    for (size_t i = 0; i < offs.size(); ++i) {
        EXPECT_GE(offs[i].sampleOffset, prevOffSample)
            << "M046 S07 P12 fingerprint: off " << i << " (pitch=" << offs[i].pitch
            << ") sampleOffset=" << offs[i].sampleOffset << " < prev=" << prevOffSample
            << ". emitMidiOutput forwards flushDue's swap-remove order straight to "
               "addEvent — T03 must buffer + stable_sort by sampleOffset.";
        prevOffSample = offs[i].sampleOffset;
    }

    // Stronger claim: ALL events (offs + any engine-generated note-ons) must be
    // monotonically non-decreasing by sampleOffset. T03's single stable_sort over
    // the union guarantees this; the per-off assertion above is a weaker subset.
    int32_t prevSample = -1;
    for (const auto& ev : host.events()) {
        EXPECT_GE(ev.sampleOffset, prevSample)
            << "event " << (ev.type == MidiEvent::NoteOn ? "on" : "off") << " pitch=" << ev.pitch
            << " sampleOffset=" << ev.sampleOffset << " < prev=" << prevSample;
        prevSample = ev.sampleOffset;
    }

    host.teardown();
}

// --- M051 S08 T02: arm -> capture -> complete state machine ---
//
// The processor gates WebUI MIDI export behind an explicit four-state machine
// (idle -> armed -> capturing -> complete, Reset from any state). These tests
// drive it headlessly through the ArmCapture/ResetCapture notify() surface the
// WebUI bridge (T03) will use, and assert every transition, Reset from each
// state, that the frozen export window is populated only in `complete`, that it
// is byte-stable after further play (the whole S08 promise), and the transport
// stop-pauses / jump-cancels / tempo-change-cancels policies. Absolute-PPQ
// timing throughout (MEM005) — the latch and N-bar window derive from
// projectTimeMusic, never an accumulated block counter.

namespace {

// kCaptureLength normalized value for an N-bar window. The processor maps
// captureLengthBars_ = 1 + round(norm * 31), so norm = (N-1)/31.
double captureLenNorm(int bars) {
    return static_cast<double>(bars - 1) / 31.0;
}

// Play forward from `startPpq` in contiguous blocks until `pred()` is true (or
// the guard trips). Returns the ppq of the block following the last processed.
template <typename Pred>
double playUntil(PolyTestHost& host, double startPpq, double tempo, Pred pred, int maxBlocks = 4000) {
    double ppq = startPpq;
    const double step = host.ppqPerBlock(tempo);
    for (int i = 0; i < maxBlocks && !pred(); ++i) {
        host.processBlock(ppq, tempo, /*playing=*/true);
        ppq += step;
    }
    return ppq;
}

// Drives a fresh host to `capturing` with a 2-bar window: apply capture length,
// prime the transport, arm mid-bar, then play across the next bar boundary so the
// machine latches. Returns the ppq for the next contiguous block.
double armAndReachCapturing(PolyTestHost& host, double tempo) {
    host.injectParamChangeThroughProcess(poly::ParamIDs::kCaptureLength, captureLenNorm(2));
    host.processBlock(0.0, tempo, /*playing=*/false); // apply capture length, idle
    host.processBlock(0.0, tempo, /*playing=*/true);  // prime transport continuity
    host.armCapture();
    host.processBlock(host.ppqPerBlock(tempo), tempo, true); // consume arm -> armed
    return playUntil(host, 2 * host.ppqPerBlock(tempo), tempo, [&] { return host.captureState() == 2; });
}

} // namespace

TEST(HostTests, CaptureStateMachine_ArmCaptureComplete_GatesExportReady) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    const double tempo = 120.0;

    host.injectParamChangeThroughProcess(poly::ParamIDs::kCaptureLength, captureLenNorm(2));
    host.processBlock(0.0, tempo, /*playing=*/false);
    EXPECT_EQ(host.captureState(), 0) << "machine starts idle";
    EXPECT_FALSE(host.exportReady());

    host.processBlock(0.0, tempo, /*playing=*/true); // prime transport continuity (idle)
    host.armCapture();
    host.processBlock(host.ppqPerBlock(tempo), tempo, /*playing=*/true); // consume arm
    EXPECT_EQ(host.captureState(), 1) << "armed; waits for the next bar boundary";
    EXPECT_FALSE(host.exportReady()) << "export not ready while armed";

    double ppq = playUntil(host, 2 * host.ppqPerBlock(tempo), tempo, [&] { return host.captureState() == 2; });
    ASSERT_EQ(host.captureState(), 2) << "latched to capturing on a bar boundary";
    EXPECT_FALSE(host.exportReady()) << "export not ready mid-capture";

    playUntil(host, ppq, tempo, [&] { return host.captureState() == 3; });
    ASSERT_EQ(host.captureState(), 3) << "N-bar window elapsed -> complete";
    EXPECT_TRUE(host.exportReady()) << "export ready ONLY in complete";
    EXPECT_GT(host.frozenEventCount(), 0u) << "frozen window populated at complete";

    host.teardown();
}

TEST(HostTests, CaptureReset_FromArmed) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    host.armCapture();
    host.processBlock(0.0, 120.0, /*playing=*/false);
    ASSERT_EQ(host.captureState(), 1);
    host.resetCapture();
    host.processBlock(0.0, 120.0, /*playing=*/false);
    EXPECT_EQ(host.captureState(), 0) << "Reset from armed -> idle";
    host.teardown();
}

TEST(HostTests, CaptureReset_FromCapturing) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    double ppq = armAndReachCapturing(host, 120.0);
    ASSERT_EQ(host.captureState(), 2);
    host.resetCapture();
    host.processBlock(ppq, 120.0, /*playing=*/true);
    EXPECT_EQ(host.captureState(), 0) << "Reset from capturing -> idle";
    EXPECT_FALSE(host.exportReady());
    host.teardown();
}

TEST(HostTests, CaptureReset_FromComplete) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    const double tempo = 120.0;
    double ppq = armAndReachCapturing(host, tempo);
    ppq = playUntil(host, ppq, tempo, [&] { return host.captureState() == 3; });
    ASSERT_EQ(host.captureState(), 3);
    ASSERT_TRUE(host.exportReady());
    host.resetCapture();
    host.processBlock(ppq, tempo, /*playing=*/true);
    EXPECT_EQ(host.captureState(), 0) << "Reset from complete -> idle";
    EXPECT_FALSE(host.exportReady()) << "Reset clears the frozen-ready flag";
    host.teardown();
}

TEST(HostTests, FrozenWindowStableAfterFurtherPlay) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    const double tempo = 120.0;
    double ppq = armAndReachCapturing(host, tempo);
    ppq = playUntil(host, ppq, tempo, [&] { return host.captureState() == 3; });
    ASSERT_EQ(host.captureState(), 3);

    auto frozen = host.frozenEvents();
    ASSERT_GT(frozen.size(), 0u) << "frozen window must be populated at complete";

    // Keep playing well past the captured window; the frozen snapshot the SMF is
    // built from must be byte-stable regardless of continued playback (S08 promise).
    const double step = host.ppqPerBlock(tempo);
    for (int i = 0; i < 400; ++i) {
        host.processBlock(ppq, tempo, /*playing=*/true);
        ppq += step;
    }

    auto after = host.frozenEvents();
    ASSERT_EQ(after.size(), frozen.size()) << "frozen window event count changed after further play";
    // Field-by-field comparison (MEM031: NoteEvent padding makes memcmp unreliable).
    for (size_t i = 0; i < frozen.size(); ++i) {
        EXPECT_DOUBLE_EQ(after[i].ppqPosition, frozen[i].ppqPosition) << "event " << i << " ppq drifted";
        EXPECT_EQ(after[i].pitch, frozen[i].pitch) << "event " << i << " pitch drifted";
        EXPECT_FLOAT_EQ(after[i].velocity, frozen[i].velocity) << "event " << i << " velocity drifted";
    }
    EXPECT_EQ(host.captureState(), 3) << "stays complete across further play";
    host.teardown();
}

TEST(HostTests, TransportStopMidCapture_RetainsCapturingState) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    const double tempo = 120.0;
    double ppq = armAndReachCapturing(host, tempo);
    ASSERT_EQ(host.captureState(), 2);

    // Stop mid-capture: several non-playing blocks at the frozen playhead. Per the
    // pause policy the capturing state must be RETAINED, not lost or completed.
    for (int i = 0; i < 5; ++i)
        host.processBlock(ppq, tempo, /*playing=*/false);
    EXPECT_EQ(host.captureState(), 2) << "stop mid-capture pauses, retaining capturing state";
    EXPECT_FALSE(host.exportReady());

    // Resume contiguously (next block after the frozen stretch) -> not a seek, so
    // capture continues and can still reach complete.
    ppq = playUntil(host, ppq + host.ppqPerBlock(tempo), tempo, [&] { return host.captureState() == 3; });
    EXPECT_EQ(host.captureState(), 3) << "resume from pause continues to complete";
    host.teardown();
}

TEST(HostTests, TransportJumpMidCapture_CancelsToArmed) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    const double tempo = 120.0;
    double ppq = armAndReachCapturing(host, tempo);
    ASSERT_EQ(host.captureState(), 2);

    // Seek far forward (no loop active) mid-capture -> jump detected -> cancel back
    // to armed so the user re-arms for a clean contiguous window.
    host.processBlock(ppq + 64.0, tempo, /*playing=*/true);
    EXPECT_EQ(host.captureState(), 1) << "seek mid-capture cancels to armed";
    EXPECT_FALSE(host.exportReady());
    host.teardown();
}

TEST(HostTests, TempoChangeMidCapture_CancelsToArmed) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    double ppq = armAndReachCapturing(host, 120.0);
    ASSERT_EQ(host.captureState(), 2);

    // Tempo change mid-capture corrupts the SMF beats-to-time math -> cancel to
    // armed. Same contiguous ppq so this isolates the tempo delta from a seek.
    host.processBlock(ppq, 140.0, /*playing=*/true);
    EXPECT_EQ(host.captureState(), 1) << "tempo change mid-capture cancels to armed";
    EXPECT_FALSE(host.exportReady());
    host.teardown();
}

// --- M053 S08 T01: timeline false->true edge seeds fixedPattern in the processor ---
// The audio-thread processor's kCoreTimeline case must seed fixedPattern[] from the
// current Euclidean state on the false->true edge, mirroring the controller cache in
// web_ui_view.cpp. Without this the lane emits from an empty fixedPattern until the
// async sendTimelinePattern->timelineSlot_ handshake lands (~1s), an audible self-blank.
namespace {
// Drives a lane's core params through the real IParameterChanges->process() path.
void setLaneCoreParam(PolyTestHost& host, int lane, int offset, double engineValue) {
    const uint32_t id = poly::ParamIDs::laneCoreParam(lane, offset);
    host.injectParamChangeThroughProcess(id,
                                         poly::params::engineToNormCore(static_cast<uint32_t>(offset), engineValue));
}
} // namespace

TEST(HostTests, TimelineEdgeSeedsFixedPatternInProcessor) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    // Establish a non-trivial Euclidean pattern: 3 hits over 8 steps, no rotation.
    // euclidean(3,8,0) has exactly 3 of 8 steps on, so a stale/empty fixedPattern
    // would be trivially distinguishable from a correctly seeded one.
    const int kSteps = 8, kHits = 3, kRot = 0;
    setLaneCoreParam(host, 0, poly::ParamIDs::kCoreSteps, kSteps);
    setLaneCoreParam(host, 0, poly::ParamIDs::kCoreHits, kHits);
    setLaneCoreParam(host, 0, poly::ParamIDs::kCoreRotation, kRot);
    host.processBlock(0.0, 120.0, /*playing=*/false);

    // Flip timeline off->on. This is the false->true edge that must seed fixedPattern.
    setLaneCoreParam(host, 0, poly::ParamIDs::kCoreTimeline, 1.0);
    host.processBlock(0.0, 120.0, /*playing=*/false);

    auto scene = deserializeSceneState(host.saveState());
    const auto& lane = scene.sceneA.lanes[0];

    std::array<bool, poly::kMaxSteps> expected{};
    poly::euclidean(kHits, kSteps, kRot, expected);

    ASSERT_TRUE(lane.timeline) << "timeline flag should be set after the edge";
    EXPECT_EQ(lane.fixedPatternLength, kSteps) << "fixedPatternLength defaulted from 0 to cycle.steps on the edge";
    int onCount = 0;
    for (int s = 0; s < kSteps; ++s) {
        EXPECT_EQ(lane.fixedPattern[static_cast<size_t>(s)], expected[static_cast<size_t>(s)])
            << "step " << s << " should match the seeded Euclidean pattern";
        if (lane.fixedPattern[static_cast<size_t>(s)])
            ++onCount;
    }
    EXPECT_EQ(onCount, kHits) << "seeded pattern must carry the Euclidean hits, not an empty grid";

    host.teardown();
}

TEST(HostTests, TimelineReSeedDoesNotClobberManualEdits) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    setLaneCoreParam(host, 0, poly::ParamIDs::kCoreSteps, 8);
    setLaneCoreParam(host, 0, poly::ParamIDs::kCoreHits, 3);
    host.processBlock(0.0, 120.0, /*playing=*/false);

    // Enter timeline mode (seeds the Euclidean pattern).
    setLaneCoreParam(host, 0, poly::ParamIDs::kCoreTimeline, 1.0);
    host.processBlock(0.0, 120.0, /*playing=*/false);

    // Simulate a manual edit landing via the async handshake: an all-on pattern that
    // is distinct from euclidean(3,8,0) (which has only 3 steps on).
    std::array<bool, poly::kMaxSteps> manual{};
    for (int s = 0; s < 8; ++s)
        manual[static_cast<size_t>(s)] = true;
    host.injectTimelinePattern(0, manual, 8);
    host.processBlock(0.0, 120.0, /*playing=*/false);

    // Re-apply timeline=true. Already-on means no false->true edge, so the processor
    // must NOT re-seed the Euclidean pattern over the manual edit.
    setLaneCoreParam(host, 0, poly::ParamIDs::kCoreTimeline, 1.0);
    host.processBlock(0.0, 120.0, /*playing=*/false);

    auto scene = deserializeSceneState(host.saveState());
    const auto& lane = scene.sceneA.lanes[0];

    int onCount = 0;
    for (int s = 0; s < 8; ++s)
        if (lane.fixedPattern[static_cast<size_t>(s)])
            ++onCount;
    EXPECT_EQ(onCount, 8) << "re-applying timeline=true with no edge must preserve the manual all-on "
                             "edit, not clobber it back to the 3-hit Euclidean seed";

    host.teardown();
}

// M053 S09: "Add Envelope" in the plugin/DAW path must grow LaneConfig::envelopeCount
// so the newly added envelope is actually evaluated by the engine (audible/rendered),
// matching the WASM reference (poly_action_set_envelope, covered by
// wasm_api_envelope_gap_slot_tests.cpp). This exercises the REAL audio-thread mutation
// site — processor.cpp envelopeSlot_.consume — through the same EnvelopeUpdate notify()
// -> process() handshake a controller drives via sendEnvelopeUpdate. Before T01 the
// consume wrote the envelope fields but left envelopeCount unchanged, so an added
// envelope was silent and lost on save/reload.
//
// A stopped process block drains the handshake and republishes stateSnapshot_, so
// saveState()/getState() reflects the grown count.
TEST(HostTests, AddEnvelopeAtCurrentCountGrowsEnvelopeCount) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    // Baseline: the default scene's lane 0 carries no envelopes.
    {
        auto baseline = deserializeSceneState(host.saveState());
        ASSERT_FALSE(baseline.sceneA.lanes.empty());
        EXPECT_EQ(baseline.sceneA.lanes[0].envelopeCount, 0)
            << "precondition: default lane starts with envelopeCount == 0";
    }

    // Add an envelope at index == current count (0) — the "Add Envelope" action.
    poly::Envelope env{};
    env.target = poly::EnvTarget::Velocity;
    env.periodBars = 1.0f;
    env.depth = 0.5f;
    host.injectEnvelope(/*laneIndex=*/0, /*envelopeIndex=*/0, /*active=*/true, env);
    host.processBlock(0.0, 120.0, /*playing=*/false);

    auto scene = deserializeSceneState(host.saveState());
    const auto& lane = scene.sceneA.lanes[0];
    EXPECT_EQ(lane.envelopeCount, 1)
        << "Add Envelope at index==count must grow envelopeCount so the engine evaluates the new slot";
    EXPECT_TRUE(lane.envelopes[0].active) << "the added envelope must be active";
    EXPECT_EQ(static_cast<uint8_t>(lane.envelopes[0].envelope.target), static_cast<uint8_t>(poly::EnvTarget::Velocity))
        << "the added envelope's payload must survive the handshake";

    host.teardown();
}

// M053 S09 parity with the M049 S01 E1 gap-slot fix, on the plugin path: adding an
// envelope at an index past the current count grows envelopeCount AND initializes the
// intervening gap slots inactive. EnvelopeAssign{} defaults to active=true, so without
// the gap clear the audio thread would resurrect phantom full-depth Velocity LFOs at
// slots [count, index).
TEST(HostTests, AddEnvelopeAtGapIndexGrowsCountAndLeavesGapSlotsInactive) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    // Jump straight to slot 2 from an empty lane. Slots 0 and 1 are intervening gaps.
    poly::Envelope env{};
    env.target = poly::EnvTarget::Density;
    env.periodBars = 8.0f;
    env.depth = 0.5f;
    host.injectEnvelope(/*laneIndex=*/0, /*envelopeIndex=*/2, /*active=*/true, env);
    host.processBlock(0.0, 120.0, /*playing=*/false);

    auto scene = deserializeSceneState(host.saveState());
    const auto& lane = scene.sceneA.lanes[0];
    EXPECT_EQ(lane.envelopeCount, 3) << "writing slot 2 from empty must grow envelopeCount to index+1";
    EXPECT_FALSE(lane.envelopes[0].active) << "gap slot 0 must be inactive (no phantom Velocity LFO)";
    EXPECT_FALSE(lane.envelopes[1].active) << "gap slot 1 must be inactive (no phantom Velocity LFO)";
    EXPECT_TRUE(lane.envelopes[2].active) << "the added envelope at slot 2 must be active";

    host.teardown();
}

// --- M073 S03 T03: MIDI-channel -1 Auto sentinel + explicit channel survive save/restore ---
//
// Pins the symmetric -1<->Auto encoding through the real host param->process->save->restore
// path the DAW drives. The WebUI JS defect (S03 T01/T02) was a read/write asymmetry that
// never emitted the -1 sentinel; the C++ native/state path already encodes it as
// engine = round(norm * 16) - 1 (Kind::MidiChannel, params_def.h:132) yielding -1..15.
// This test proves that contract end-to-end so any future regression on the C++ side that
// re-introduces an off-by-one (explicit N stored as N+1) or drops the Auto sentinel (stored
// as 0 or an explicit channel) fails here — the same user-visible symptom the Cubase UAT
// (MEM063) reported, now guarded on the plugin surface the WebUI shares state with.
TEST(HostTests, MidiChannel_AutoSentinelAndExplicitSurviveSaveRestore) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    const uint32_t lane0Channel = poly::ParamIDs::laneCoreParam(0, poly::ParamIDs::kCoreMidiChannel);
    const uint32_t lane1Channel = poly::ParamIDs::laneCoreParam(1, poly::ParamIDs::kCoreMidiChannel);

    // Normalized encodings the controller/native path use: norm = (channel + 1) / 16, so
    // round(norm * 16) - 1 recovers the channel. Auto is norm 0.0 -> -1.
    const double kAutoNorm = 0.0;                    // round(0)  - 1 = -1 (Auto sentinel)
    const double kChannel7Norm = (7.0 + 1.0) / 16.0; // round(8)  - 1 =  7
    const double kChannel9Norm = (9.0 + 1.0) / 16.0; // round(10) - 1 =  9

    // Stamp lane 0 with an EXPLICIT channel first. The lane's struct default is already -1
    // (types.h:200), so without this precondition a later "reads back -1" assertion could pass
    // trivially on an untouched lane. Proving explicit 7 lands, THEN overwriting with Auto and
    // reading back -1, shows the write path genuinely emits the sentinel.
    host.injectParamChangeThroughProcess(lane0Channel, kChannel7Norm);
    host.processBlock(0.0, 120.0, /*playing=*/true);
    {
        auto pre = deserializeSceneState(host.saveState());
        ASSERT_EQ(pre.sceneA.lanes[0].midiChannel, 7)
            << "precondition: explicit channel 7 must land through the param->process path before "
               "we overwrite lane 0 with Auto";
    }

    // Now set lane 0 back to Auto (-1) and lane 1 to explicit channel 9 in the same block.
    host.injectParamChangeThroughProcess(lane0Channel, kAutoNorm);
    host.injectParamChangeThroughProcess(lane1Channel, kChannel9Norm);
    host.processBlock(0.0, 120.0, /*playing=*/true);

    auto bytes = host.saveState();
    ASSERT_FALSE(bytes.empty()) << "saveState() should produce a non-empty buffer";

    // Reload into a fresh processor (the DAW project-reload path) and drain setState.
    PolyTestHost restored;
    ASSERT_TRUE(restored.setup(44100.0, 512));
    ASSERT_TRUE(restored.loadState(bytes)) << "loadState() should accept a valid saveState() buffer";
    restored.processBlock(0.0, 120.0, /*playing=*/false);

    auto scene = deserializeSceneState(restored.saveState());
    EXPECT_EQ(scene.sceneA.lanes[0].midiChannel, -1)
        << "Auto lane must round-trip as the -1 sentinel, not 0 or an explicit channel. A value of "
           "0 would mean the Auto encoding collapsed to 'channel 1'; the symptom MEM063 reported.";
    EXPECT_EQ(scene.sceneA.lanes[1].midiChannel, 9)
        << "Explicit channel 9 must round-trip with no off-by-one (10 would be the classic +1 bug).";

    restored.teardown();
    host.teardown();
}

// --- M073 S04 T03: envelope periodBars round-trips through notify()->process() and persists ---
//
// S04 makes the envelope period (duration in bars) editable in the WebUI, routed through the
// EXISTING setEnvelope bridge with NO C++/serialization change — periodBars is already a real
// engine field (types.h:170), already carried by the EnvelopeUpdate notify()->process()
// handshake (the same path injectEnvelope drives, mirroring AddEnvelopeAtCurrentCountGrowsEnvelopeCount
// above), and already serialized in v15 state (state_io_envelope.h). This test is the plugin-path
// proof the slice success criterion demands: a non-DEFAULT period (7.0; the struct default is 4.0,
// types.h:170) survives the real DAW handshake AND a fresh save/restore cycle. Any future regression
// that drops periodBars from the audio-thread consume or from (de)serialization fails here with a
// visible mismatch, not a silent reset to the 4.0 default. Per MEM051, this must live in
// poly_host_tests (processor.cpp + SDK) — poly_tests links only poly_engine and cannot reach the
// notify()/process() mutation site.
TEST(HostTests, EnvelopePeriodBarsRoundTripsThroughProcessAndPersists) {
    constexpr float kNonDefaultPeriod = 7.0f; // deliberately != the 4.0 struct default (types.h:170)

    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    // Add an envelope carrying a non-default period through the same EnvelopeUpdate handshake a
    // controller drives via sendEnvelopeUpdate / the setEnvelope bridge action.
    poly::Envelope env{};
    env.target = poly::EnvTarget::Velocity;
    env.periodBars = kNonDefaultPeriod;
    env.depth = 0.5f;
    host.injectEnvelope(/*laneIndex=*/0, /*envelopeIndex=*/0, /*active=*/true, env);
    host.processBlock(0.0, 120.0, /*playing=*/false);

    // Snapshot the saved state once (a drained, stopped block republished stateSnapshot_).
    auto bytes = host.saveState();
    ASSERT_FALSE(bytes.empty()) << "saveState() should produce a non-empty buffer";

    // 1) The period survives the notify()->process() handshake and is published in the snapshot.
    {
        auto scene = deserializeSceneState(bytes);
        const auto& lane = scene.sceneA.lanes[0];
        ASSERT_EQ(lane.envelopeCount, 1) << "the injected envelope must land in slot 0";
        ASSERT_TRUE(lane.envelopes[0].active) << "the injected envelope must be active";
        EXPECT_FLOAT_EQ(lane.envelopes[0].envelope.periodBars, kNonDefaultPeriod)
            << "period must round-trip through the audio-thread consume, not reset to the 4.0 default";
    }

    // 2) Persistence: reload the saved bytes into a FRESH processor (the DAW project-reload path)
    //    and confirm the non-default period is still there after setState + a draining block.

    PolyTestHost restored;
    ASSERT_TRUE(restored.setup(44100.0, 512));
    ASSERT_TRUE(restored.loadState(bytes)) << "loadState() should accept a valid saveState() buffer";
    restored.processBlock(0.0, 120.0, /*playing=*/false);

    auto scene = deserializeSceneState(restored.saveState());
    const auto& lane = scene.sceneA.lanes[0];
    ASSERT_EQ(lane.envelopeCount, 1) << "the envelope must survive save/restore";
    EXPECT_FLOAT_EQ(lane.envelopes[0].envelope.periodBars, kNonDefaultPeriod)
        << "period must persist across save/restore; a 4.0 here means the field was dropped on the "
           "(de)serialization or setState path";

    restored.teardown();
    host.teardown();
}

// M073: the desk emission overlay + played timeline read host.getLaneEmissions,
// which in the DAW is fed by the processor draining the engine
// EmissionEventBuffer into the per-lane UISnapshot rings (publishEmissions),
// serialized to the frame by web_ui_view.cpp. This proves that native handoff
// end-to-end: after a playing block the rings hold classified emissions with a
// valid shifted onset, and a stopped block clears them (mirroring the WASM
// host's resetLaneEmissions on Stop, so the overlay degrades to empty).
TEST(HostTests, EmissionRingsPopulatedOnPlayAndClearedOnStop) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));

    poly::UISnapshot* snap = host.controllerUiSnapshot();
    ASSERT_NE(snap, nullptr) << "fixture broken: connect never populated the UISnapshot pointer";

    // Before any playing block the rings are empty — no emission published.
    for (int lane = 0; lane < poly::kMaxLanes; ++lane)
        EXPECT_EQ(snap->emissionHead[lane].load(std::memory_order_acquire), 0u)
            << "lane " << lane << " ring should be empty before playback";

    // Play a full bar. The default render scene has active lanes with hits, so
    // the engine emits Base classifications the processor drains into the rings.
    host.processBlock(0.0, 120.0, /*playing=*/true);

    uint64_t totalEmitted = 0;
    constexpr int cap = poly::UISnapshot::kEmissionRingCap;
    for (int lane = 0; lane < poly::kMaxLanes; ++lane) {
        uint64_t head = snap->emissionHead[lane].load(std::memory_order_acquire);
        totalEmitted += head;
        int n = head < static_cast<uint64_t>(cap) ? static_cast<int>(head) : cap;
        for (int k = 0; k < n; ++k) {
            const auto& slot = snap->emissionRing[lane][k];
            int kind = slot.kind.load(std::memory_order_relaxed);
            EXPECT_GE(kind, 0);
            EXPECT_LE(kind, 3) << "kind must be a valid EmissionKind (0..3)";
            // Fired kinds (Base/Ghost/Add) carry a real shifted onset; a Drop
            // never schedules a note so its shifted onset equals the grid ppq.
            double ppq = slot.ppq.load(std::memory_order_relaxed);
            double shifted = slot.shiftedPpq.load(std::memory_order_relaxed);
            EXPECT_GE(ppq, 0.0);
            EXPECT_GE(shifted, 0.0);
            if (kind == static_cast<int>(poly::EmissionKind::Drop))
                EXPECT_DOUBLE_EQ(shifted, ppq) << "a Drop has no shifted onset — must equal the grid ppq";
        }
    }
    EXPECT_GT(totalEmitted, 0u) << "a playing bar with active hit lanes must publish at least one emission";

    // Stop. The stopped-path clears the rings so the overlay reads empty and
    // degrades to positional-pattern-only (no lingering stale marks).
    host.processBlock(4.0, 120.0, /*playing=*/false);
    for (int lane = 0; lane < poly::kMaxLanes; ++lane)
        EXPECT_EQ(snap->emissionHead[lane].load(std::memory_order_acquire), 0u)
            << "lane " << lane << " ring must clear on Stop";

    host.teardown();
}

// M073 fix regression guard: the WebUI played timeline ages each dot against the
// frame's NORMALIZED t8 (= fmod(ppqStart, 128)/128 · 256). The processor must
// therefore store emission onsets on that SAME wrapped 128-beat timeline, NOT as
// raw absolute project ppq — otherwise, once the transport passes ppq 128, every
// stored onset dwarfs the normalized t8, every dot ages out of the [0, cyc8]
// window, and the DAW column renders blank while the engine keeps emitting. This
// plays a block WELL past the 128-beat wrap and asserts every stored onset lands
// in [0, 128): raw absolute onsets (~192) would fail; wrapped ones (~64) pass.
TEST(HostTests, EmissionOnsetsNormalizedToFrameTimeline) {
    PolyTestHost host;
    ASSERT_TRUE(host.setup(44100.0, 512));
    poly::UISnapshot* snap = host.controllerUiSnapshot();
    ASSERT_NE(snap, nullptr) << "fixture broken: connect never populated the UISnapshot pointer";

    // Play a block starting at ppq 192 — 1.5 laps into the 128-beat wrap window,
    // so absolute onsets (~192.x) and wrapped onsets (~64.x) are unambiguously
    // distinguishable (192 vs 64 straddle the 128 boundary).
    const double startPpq = 192.0;
    host.processBlock(startPpq, 120.0, /*playing=*/true);

    constexpr int cap = poly::UISnapshot::kEmissionRingCap;
    uint64_t totalEmitted = 0;
    for (int lane = 0; lane < poly::kMaxLanes; ++lane) {
        uint64_t head = snap->emissionHead[lane].load(std::memory_order_acquire);
        totalEmitted += head;
        int n = head < static_cast<uint64_t>(cap) ? static_cast<int>(head) : cap;
        for (int k = 0; k < n; ++k) {
            const auto& slot = snap->emissionRing[lane][k];
            double ppq = slot.ppq.load(std::memory_order_relaxed);
            double shifted = slot.shiftedPpq.load(std::memory_order_relaxed);
            EXPECT_GE(ppq, 0.0);
            EXPECT_LT(ppq, 128.0) << "onset must be wrapped onto the frame timeline, not raw absolute ppq";
            EXPECT_GE(shifted, 0.0);
            EXPECT_LT(shifted, 128.0) << "shifted onset must be wrapped onto the frame timeline";
        }
    }
    EXPECT_GT(totalEmitted, 0u) << "a playing bar past the wrap must still publish emissions";

    host.teardown();
}
