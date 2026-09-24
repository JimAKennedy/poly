// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// libFuzzer target for the Standard MIDI File reader — the second untrusted
// input SECURITY.md names (open-source-launch M002/S02, OS10). A user drops any
// file on a lane; parseSMF must reject or read it without ever touching a byte
// outside [data, data+size), and the fit-and-apply path behind the drop must
// hold for whatever the parse produced.
#include <cstddef>
#include <cstdint>

#include "poly/fitter.h"
#include "poly/midi_reader.h"
#include "poly/types.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    const poly::MidiParseResult parsed = poly::parseSMF(data, size);
    if (parsed.valid)
        (void)poly::fitEuclidean(parsed.onsetsPpq, parsed.loopLengthPpq, parsed.tempoBpm);

    // The same bytes through the single import operation both UI surfaces
    // call, on a lane whose note is the kick — the common drop.
    poly::LaneConfig lane{};
    lane.midiNote = 36;
    (void)poly::importMidiToLane(data, size, lane);
    return 0;
}
