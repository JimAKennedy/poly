#pragma once

#include "poly/types.h"

namespace poly {

float evaluateShape(Shape shape, float phase);

float evaluateShapeFull(const Envelope& env, float phase);

// M051 S02: ppqPerBar defaults to 4.0 (one whole-note bar = 4 quarters) so engine
// tests and hosts without a published time signature see the historical 4/4 behavior
// unchanged. Real render paths thread TransportContext::ppqPerBar() through.
double computeEnvelopePhase(double ppqPosition, float periodBars, float phaseOffset, double ppqPerBar = 4.0);

} // namespace poly
