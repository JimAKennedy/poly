#pragma once

#include "poly/types.h"

namespace poly {

float evaluateShape(Shape shape, float phase);

float evaluateShapeFull(const Envelope& env, float phase);

double computeEnvelopePhase(double ppqPosition, float periodBars, float phaseOffset);

} // namespace poly
