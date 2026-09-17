import { mutationActive } from './preset-contract';
import { noteOns, type ProbeNote } from './probe-assert';

// M004 S03 (DAW03) — the property a host's transport can break.
//
// Poly's timing convention is that envelope and cycle phase derive from
// ABSOLUTE PPQ and are never accumulated. Golden tests enforce determinism for
// linear playback only, and linear playback is the one case where an
// accumulator and a derivation agree: they diverge the moment the transport
// jumps.
//
// So the property stated positionally: what Poly emits at a given PPQ is a
// function of that PPQ, not of how the transport arrived there. A locate
// backwards or a cycle wrap makes the transport visit the same PPQ range more
// than once in a single session, and the notes emitted must match across those
// visits. An accumulating lane would emit the second visit shifted by however
// long the transport had been rolling.
//
// This needs no second capture and no golden: one probe JSONL from a session
// that jumps contains the repeated visits, and the comparison is over that file
// alone.

/** One contiguous forward run of the transport, cut at each backward jump. */
export interface Pass {
  index: number;
  notes: ProbeNote[];
  startPpq: number;
  endPpq: number;
}

/**
 * Split a probe capture into passes.
 *
 * A pass ends where the stream's PPQ goes backwards by more than `epsilon` —
 * that is a locate or a cycle wrap. Small backward wobble within a pass is
 * tolerated because note-ons at the same step can be written in any order
 * within the block that produced them.
 */
export function splitPasses(notes: ProbeNote[], epsilon = 1e-6): Pass[] {
  const passes: Pass[] = [];
  let current: ProbeNote[] = [];
  let high = -Infinity;

  const flush = () => {
    if (current.length === 0) return;
    const ppqs = current.map((n) => n.ppq);
    passes.push({
      index: passes.length,
      notes: current,
      startPpq: Math.min(...ppqs),
      endPpq: Math.max(...ppqs),
    });
    current = [];
  };

  for (const note of notes) {
    if (note.ppq < high - epsilon) {
      flush();
      high = note.ppq;
    } else {
      high = Math.max(high, note.ppq);
    }
    current.push(note);
  }
  flush();
  return passes;
}

/** Pitches emitted at each PPQ within a pass, quantised so float noise cannot split a step. */
export function pitchesByPpq(pass: Pass, quantum = 1e-3): Map<string, number[]> {
  const byPpq = new Map<string, number[]>();
  for (const note of noteOns(pass.notes)) {
    const key = (Math.round(note.ppq / quantum) * quantum).toFixed(6);
    const list = byPpq.get(key) ?? [];
    list.push(note.pitch);
    byPpq.set(key, list);
  }
  for (const list of byPpq.values()) list.sort((a, b) => a - b);
  return byPpq;
}

export interface Divergence {
  ppq: number;
  firstPass: number;
  laterPass: number;
  firstPitches: number[];
  laterPitches: number[];
}

/**
 * Compare every PPQ the transport visited more than once.
 *
 * Comparison is over the two passes' OVERLAPPING PPQ RANGE, and a position
 * present in one pass but not the other counts as a divergence with an empty
 * side. An earlier version compared only positions present in both, and the
 * unit test caught what that misses: an accumulating lane shifts notes OFF the
 * positions they should occupy, so the revisited position simply goes silent
 * and a both-sides-only comparison sees nothing to compare.
 *
 * Outside the overlap there is nothing to say: a pass that covers less ground
 * than another did not diverge, the transport just did not get that far, and
 * treating that as a failure would break the honest case where a cycle wraps
 * early.
 *
 * Velocity is deliberately not compared. It is derived from the same absolute
 * step index, so it would match, but comparing floats adds a tolerance question
 * without adding signal: which pitches sound at a position is the load-bearing
 * claim.
 */
export function findDivergences(passes: Pass[]): Divergence[] {
  const out: Divergence[] = [];
  if (passes.length < 2) return out;

  const maps = passes.map((p) => pitchesByPpq(p));
  for (let later = 1; later < passes.length; later++) {
    for (let first = 0; first < later; first++) {
      const lo = Math.max(passes[first].startPpq, passes[later].startPpq);
      const hi = Math.min(passes[first].endPpq, passes[later].endPpq);
      if (hi < lo) continue;

      const keys = new Set([...maps[first].keys(), ...maps[later].keys()]);
      for (const key of keys) {
        const ppq = Number(key);
        if (ppq < lo - 1e-9 || ppq > hi + 1e-9) continue;
        const firstPitches = maps[first].get(key) ?? [];
        const laterPitches = maps[later].get(key) ?? [];
        if (firstPitches.join(',') !== laterPitches.join(',')) {
          out.push({
            ppq,
            firstPass: passes[first].index,
            laterPass: passes[later].index,
            firstPitches,
            laterPitches,
          });
        }
      }
      break; // compare against the earliest pass that overlaps this one
    }
  }
  return out.sort((a, b) => a.ppq - b.ppq);
}

export class TransportContractError extends Error {}

/**
 * The capture must actually contain a jump, or the check proves nothing.
 *
 * A session where the transport never went backwards yields one pass, no
 * repeated PPQ, and therefore no divergences — a green result from a run that
 * never exercised the property. That is the vacuous-predicate failure this
 * programme has found repeatedly, so it is an error rather than a pass.
 */
export function requireRepeatedVisits(passes: Pass[]): void {
  if (passes.length < 2)
    throw new TransportContractError(
      'the probe capture contains a single forward pass, so the transport never ' +
        'located or wrapped: this run cannot say anything about PPQ-derived phase. ' +
        'Check that the motion scenario ran and that the MIDI Remote locate/cycle ' +
        'bindings took effect.',
    );

  const maps = passes.map((p) => pitchesByPpq(p));
  const first = maps[0];
  const overlap = maps
    .slice(1)
    .some((m) => [...m.keys()].some((k) => first.has(k)));
  if (!overlap)
    throw new TransportContractError(
      `the capture has ${passes.length} passes but no PPQ appears in more than one, ` +
        'so nothing is comparable. The transport jumped forwards only, or the ' +
        'passes do not overlap.',
    );
}

/**
 * The red path: POLY_E2E_MUTATE=s03-accumulated-phase.
 *
 * Shifts every note after the first pass by the offset a naive accumulator
 * would have drifted — the elapsed length of the passes before it. That is
 * exactly the defect DAW03 names, and applying it to the CAPTURE rather than to
 * the engine proves the comparison catches it without needing a deliberately
 * broken build.
 */
export function applyMutation(passes: Pass[], mutate: string | undefined): Pass[] {
  if (!mutationActive('s03-accumulated-phase', mutate)) return passes;
  let drift = 0;
  return passes.map((pass, i) => {
    if (i === 0) {
      drift = pass.endPpq - pass.startPpq;
      return pass;
    }
    const shifted = {
      ...pass,
      notes: pass.notes.map((n) => ({ ...n, ppq: n.ppq + drift })),
      startPpq: pass.startPpq + drift,
      endPpq: pass.endPpq + drift,
    };
    drift += pass.endPpq - pass.startPpq;
    return shifted;
  });
}
