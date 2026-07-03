#pragma once

#include "poly/scene.h"
#include "poly/types.h"

namespace poly {

// Clamp every field of a deserialized state into the ranges the engine's
// render path assumes. Deserialization (state_io.h) reads raw bytes from
// host-provided streams; a corrupted or hostile preset must never be able to
// index past fixed-size arrays or produce zero-length cycles in
// renderRange(). Idempotent; cheap; called from readGrooveState/
// readSceneState so no caller can forget it.
void sanitizeGrooveState(GrooveState& state);
void sanitizeSceneState(SceneState& scene);

} // namespace poly
