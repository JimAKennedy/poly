import { mutationActive } from './preset-contract';
import { pitchesByPpq, type Pass } from './transport-contract';

// M004 S06 (DAW06) — Poly emits the same notes when Cubase renders offline as
// when it plays in real time.
//
// Offline export drives process() with a different clock and, in general,
// different block sizes than realtime playback, so a block-size dependency --
// a phase advanced per block rather than derived from PPQ, an event dropped at
// a block edge -- shows up as a difference between the two. Determinism is
// golden-tested for the engine's own render; this is the host's two paths.
//
// THE CAPTURES are both in one probe file: the spec plays the passage in real
// time, then exports the locator range offline. Cubase's export starts again at
// the left locator, so the probe sees a backward jump and splitPasses() gives
// pass 0 (realtime) and pass 1 (offline).
//
// THE TOLERANCE, written down so it can be audited:
//   - Positions compare after quantising PPQ to 1e-3 (pitchesByPpq), the same
//     quantum the transport check uses. A block-size defect moves a note by a
//     fraction of a block -- at 48 kHz and 120 BPM a 512-sample block is
//     0.021 PPQ, twenty times the quantum -- so this cannot hide one.
//   - Velocity is not compared, as in the transport check: it derives from the
//     same step index as position, and a float tolerance would add a question
//     without adding signal.
//   - No start-offset alignment is applied. A realtime pass and a bounce could
//     in principle differ by a constant offset without differing musically; on
//     Cubase 14 both start at exactly PPQ 0 (observed on the runner,
//     2026-10-07), so aligning would only create a way for a real shift to be
//     explained away.
//   - Only [0, endPpq) is compared. The realtime pass covers the driver's four
//     bars plus a tail; the export covers the locator range (eight bars in the
//     fixture). Their overlap is the passage.

/** End of the compared range: the driver's four 4/4 bars. */
export const PASSAGE_END_PPQ = 16;

export interface BounceFinding {
  ppq: number;
  realtime: number[];
  offline: number[];
}

/** Every position in [0, endPpq) where the offline pass differs from realtime. */
export function bounceFindings(realtime: Pass, offline: Pass, endPpq = PASSAGE_END_PPQ): BounceFinding[] {
  const rt = pitchesByPpq(realtime);
  const off = pitchesByPpq(offline);
  const out: BounceFinding[] = [];
  for (const key of new Set([...rt.keys(), ...off.keys()])) {
    const ppq = Number(key);
    if (ppq < -1e-9 || ppq >= endPpq - 1e-9) continue;
    const a = rt.get(key) ?? [];
    const b = off.get(key) ?? [];
    if (a.join(',') !== b.join(',')) out.push({ ppq, realtime: a, offline: b });
  }
  return out.sort((x, y) => x.ppq - y.ppq);
}

export class BounceContractError extends Error {}

/**
 * Both captures must exist and cover the passage, or agreement proves nothing:
 * an export that never ran leaves one pass, and an empty pass agrees with
 * nothing in particular.
 */
export function requireBothPasses(passes: Pass[], endPpq = PASSAGE_END_PPQ): void {
  if (passes.length !== 2)
    throw new BounceContractError(
      `expected 2 passes (realtime, offline) and the capture has ${passes.length}; ` +
        (passes.length < 2
          ? 'the offline export did not reach the probe'
          : 'something else moved the transport in this session'),
    );
  for (const [i, label] of [[0, 'realtime'], [1, 'offline']] as const) {
    if (passes[i].endPpq < endPpq - 1 - 1e-9)
      throw new BounceContractError(
        `the ${label} pass ends at ppq ${passes[i].endPpq}, short of the passage (${endPpq})`,
      );
  }
}

/** Ppq shift applied by the red path: a quarter of a beat, one 16th step. */
export const MUTATION_SHIFT_PPQ = 0.25;

/**
 * The red path: POLY_E2E_MUTATE=s06-perturb-capture.
 *
 * Shifts the offline pass's first note in the passage by a 16th, so the
 * comparison must fail naming that position. It perturbs the capture, never
 * the plugin.
 */
export function applyMutation(offline: Pass, mutate: string | undefined): Pass {
  if (!mutationActive('s06-perturb-capture', mutate)) return offline;
  const i = offline.notes.findIndex((n) => n.ppq >= 0);
  if (i < 0) return offline;
  const notes = offline.notes.map((n, k) => (k === i ? { ...n, ppq: n.ppq + MUTATION_SHIFT_PPQ } : n));
  return { ...offline, notes };
}
