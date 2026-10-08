import { pitchesByPpq, type Pass } from './transport-contract';

// M004 S07 (DAW07) — a host automation lane changes Poly's output where it
// says, and nowhere else.
//
// THE PARAMETER is lane 0's "Active" (ParamIDs::laneParam(0, kActive),
// Poly's parameter index 8), and it was chosen for the assertion it allows.
// Switching a lane off removes that lane's notes and touches nothing else:
// lane 0 is the kick, pitch 36, and no other lane in the fixture's default
// patch plays 36. So "the output changed here and not before" has an exact
// form -- before the change the two passes match note for note; from the change
// on, the read pass has no pitch-36 note and every other pitch still matches.
// A continuous parameter (velocity, probability) would need a tolerance, and a
// structural one (steps, hits) would move notes across the change point.
//
// THE PASSES come from tests/cubase/driver/remote.py `automate`: a baseline
// before any lane exists, a write pass with W on, and a read pass with W off and
// R on. The write pass plays the change live, so it proves nothing about the
// lane; the read pass is the lane playing back, and it is compared against the
// baseline.

/** Lane 0's note in the fixture's default patch. */
export const AUTOMATED_PITCH = 36;
/** Bar at whose downbeat remote.py writes the switch-off (1-based). */
export const OFF_AT_BAR = 3;
export const BEATS_PER_BAR = 4;
export const PASSAGE_BARS = 4;
/** PPQ of the change: the downbeat of OFF_AT_BAR. */
export const CHANGE_PPQ = (OFF_AT_BAR - 1) * BEATS_PER_BAR;
/**
 * PPQ where the passage ends. The driver plays TAIL_SECONDS past it, so a pass
 * can carry a note at exactly this position; it is outside the passage and is
 * not compared, as compare_probe_golden.py trims it with --max-ppq.
 */
export const PASSAGE_END_PPQ = PASSAGE_BARS * BEATS_PER_BAR;

export interface AutomationFinding {
  ppq: number;
  baseline: number[];
  automated: number[];
  why: string;
}

/**
 * Every position where the read pass departs from what the lane predicts.
 *
 * Before `changePpq`: the read pass must equal the baseline exactly. A missing
 * pitch-36 note here is the "applied too early" failure, which a bare "did the
 * output change" check would pass.
 *
 * From `changePpq` on: the read pass must equal the baseline with `pitch`
 * removed. A pitch-36 note still sounding is the lane not taking effect; any
 * other difference is the change leaking beyond the parameter it automates.
 *
 * Positions at or past `endPpq` are outside the passage and ignored.
 */
export function automationFindings(
  baseline: Pass,
  automated: Pass,
  pitch = AUTOMATED_PITCH,
  changePpq = CHANGE_PPQ,
  endPpq = PASSAGE_END_PPQ,
): AutomationFinding[] {
  const base = pitchesByPpq(baseline);
  const auto = pitchesByPpq(automated);
  const out: AutomationFinding[] = [];
  for (const key of new Set([...base.keys(), ...auto.keys()])) {
    const ppq = Number(key);
    if (ppq >= endPpq - 1e-9) continue;
    const b = base.get(key) ?? [];
    const a = auto.get(key) ?? [];
    const after = ppq >= changePpq - 1e-9;
    const expected = after ? b.filter((p) => p !== pitch) : b;
    if (expected.join(',') === a.join(',')) continue;
    let why: string;
    if (!after && b.includes(pitch) && !a.includes(pitch)) why = 'lane silenced before the change';
    else if (after && a.includes(pitch)) why = 'lane still sounding after the change';
    else why = 'a pitch other than the automated lane changed';
    out.push({ ppq, baseline: b, automated: a, why });
  }
  return out.sort((x, y) => x.ppq - y.ppq);
}

export class AutomationContractError extends Error {}

/**
 * The comparison must have something to compare, or it proves nothing.
 *
 * The read pass is pass index 2 of three. A capture with fewer passes means a
 * pass never played; a baseline with no automated-pitch note after the change
 * would make "the lane went silent" true of a lane that was never sounding.
 */
export function requireComparable(passes: Pass[], pitch = AUTOMATED_PITCH, changePpq = CHANGE_PPQ): void {
  if (passes.length < 3)
    throw new AutomationContractError(
      `expected 3 passes (baseline, write, read) and the capture has ${passes.length}; ` +
        'check that remote.py automate ran all three',
    );
  const sounding = passes[0].notes.some(
    (n) => n.pitch === pitch && n.ppq >= changePpq - 1e-9 && n.ppq < PASSAGE_END_PPQ - 1e-9,
  );
  if (!sounding)
    throw new AutomationContractError(
      `the baseline has no pitch-${pitch} note after ppq ${changePpq}, so silencing ` +
        'it would be indistinguishable from it never playing',
    );
}
