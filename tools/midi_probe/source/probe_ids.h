// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

#include "pluginterfaces/base/funknown.h"

namespace probe {

static const Steinberg::FUID kProbeProcessorUID(0x4D1E8A02, 0xB3C75F91, 0x6A2D9E04, 0xF87B3C65);
// Paired edit-controller UID. Cubase instantiates a VST3 instrument track only
// for a component that advertises a controller class (kInstrumentSynth needs the
// processor+controller pair), so the probe now ships a minimal controller.
static const Steinberg::FUID kProbeControllerUID(0x9C2F5B18, 0x7A46D3E0, 0x1B8E4C72, 0x5D93A0F6);

static constexpr auto kProbePluginName = "Poly MIDI Probe";
static constexpr auto kProbeControllerName = "Poly MIDI Probe Controller";
static constexpr auto kProbeVersionString = "1.0.0";

} // namespace probe
