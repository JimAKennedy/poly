---
class: gated
---

# M002 — Decisions

Append-only. One entry per planning session or in-flight judgment call, so the
milestone's review can see what shaped it without reconstructing it from diffs.

## 2026-09-13 — planning M002

- **Decision:** M002/S01 classified **architectural**; a design document was
  written and approved before any code (`M002-S01-design.md`). M002/S02 is
  **bounded** — it edits prose and flips triage verdicts, against a capability
  S01 will already have shipped. — **Why:** S01 adds an enum and two fields to
  `LaneConfig`, changes how patterns are derived, bumps the state version and
  the emitter schema, and touches the WebUI bridge. That restructures how parts
  fit together, which is the architectural test.

- **Q:** Rule 5 names four interlock styles, but Poly gives each lane one MIDI
  note and *norot* is defined by pitch oscillation. Which modes should ship?
  — **A:** Three rhythmic modes, norot documented as inexpressible.
- **Decision:** `KotekanMode { NyogCag, Telu, Empat }`. — **Why:** The three are
  rhythmic and therefore expressible in a one-note-per-lane model. Giving norot
  a rhythmic stand-in would attach a traditional style name to something that is
  not that style, which is the class of overclaim the theory-audit programme
  spent seven milestones removing.

- **Q:** How should polos–sangsih overlap be controlled? — **A:** A per-lane
  overlap count, default 0.
- **Decision:** `kotekanOverlap`, an integer count of structural points where
  both parts strike — cycle boundary, then phrase join, then midpoint, the order
  Rule 4 names them. — **Why:** Default 0 reproduces today's strict complement
  exactly, so the determinism goldens and three of the four kotekan presets stay
  byte-identical, while a patch that sets it makes Rule 4's strict predicate
  able to fail for the first time. Reusing accent masks was rejected: that is
  the workaround Rule 4 tells readers to use *until kotekan modes ship*, so the
  row could not close.

- **Q:** Four shipped presets derive a kotekan complement. Should any adopt a
  non-strict mode? — **A:** `Balinese Kotekan` only.
- **Decision:** Its guiro lane takes `Telu` with overlap 1; the other three keep
  `NyogCag`. — **Why:** The one preset named for the practice demonstrates the
  capability, so "the mode reaches `presets.json`" is provable from something a
  user loads rather than only a test fixture. Applying Balinese style names to
  `Ewe Polymetric Ensemble` or `Afro-Electronic Fusion` would be the
  cross-tradition mislabelling the theory audit repeatedly corrected.

- **Decision:** The two new fields are **state-only** — serialized and
  WebUI-editable, with no VST3 parameter. — **Why:** Measured before designing:
  `kParamsPerLane = 16` and `kKotekanSource` occupies slot 15, so the per-lane
  family is full, and raising the stride would shift every lane's parameter IDs
  and break automation in existing projects. State-only lane fields are the
  established pattern regardless — 16 of `LaneConfig`'s 35 fields have no
  parameter, including `hitCount`, `cycle`, `cellSizes` and `microTimingMs` —
  and a style choice does not want an automation lane.

- **Q (raised by the design's risk section):** `Telu` and `Empat` are modelled
  rhythmically while the terms also carry pitch meaning. Keep the traditional
  names? — **A:** Keep them, and state explicitly on the site that only the
  rhythmic aspect is modelled, directing the reader to the references for the
  pitch aspects.
- **Decision:** A required, specified disclosure, promoted from a caution in the
  design to a definition-of-done item on S02: the page must say Poly models only
  the rhythmic dimension of *telu* and *empat* — the cell length the interlock
  repeats on — direct the reader to Tenzer (2000) and Vitale (1990) for the
  pitch dimension, and name *norot* as not expressible and why. — **Why:** Both
  sources are already cited on the page at Tier A, and their appendix
  annotations are exactly on point: Tenzer is "the authoritative analysis of
  kotekan varieties" and Vitale "shows real kotekan is not a pure set
  complement", which is Rule 4's own substance. The alternative — dropping the
  names for `Cell3`/`Cell4` — was rejected because it would cut the reader off
  from the literature the guide is built on.

## 2026-09-13 — executing M002/S01 task 1 (judgment call)

- **Decision:** Overlap forces a step on **only where the source also strikes**,
  rather than unconditionally as the design's wording implied. — **Why:** The
  purpose is a *shared* strike. Forcing a step where the source is silent
  produces no intersection at all — it just makes the complement denser, which
  is not what Rule 4 describes. The design said "force `pattern[s] = true` at
  the first N structural points"; the implementation reads the source at that
  step first. Obviously right and too small to halt for, but it is a narrowing
  of the design's text, so it is recorded rather than left in the diff.
- **Decision:** `GrooveStateCopyBenchmark.ReportsFactSizes`'s pinned size was
  updated from 13712 to 13776 with a comment naming this milestone. — **Why:**
  That test exists to make struct growth deliberate and documented, and its
  comments already record each previous growth the same way. Updating it is the
  intended workflow, not a test being bent to fit.

## 2026-09-13 — executing M002/S01 task 3 (finding and correction)

- **Finding:** M002/S01's declared validation set omitted `doc-discipline`,
  while the slice changes `engine/include/poly/types.h`, `engine/src/engine.cpp`
  and `engine/tools/emit_presets.cpp` — all mapped sources in
  `.github/docs-drift-map.yml`. Tasks 1 and 2 therefore passed their full
  declared set while leaving two doc-drift violations behind them. Found only
  because task 3 ran `doc-discipline` opportunistically after committing.
- **Decision:** `doc-discipline` added to the slice's validation line.
  — **Why:** It corrects an under-declaration rather than widening the slice:
  the work is unchanged, the gate is now honest about what this slice can break.
  A slice that edits mapped sources and does not owe the check that guards them
  is the same defect class M006 closed one level up.
- **Decision:** The `docs/engine-spec.md` violation was fixed by writing the
  derivation, not by regenerating. — **Why:** That doc's `LaneConfig` table is
  generated but curated — 17 of 35 fields — and correctly omits the two new
  ones, so `generate-param-docs.mjs` produced no diff. What had actually gone
  stale was the prose describing how a lane's pattern is derived, which is
  exactly what this slice changed.
- **Decision:** The `docs/testing-strategy.md` violation was discharged with a
  `Docs-Not-Affected:` trailer rather than a doc edit. — **Why:** That rule's
  own stated reason exempts this case: "Per-file additions to existing binaries
  or suites do not — they exercise the taxonomy, they don't change it."
  `kotekan_mode_tests.cpp` is a per-file addition to the existing `poly_tests`
  binary; no new binary, Playwright surface or JS entry-point. The trailer
  quotes that exemption so a reader can check the claim.
