#pragma once

#include <array>
#include <string>

#include "poly/scene.h"

namespace poly {

std::string escapeJsonString(const std::string& s);
const char* roleToString(Role r);
const char* envTargetToString(EnvTarget t);
const char* sceneSelectToString(SceneSelect s);
std::string laneToJson(const LaneConfig& cfg, const std::string& name, int laneIndex);

// region:groove-state-to-json
using LaneNameFn = const std::string& (*)(int lane, void* ctx);

std::string grooveStateToJson(const GrooveState& gs, const SceneState& ss, LaneNameFn nameFunc, void* nameCtx,
                              const std::string& presetName);
// endregion:groove-state-to-json

} // namespace poly
