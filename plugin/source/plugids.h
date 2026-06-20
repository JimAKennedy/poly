#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace poly {

static const Steinberg::FUID kPolyProcessorUID(
    0x8A3B7C01, 0xE5F24D83, 0x9B1A6E47, 0xD20C5F98);

static const Steinberg::FUID kPolyControllerUID(
    0x2F6D9B04, 0xA7C83E15, 0x4E0F2B69, 0x81D7A3C6);

static constexpr auto kPolyPluginName = "Poly";
static constexpr auto kPolyVersionString = "0.1.0";

} // namespace poly
