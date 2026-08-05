import path from 'node:path';

// M042 S09 — shared constants for the toggle e2e, used by BOTH the Playwright
// spec (toggle-step.spec.ts, which toggles the step while Cubase is up) and the
// post-quit assertion CLI (assert-toggle.ts, which reads the flushed probe).
// Keeping them here means the two halves can never drift on which step/pitch is
// under test.

// The fixture uses Poly's default patch; the default kick lane's MIDI note is 36
// (GM kick), and the fixed-pattern (timeline) grid is 16 steps/bar in 4/4. We
// toggle step 4 (beat 1, ppq 1.0) — its onset is distinct from the always-present
// downbeat, so the assertion is unambiguous.
export const KICK_PITCH = 36;
export const STEPS_PER_BAR = 16;
export const TOGGLE_STEP_INDEX = 4;
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
