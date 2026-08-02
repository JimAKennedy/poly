// Single source of truth for the site's Euclidean (Bjorklund) rhythm generator.
//
// Both the EuclideanDiagram component (what the docs draw) and the audio
// preset surface (site/src/audio/preset-patterns.ts) import this module, so the
// site cannot fork its own generator internally. This is the normative JS
// reference that engine/src/euclidean.cpp mirrors: the exhaustive
// engine-vs-reference equality test in tests/euclidean_tests.cpp
// (ExhaustiveMatchesReference) pins the engine to *this* phase for every (k, n,
// rotation), and site/tests/bjorklund.test.mjs pins the documented Toussaint
// canon spellings so an optimisation on either side cannot silently drift.
//
// Signature note: bjorklund(steps, hits) takes n first then k — matching the
// component's original call site — whereas the engine's euclidean(k, n, ...)
// takes k first. Same distribution, different argument order.

// Bjorklund's pairing/elimination algorithm — the maximally-even distribution
// of `hits` pulses across `steps` steps at rotation 0. Returns one boolean per
// step (true = onset).
export function bjorklund(steps: number, hits: number): boolean[] {
  if (hits >= steps) return Array(steps).fill(true);
  if (hits <= 0) return Array(steps).fill(false);

  let pattern: number[][] = [];
  for (let i = 0; i < hits; i++) pattern.push([1]);
  let remainder: number[][] = [];
  for (let i = 0; i < steps - hits; i++) remainder.push([0]);

  while (remainder.length > 1) {
    const newPattern: number[][] = [];
    const minLen = Math.min(pattern.length, remainder.length);
    for (let i = 0; i < minLen; i++) {
      newPattern.push([...pattern[i], ...remainder[i]]);
    }
    const leftover =
      pattern.length > remainder.length ? pattern.slice(minLen) : remainder.slice(minLen);
    pattern = newPattern;
    remainder = leftover;
  }

  const flat = [...pattern, ...remainder].flat();
  return flat.map((v) => v === 1);
}

// Right-shift rotation, matching the engine's out[i] = base[(i - rotation) mod n]
// and the diagram's original slice expression. Normalised so out-of-range or
// negative rotations wrap safely; for rotation in [0, n) it is byte-identical to
// the component's prior inline `[...slice(n - r), ...slice(0, n - r)]`.
export function rotateRight<T>(items: T[], rotation: number): T[] {
  const n = items.length;
  if (n === 0) return items.slice();
  const shift = ((rotation % n) + n) % n;
  return [...items.slice(n - shift), ...items.slice(0, n - shift)];
}
