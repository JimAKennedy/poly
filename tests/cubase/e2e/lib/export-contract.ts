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
