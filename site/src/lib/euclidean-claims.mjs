// Reference implementation for Euclidean claim verification (M048 S08).
//
// One shared bjorklund() function so the appendix's E(k,n) claims, chapter
// prose patternClaims front-matter, and the EuclideanDiagram component all
// agree on the canonical pattern for any (k, n). Ports EuclideanDiagram.astro's
// implementation into a plain ESM module tests can import.
//
// The verifier catches the D1-D3 error class from the 2026-07-16 review:
// hand-typed onset strings that disagree with the theoretical claim.

// Bjorklund's algorithm — canonical Euclidean distribution at rotation 0.
// Mirrors EuclideanDiagram.astro:11-33 and engine/src/euclidean.cpp.
export function bjorklund(steps, hits) {
  if (hits >= steps) return Array(steps).fill(true);
  if (hits <= 0) return Array(steps).fill(false);

  let pattern = [];
  for (let i = 0; i < hits; i++) pattern.push([1]);
  let remainder = [];
  for (let i = 0; i < steps - hits; i++) remainder.push([0]);

  while (remainder.length > 1) {
    const newPattern = [];
    const minLen = Math.min(pattern.length, remainder.length);
    for (let i = 0; i < minLen; i++) {
      newPattern.push([...pattern[i], ...remainder[i]]);
    }
    const leftover =
      pattern.length > remainder.length ? pattern.slice(minLen) : remainder.slice(minLen);
    pattern = newPattern;
    remainder = leftover;
  }

  return [...pattern, ...remainder].flat().map((v) => v === 1);
}

// Parse an onset-string like "x . x . ." (with any whitespace between symbols)
// into a boolean array. Accepts x/X/1 as onset, . or 0 as rest. Throws on
// unknown characters so typos surface loudly.
export function parsePattern(str) {
  const cleaned = str.replace(/\s+/g, '');
  const out = [];
  for (const ch of cleaned) {
    if (ch === 'x' || ch === 'X' || ch === '1') out.push(true);
    else if (ch === '.' || ch === '0') out.push(false);
    else throw new Error(`parsePattern: unexpected character '${ch}' in "${str}"`);
  }
  return out;
}

export function onsetCount(pattern) {
  return pattern.reduce((acc, b) => acc + (b ? 1 : 0), 0);
}

// Gap sequence = intervals between consecutive onsets, wrapping from the last
// onset back to the first. Sums to pattern.length. Empty for all-rests.
export function gapSequence(pattern) {
  const n = pattern.length;
  const onsetIdx = [];
  for (let i = 0; i < n; i++) if (pattern[i]) onsetIdx.push(i);
  if (onsetIdx.length === 0) return [];
  const gaps = [];
  for (let i = 0; i < onsetIdx.length; i++) {
    const next = onsetIdx[(i + 1) % onsetIdx.length];
    const cur = onsetIdx[i];
    const gap = i === onsetIdx.length - 1 ? n - cur + next : next - cur;
    gaps.push(gap);
  }
  return gaps;
}

function arrayEq(a, b) {
  if (a.length !== b.length) return false;
  for (let i = 0; i < a.length; i++) if (a[i] !== b[i]) return false;
  return true;
}

export function rotate(pattern, r) {
  const n = pattern.length;
  const shift = ((r % n) + n) % n;
  if (shift === 0) return pattern.slice();
  return [...pattern.slice(n - shift), ...pattern.slice(0, n - shift)];
}

// Return the rotation R such that rotate(bjorklund(k,n), R) === pattern,
// or null if no rotation matches. Used to answer "is this pattern Euclidean?".
export function findEuclideanRotation(pattern, k, n) {
  if (pattern.length !== n) return null;
  if (onsetCount(pattern) !== k) return null;
  const canonical = bjorklund(n, k);
  for (let r = 0; r < n; r++) {
    if (arrayEq(rotate(canonical, r), pattern)) return r;
  }
  return null;
}

// Search all (k, n) up to a bound: returns {k, n, rotation} for the first
// (canonical) match, else null. Useful for cross-checking prose claims of
// the form "pattern X is not Euclidean" — a match at any (k, n) refutes.
export function findAnyEuclideanFit(pattern) {
  const n = pattern.length;
  const k = onsetCount(pattern);
  if (k === 0 || k === n) return { k, n, rotation: 0 };
  const rot = findEuclideanRotation(pattern, k, n);
  return rot === null ? null : { k, n, rotation: rot };
}

// Parse a grouping cell like "2+2+3" or "3 + 3 + 4 + 3 + 3" into an int array.
// Throws on non-numeric tokens.
export function parseGrouping(str) {
  const parts = str.split(/\+/).map((s) => s.trim());
  const nums = [];
  for (const p of parts) {
    if (!/^\d+$/.test(p)) throw new Error(`parseGrouping: non-numeric token '${p}' in "${str}"`);
    nums.push(parseInt(p, 10));
  }
  return nums;
}
