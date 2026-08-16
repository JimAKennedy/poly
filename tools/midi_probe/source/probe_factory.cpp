// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/main/pluginfactory.h"

#include "probe_controller.h"
#include "probe_ids.h"
#include "probe_processor.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

BEGIN_FACTORY_DEF("jk.digital", "https://jk.digital", "mailto:contact@jk.digital")

// The probe registers as a VST3 instrument (kInstrumentSynth), NOT an analyzer.
// A kFxAnalyzer never appears in Cubase's MIDI Inserts list and an audio-insert
// analyzer receives audio, not MIDI — so a kFxAnalyzer probe cannot receive
// Poly's note stream on any track type without fragile routing. An instrument,
// by contrast, gets its track's MIDI as data.inputEvents automatically, which is
// exactly the capture path the probe needs. The paired controller class below is
// required: Cubase instantiates an instrument only for a processor+controller
// pair.
DEF_CLASS2(INLINE_UID_FROM_FUID(probe::kProbeProcessorUID), PClassInfo::kManyInstances, kVstAudioEffectClass,
           probe::kProbePluginName, 0, Vst::PlugType::kInstrumentSynth, probe::kProbeVersionString, kVstVersionString,
           probe::ProbeProcessor::createInstance)

DEF_CLASS2(INLINE_UID_FROM_FUID(probe::kProbeControllerUID), PClassInfo::kManyInstances, kVstComponentControllerClass,
           probe::kProbeControllerName, 0, "", probe::kProbeVersionString, kVstVersionString,
           probe::ProbeController::createInstance)

END_FACTORY
