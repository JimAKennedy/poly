#pragma once

#include "poly/types.h"

namespace poly {

class Engine {
public:
    void renderRange(const TransportContext& tc, NoteEventBuffer& out);
};

} // namespace poly
