#pragma once

#include "poly/types.h"

namespace poly {

// region:render-range
class Engine {
public:
    void renderRange(const TransportContext& tc, const GrooveState& state, NoteEventBuffer& out);
};
// endregion:render-range

} // namespace poly
