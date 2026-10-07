import { pitchesByPpq, type Pass } from './transport-contract';

// M004 S05 (DAW05) — two Poly instances in one project keep to themselves.
//
// THE FIXTURE is tests/cubase/fixtures/poly-2instance.cpr: track 1 is Poly on
// its default patch, track 2 a second Poly on "Reich Phasing", and each feeds a
// probe of its own (tracks 3 and 4, both monitoring). poly_midi_probe gives each
// live instance its own file -- probe.jsonl and probe-2.jsonl -- by the lowest
// free slot it claims at initialize(), which follows load order and is not
// assumed to follow track order. So nothing below says which file is which
// instance; the assertion is about the PAIR.
//
// THE ASSERTION is asymmetry: each capture contains something the other does
// not. Shared state -- one state blob behind both instances, one bridge driving
// both, one capture written by both probes -- makes the two captures equal, and
// then neither has anything of its own. A one-sided difference is not enough: a
// capture that is a strict subset of the other means one instance is emitting
// the other's notes as well as, or instead of, its own.

/** The preset the fixture saved on its second instance (presets.json index 6). */
export const SECOND_PRESET_INDEX = 6;
export const SECOND_PRESET_NAME = 'Reich Phasing';

/** Count of (position, pitch) pairs present in `a` and absent from `b`. */
export function ownNotes(a: Pass, b: Pass): number {
  const ma = pitchesByPpq(a);
  const mb = pitchesByPpq(b);
  let own = 0;
  for (const [key, pitches] of ma) {
    const other = mb.get(key) ?? [];
    for (const p of pitches) if (!other.includes(p)) own++;
  }
  return own;
}

export interface Asymmetry {
  onlyFirst: number;
  onlySecond: number;
}

export function asymmetry(first: Pass, second: Pass): Asymmetry {
  return { onlyFirst: ownNotes(first, second), onlySecond: ownNotes(second, first) };
}
