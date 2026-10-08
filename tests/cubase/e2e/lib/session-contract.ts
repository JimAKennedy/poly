import { mutationActive } from './preset-contract';

// M004 S01 (DAW01) — a project saved with a non-default patch reopens carrying
// it.
//
// THE ROUND TRIP as data. The save pass applies three edits through the same
// bridge actions the editor's controls send, reads the resulting state back
// from the native side, and records it; the reopen pass reads the state again
// in a fresh Cubase and compares. Each edit covers one clause of the slice's
// definition of done, and each is something the default patch does not have:
//
//   - a selected preset   -> state.preset
//   - an edited step      -> lane 0 switched to timeline mode, one step toggled
//                            (state.lanes[0].timeline, .fixed)
//   - per-step micro-timing -> state.lanes[0].mt
//
// THE FINGERPRINT is the subset of state the round trip must preserve: the
// preset name, the seed, and per lane its name, timeline flag, fixed pattern,
// derived pattern and micro-timing. Velocities, envelopes and macros are
// serialised too, but none of them is edited here, so including them adds
// fields that can only agree; the comparison stays on what was changed and on
// the shape around it.

/** Factory preset applied first (bridge-schema.md applyPreset). */
export const PRESET_INDEX = 3;
/** Lane whose step and micro-timing are edited. */
export const EDIT_LANE = 0;
/** Step toggled in timeline mode. */
export const EDIT_STEP = 1;
/** Step given a micro-timing offset, and the offset (ms; the UI's range is +/-20). */
export const MT_STEP = 2;
export const MT_MS = 12;

/** File the save pass writes and the reopen pass reads, in the session dir. */
export const EXPECTED_FILE = 'session-expected.json';

export interface LaneFingerprint {
  name: string;
  timeline: boolean;
  fixed: number[] | null;
  pattern: number[];
  mt: number[];
}
export interface SessionFingerprint {
  preset: string;
  seed: number;
  lanes: LaneFingerprint[];
}

interface StateLike {
  preset: string;
  seed: number;
  lanes: Array<{
    name: string;
    timeline: boolean;
    fixed: number[] | null;
    pattern: number[];
    mt: number[];
  }>;
}

/** The fields of a native `state` push the round trip must preserve. */
export function fingerprint(state: StateLike): SessionFingerprint {
  return {
    preset: state.preset,
    seed: state.seed,
    lanes: state.lanes.map((l) => ({
      name: l.name,
      timeline: !!l.timeline,
      fixed: l.fixed ? [...l.fixed] : null,
      pattern: [...l.pattern],
      mt: [...l.mt],
    })),
  };
}

/**
 * Every path where `actual` differs from `expected`, as `path: expected ->
 * actual` lines. Empty means the session came back as it was saved.
 */
export function compareFingerprints(expected: SessionFingerprint, actual: SessionFingerprint): string[] {
  const out: string[] = [];
  const show = (v: unknown) => JSON.stringify(v);
  const diff = (p: string, e: unknown, a: unknown) => {
    if (show(e) !== show(a)) out.push(`${p}: ${show(e)} -> ${show(a)}`);
  };
  diff('preset', expected.preset, actual.preset);
  diff('seed', expected.seed, actual.seed);
  diff('lanes.length', expected.lanes.length, actual.lanes.length);
  const n = Math.min(expected.lanes.length, actual.lanes.length);
  for (let i = 0; i < n; i++) {
    const e = expected.lanes[i];
    const a = actual.lanes[i];
    for (const key of ['name', 'timeline', 'fixed', 'pattern', 'mt'] as const) {
      diff(`lanes[${i}].${key}`, e[key], a[key]);
    }
  }
  return out;
}

export class SessionContractError extends Error {}

/**
 * The saved state must actually carry the edits, or reopening it proves
 * nothing: a round trip of the default patch passes whether or not state is
 * serialised at all, because the default is what a fresh instance has anyway.
 */
export function requireEdited(saved: SessionFingerprint, presetName: string): void {
  const lane = saved.lanes[EDIT_LANE];
  const problems: string[] = [];
  if (saved.preset !== presetName) problems.push(`preset is '${saved.preset}', not '${presetName}'`);
  if (!lane) problems.push(`lane ${EDIT_LANE} is missing`);
  else {
    if (!lane.timeline) problems.push(`lane ${EDIT_LANE} is not in timeline mode`);
    if ((lane.mt[MT_STEP] ?? 0) !== MT_MS)
      problems.push(`lane ${EDIT_LANE} step ${MT_STEP} micro-timing is ${lane.mt[MT_STEP]}, not ${MT_MS}`);
  }
  if (problems.length)
    throw new SessionContractError(
      `the state about to be saved does not carry the edits: ${problems.join('; ')}`,
    );
}

/**
 * The red path: POLY_E2E_MUTATE=s01-perturb-state.
 *
 * Changes the EXPECTED micro-timing of the edited step before the reopened
 * session is compared against it, so the comparison must fail on exactly that
 * field. It perturbs the expectation file's content, never the saved project:
 * the knob must not be able to change what Poly does.
 */
export function applyMutation(expected: SessionFingerprint, mutate: string | undefined): SessionFingerprint {
  if (!mutationActive('s01-perturb-state', mutate)) return expected;
  const lanes = expected.lanes.map((l, i) =>
    i === EDIT_LANE ? { ...l, mt: l.mt.map((v, s) => (s === MT_STEP ? v + 1 : v)) } : l,
  );
  return { ...expected, lanes };
}
