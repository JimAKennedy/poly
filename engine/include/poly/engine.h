#pragma once

#include "poly/types.h"

namespace poly {

// region:render-range
class Engine {
public:
    // emissions (optional): if non-null, receives one EmissionEvent per
    // step considered by the render, classifying each as Base/Ghost/Add/Drop.
    // Off-pattern steps that produce no hit are omitted (Silent). Default
    // nullptr preserves the prior contract for callers that don't need
    // classification — audio thread, plugin process(), tests, etc.
    void renderRange(const TransportContext& tc, const GrooveState& state, NoteEventBuffer& out,
                     EmissionEventBuffer* emissions = nullptr);
};
// endregion:render-range

} // namespace poly
