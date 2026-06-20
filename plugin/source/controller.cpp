#include "controller.h"

namespace poly {

Steinberg::tresult PLUGIN_API PolyController::initialize(
    Steinberg::FUnknown* context) {
    auto result = EditController::initialize(context);
    if (result != Steinberg::kResultOk)
        return result;

    return Steinberg::kResultOk;
}

} // namespace poly
