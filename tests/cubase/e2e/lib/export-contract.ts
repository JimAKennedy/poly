import path from 'node:path';

// M032 export UAT automation (Level A) — shared constants for the in-DAW MIDI
// export e2e. Used by export-midi.spec.ts (which clicks the shipping Export chip
// inside Cubase over CDP) and the post-hoc validator invocation.
//
// The plugin's exportSaveAs / beginMidiDrag handlers, when POLY_EXPORT_SINK is
// set, write the exact SMF bytes they would hand the native Save-As panel to
// that path and SKIP the modal dialog (see web_ui_view.cpp
// writeExportSinkIfEnabled / openMidiExportDialog). The workflow exports the var
// at Cubase launch so the plugin inherits it. This is how the e2e captures the
// real in-plugin export blob without automating a native file dialog or OS drag.

const ARTIFACT_DIR =
  process.env.POLY_ARTIFACT_DIR || path.join(process.cwd(), '_artifacts');

// Where the plugin writes the export bytes in sink mode. Must match the
// POLY_EXPORT_SINK the workflow exports before launching Cubase. Kept in the
// artifact dir so the captured .mid is uploaded with the run for diagnosis.
export const EXPORT_SINK =
  process.env.POLY_EXPORT_SINK || path.join(ARTIFACT_DIR, 'export-sink.mid');

// The lane whose per-lane export the spec exercises (lane 0 = Kick in the
// default preset / poly-4bar fixture). Mirrors KICK_LANE_INDEX in
// toggle-contract.ts.
export const EXPORT_LANE_INDEX = 0;
export const EXPORT_LANE_NAME = 'Kick';

// The lane-track names an all-lanes export is expected to produce from the
// poly-4bar fixture.
//
// These are the ENGINE's GM names (engine/src/lane_name.cpp laneName()), which
// is what the export path actually writes: offline_render.cpp names each MTrk
// via laneName(lane.midiNote). They are deliberately NOT the WebUI display
// names in kWebPresetLaneNames (web_ui_view.cpp), which spell the toms
// "Tom Hi"/"Tom Lo" — laneName() collapses the whole GM tom range to "Tom" and
// cannot emit those strings at all.
//
// The set is derived from the fixture's state, not from a factory preset. The
// fixture holds Poly's default processor state, whose 4-bar golden
// (tests/golden/processor_default_4bars.txt) emits pitches 36, 38, 42, 45 —
// laneName() maps those to Kick, Snare, Hi-Hat, Tom respectively. Factory
// preset 0 is a different patch (its 4th lane is note 46, "Open Hat"), so
// expectations taken from it do not describe this export.
//
// Re-derive from the golden's distinct pitches if the fixture is ever re-cut.
export const EXPORT_ALL_LANE_NAMES = ['Kick', 'Snare', 'Hi-Hat', 'Tom'];
