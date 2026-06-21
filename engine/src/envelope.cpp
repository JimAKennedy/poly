#include "poly/envelope.h"

#include <cmath>

namespace poly {

static constexpr double kTwoPi = 6.283185307179586;

float evaluateShape(Shape shape, float phase) {
    switch (shape) {
    case Shape::Sine:
        return 0.5f * (1.0f + std::sin(static_cast<float>(kTwoPi) * phase));
    case Shape::Ramp:
        return phase;
    case Shape::Triangle:
        return 1.0f - std::abs(2.0f * phase - 1.0f);
    case Shape::Curve:
        return 0.5f;
    case Shape::StepList:
        return 0.5f;
    }
    return 0.5f;
}

double computeEnvelopePhase(double ppqPosition, float periodBars, float phaseOffset) {
    double periodPpq = static_cast<double>(periodBars) * 4.0;
    double raw = std::fmod(ppqPosition / periodPpq + static_cast<double>(phaseOffset), 1.0);
    if (raw < 0.0)
        raw += 1.0;
    return raw;
}

} // namespace poly
