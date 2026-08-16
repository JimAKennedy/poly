#include "poly/offline_render.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

#include "poly/constraint.h"
#include "poly/engine.h"
#include "poly/lane_name.h"
#include "poly/macro.h"
#include "poly/scene.h"
#include "poly/smf_writer.h"
#include "poly/types.h"

namespace poly {

namespace {

// Resolve the SceneState down to the single GrooveState the engine should
// render, mirroring wasm_api.cpp readState() + poly_render(): honor scene
// select/morph, then apply macros and constraints. Keeping this identical to
// the site/plugin render path is what makes the exported file match playback.
GrooveState resolveRenderState(const SceneState& scene) {
    GrooveState base;
    if (scene.select == SceneSelect::Morph) {
        base = interpolateGrooveState(scene.sceneA, scene.sceneB, scene.morphAmount);
    } else if (scene.select == SceneSelect::B) {
        base = scene.sceneB;
    } else {
        base = scene.sceneA;
    }
    return resolveConstraints(base, resolveMacros(base));
}

} // namespace

std::vector<uint8_t> renderPatternToSMF(const SceneState& scene, int bars, double tempo, int timeSigNumerator,
                                        int timeSigDenominator, int laneFilter) {
    // Argument hardening: this primitive has no external dependencies, so the
    // only failure modes are out-of-range inputs. Clamp them to sane ranges so
    // the block loop is finite and every field the engine indexes is valid.
    const int safeBars = std::clamp(bars <= 0 ? 1 : bars, 1, kMaxRenderBars);
    const int safeNum = (timeSigNumerator > 0) ? timeSigNumerator : 4;
    const int safeDen = (timeSigDenominator > 0) ? timeSigDenominator : 4;
    // Floor tempo for the samples->PPQ block-stepping math (writeSMF clamps the
    // tempo it serializes separately). Keeps ppqPerBlock strictly positive so
    // the loop always terminates.
    const double safeTempo = (std::isfinite(tempo) && tempo >= kSmfMinTempo) ? tempo : kSmfMinTempo;

    TransportContext tc{};
    tc.tempo = safeTempo;
    tc.sampleRate = 44100.0;
    tc.blockSize = 512;
    tc.playing = true;
    tc.timeSigNumerator = static_cast<int16_t>(safeNum);
    tc.timeSigDenominator = static_cast<int16_t>(safeDen);

    const double totalPpq = static_cast<double>(safeBars) * tc.ppqPerBar();
    // One host block's worth of musical time: (blockSize / sampleRate) seconds
    // converted to quarter notes at the render tempo. Fabricating realistic
    // multi-block advancement (rather than a single giant block) exercises the
    // same per-block render path the live plugin uses.
    const double secondsPerBlock = static_cast<double>(tc.blockSize) / tc.sampleRate;
    const double ppqPerBlock = secondsPerBlock * (safeTempo / 60.0);

    const GrooveState resolved = resolveRenderState(scene);

    Engine engine;
    NoteEventBuffer buffer;
    std::vector<NoteEvent> events;

    double ppq = 0.0;
    while (ppq < totalPpq) {
        double blockEnd = ppq + ppqPerBlock;
        if (blockEnd > totalPpq)
            blockEnd = totalPpq;

        tc.ppqStart = ppq;
        tc.ppqEnd = blockEnd;

        buffer.clear();
        engine.renderRange(tc, resolved, buffer);

        for (size_t i = 0; i < buffer.count; ++i)
            events.push_back(buffer.events[i]);

        ppq = blockEnd;
    }

    // Per-lane export: keep only the requested lane's events before serializing.
    // laneFilter < 0 means "all lanes" (default). A specific N drops every event
    // whose laneIndex differs, so writeMultiTrackSMF emits the conductor track
    // plus exactly one named MTrk for lane N; an out-of-range N simply matches
    // nothing, yielding a conductor-only file (never a throw).
    if (laneFilter >= 0) {
        events.erase(std::remove_if(events.begin(), events.end(),
                                    [laneFilter](const NoteEvent& e) { return e.laneIndex != laneFilter; }),
                     events.end());
    }

    std::sort(events.begin(), events.end(),
              [](const NoteEvent& a, const NoteEvent& b) { return a.ppqPosition < b.ppqPosition; });

    // Format-1 export: one named MTrk per active lane. The offline path has the
    // resolved GrooveState, so each lane's configured GM note yields a real drum
    // name ("Kick"/"Snare"/...) via laneName() rather than a bare "Lane N". A
    // NoteEvent::laneIndex outside the lane array falls back to the empty name,
    // which writeMultiTrackSMF turns into "Lane N".
    auto nameForLane = [&resolved](int laneIndex) -> std::string {
        if (laneIndex >= 0 && laneIndex < kMaxLanes)
            return laneName(resolved.lanes[static_cast<size_t>(laneIndex)].midiNote);
        return {};
    };
    // ppqOffset 0.0: the render starts at bar 0, so absolute PPQ positions are
    // already the intended file positions (no leading-silence trim needed).
    return writeMultiTrackSMF(events.data(), events.size(), safeTempo, 0.0, nameForLane);
}

} // namespace poly
