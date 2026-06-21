#pragma once

#include "poly/types.h"

namespace poly {

enum class SceneSelect : uint8_t { A, B, Morph };

struct SceneState {
    GrooveState sceneA{};
    GrooveState sceneB{};
    SceneSelect select = SceneSelect::A;
    float morphAmount = 0.0f;
};

GrooveState interpolateGrooveState(const GrooveState& a, const GrooveState& b, float t);

} // namespace poly
