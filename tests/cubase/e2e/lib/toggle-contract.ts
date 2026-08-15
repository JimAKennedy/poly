import path from 'node:path';

// M042 S09 — shared constants for the toggle e2e, used by BOTH the Playwright
// spec (toggle-step.spec.ts, which toggles the step while Cubase is up) and the
// post-quit assertion CLI (assert-toggle.ts, which reads the flushed probe).
// Keeping them here means the two halves can never drift on which step/pitch is
// under test.

// The fixture's kick lane (lane 0, MIDI note 36) is a timeline lane whose ladder
// renders 4 steps/bar in 4/4 — one step per quarter note — and ALL FOUR steps
// start ON (the golden shows pitch 36 firing at every integer ppq: 0,1,2,3,4,5…).
// Because the lane is fully lit there is no OFF step to toggle ON, so the e2e
// instead toggles step 2 (ppq 2.0, a mid-bar quarter) OFF and asserts that
// note-on DISAPPEARS from the probe. Step 2 is unambiguous: at ppq 2.0 only lane
// 0 emits pitch 36 (other lanes fire 38/42 there), and the 4-step cycle == 1 bar
// so the step is removed in every bar (ppq 2.0, 6.0, 10.0, 14.0).
export const KICK_PITCH = 36;
export const STEPS_PER_BAR = 4;
export const TOGGLE_STEP_INDEX = 2;
export const KICK_LANE_INDEX = 0;

// The spec writes the expected {pitch, ppq} here after toggling; assert-toggle.ts
// reads it after Cubase quits and the probe JSONL exists. Lives in the artifact
// dir so it's captured alongside the probe output on failure.
const ARTIFACT_DIR =
  process.env.POLY_ARTIFACT_DIR || path.join(process.cwd(), '_artifacts');
export const EXPECTED_HIT_FILE = path.join(ARTIFACT_DIR, 'e2e-expected-hit.json');

// Where poly_midi_probe flushes JSONL on Cubase deactivate (workflow env).
export const PROBE_OUTPUT =
  process.env.POLY_PROBE_OUTPUT || path.join(ARTIFACT_DIR, 'probe.jsonl');
