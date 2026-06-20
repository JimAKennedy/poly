#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

namespace poly {

class PolyController : public Steinberg::Vst::EditController {
public:
    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IEditController*>(
            new PolyController());
    }

    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
};

} // namespace poly
