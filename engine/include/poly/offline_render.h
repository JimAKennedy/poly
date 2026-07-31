#pragma once

#include <cstdint>
#include <vector>

#include "poly/scene.h"
#include "poly/types.h"

namespace poly {

// M053 S11 (T01): Offline SMF render primitive.
//
// Renders the CURRENT pattern described by `scene` to a Standard MIDI File blob
// with NO DAW transport involvement. A playing transport is fabricated from
// ppq 0 through `bars * ppqPerBar` and the pattern is rendered in multiple
// blocks through Engine::renderRange, exactly mirroring the resolution the live
// audio/site paths perform (scene select/morph -> resolveMacros ->
// resolveConstraints) so the exported file matches what the user hears.
//
// The result is deterministic: the same (scene, bars, tempo, timeSig) inputs
// always produce a byte-identical blob, because renderRange derives all phase
// from absolute PPQ + seed and carries no cross-block state.
//
// This is the single primitive behind the WebUI Export chip (Save-As and
// drag-to-DAW), letting export work from a stopped preview instead of the
// arm->capture->complete state machine.
//
// Robustness (no external dependencies; pure in-memory computation):
//   - bars is clamped to [1, kMaxRenderBars]; non-positive values render 1 bar,
//     pathological values are capped so memory stays bounded.
//   - tempo is clamped to a musical floor for the block-stepping math; writeSMF
//     independently clamps the tempo meta it writes (see kSmfMinTempo).
//   - Degenerate time signatures fall back to 4/4.
// The function therefore always returns a valid SMF blob (at minimum a header +
// empty track), never throws for out-of-range arguments, and performs no I/O.
static constexpr int kMaxRenderBars = 256;

std::vector<uint8_t> renderPatternToSMF(const SceneState& scene, int bars, double tempo, int timeSigNumerator = 4,
                                        int timeSigDenominator = 4);

} // namespace poly
