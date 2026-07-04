#pragma once

#include "pluginterfaces/base/funknown.h"

namespace probe {

static const Steinberg::FUID kProbeProcessorUID(0x4D1E8A02, 0xB3C75F91, 0x6A2D9E04, 0xF87B3C65);

static constexpr auto kProbePluginName = "Poly MIDI Probe";
static constexpr auto kProbeVersionString = "1.0.0";

} // namespace probe
