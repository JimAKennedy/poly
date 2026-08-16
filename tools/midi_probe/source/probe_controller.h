// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

namespace probe {

// Minimal edit-controller for the Poly MIDI Probe instrument.
//
// The probe is a diagnostic VST3 instrument (kInstrumentSynth): Cubase places an
// instrument on an instrument track and feeds it the track's MIDI as
// data.inputEvents, which is exactly what the probe needs to capture Poly's note
// stream — no MIDI-insert routing gymnastics. A VST3 instrument must advertise a
// paired controller class, so this controller exists purely to satisfy that
// contract. It has no parameters and no UI (the probe writes JSONL to
// POLY_PROBE_OUTPUT; there is nothing to edit), so the default EditController
// behaviour is sufficient.
class ProbeController : public Steinberg::Vst::EditController {
public:
    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IEditController*>(new ProbeController()); // ownership-transfer
    }
};

} // namespace probe
