---
class: gated
---

# M002 — The eleven formulae are factory presets

> Every formula Arom transcribes in the chapter's worked examples ships as a
> factory preset that plays his marks exactly, and CI fails if one position
> moves.

Ledger: `docs/plans/central-africa/ledger.md` · branch
`milestone/M002-central-african-presets` · 6 commits, **none untraced**

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M002/S01 | Eleven formulae are presets | CA04–CA18 | done |

## What changed

**Eleven presets, indices 45–55, in a new "Central African" category.** Seven
are Aka formulae (Ngbolu, Mokondi, Yombe, Monzoli, Mombenzele, Zoboko, Bondo)
plus the Ngbolu drums on their own. The other three are Banda-Dakpa: Gaza,
Zande: Kponingbo and Banda-Linda: Aga-torumo. They live in a new
`engine/src/presets_central_africa.cpp` that cites Arom for each. Every lane
plays at probability 1 with no mutation, drift, swing, humanize or phrase
gating, and the macros sit at the values where a Euclidean lane's hits and
rotation are left exactly as set. Each dì.kétɔ̀ lane stays Euclidean, so turning
Aka: Ngbolu's Rotation to 22, 19, 13 or 10 places the mould where mò.nzòlì,
yómbè and zɔ̀bɔ̀kɔ̀, mò.kóndí or mò.mbénzélé put it. "African" is now "West
African".

**The proof is the point.** `tests/central_african_preset_tests.cpp` resolves
each preset through the production pipeline, renders it with the engine and
compares every lane, position by position, with Arom's grid. It also compares
the per-position count of sounding parts with his `(count)` row. A second case
per formula checks that the fixture's own rows sum to that count row, so a
transcription slip fails before any engine is involved. Moving one dì.kétɔ̀
rotation, or deleting one fixture mark, turned the harness red.

**Every count site agrees.** The plugin's lane-name table, the WebUI mock host
and its tests, both expected-category lists, the taxonomy doc and its
conformance test, the Cubase sweep's wording, the emitter comment, the
changelog's musician block and the drift map now say 56 presets in 11
categories. The committed WASM engine was rebuilt to match. Two surfaces were
already stale before this milestone: the mock host was at 43, and the modal e2e
spec expected 43 against 45.

## Definition of done

- [x] Eleven presets exist in a new "Central African" category, and the
      existing "African" category is named "West African"
- [x] Every preset's lanes, tiled to the formula's period, equal Arom's grid
      position by position at the preset's own macro settings, and the
      per-position count of marking parts equals his `(count)` row, asserted
      by a test that fails if one position moves
- [x] Every lane that carries marks has Mutation, Drift, Swing and Humanize at
      0, Probability 1 and no phrase gating
- [x] Every place that hard-codes the preset or category count agrees with 56
      and 11, and the WebUI lists all 56 presets with a lane name for each
      Central African lane
- [x] The committed WASM engine contains the new presets

## Validation

| Token | Result |
|---|---|
| `format` | pass |
| `unit` | pass: 731 tests (710 after task 1) |
| `rt-safety` | pass |
| `site-unit` | pass: 402 tests |
| `webui-e2e` | pass: 360 tests |
| `cubase-harness` | pass |
| `doc-discipline` | pass |
| `e2e` | pass: every gate in `site-verify-local.sh`, including preset-consistency |

`guards` also passed. The modal category spec, which no local gate runs, passed
against a preview of the fresh build.

## Look at twice

- **The locked-referent rule now has a category exception.** You chose this:
  Central African presets need at least one locked lane, and every other
  category still needs exactly one.
- **The bòndò count, a judgment call.** The harness counts Arom's parts, not
  lanes, so bòndò's three drums on one model rhythm match his count row. The
  vision's stated reason for three lanes was wrong; the lanes themselves are as
  you chose.
- **Kponingbo's pellet bells use GM note 54.** The site's preview loader
  resolves it by note to the manifest's tambourine, and the sample-loader
  snapshot records that fallback.
- **Rows 14–44 of `kWebPresetLaneNames` are now explicit empty rows**, because
  the table is filled by position. They keep the default names, as before.
- **The local `build-presets/` tree was reconfigured.** It cached an old macOS
  deployment target that current Xcode rejects. It's ignored, so nothing
  tracked changed, but another machine with an old cache will hit the same
  failure.
- **No preset has been heard in a DAW yet.** The Cubase nightly's sweep will
  select all 56 once this merges, and its loopMIDI port on JIMW1 was missing on
  2026-10-15.

## Traceability

Commits are listed by subject, not SHA: the branch is squash-merged, and a
branch SHA in a durable document stops resolving once it is.

| Commit | Slice | Rows |
|---|---|---|
| docs(plans): plan central-africa M002/S01 | M002/S01 | — |
| feat(presets): Aka: Ngbolu, proven against Arom's grid by rendering | M002/S01 | CA04, CA15, CA16 |
| feat(presets): the other ten Central African formulae | M002/S01 | CA05–CA14 |
| chore(presets): every count and name site agrees with 56 presets | M002/S01 | CA17 |
| build(wasm): rebuild the engine with the Central African presets | M002/S01 | CA18 |
| docs(plans): write the M002 report | M002/S01 | — |

Untraced commits: none.

## Decisions

The contents of `M002-decisions.md`, verbatim:

## 2026-10-15 — planning M002/S01

- **Q:** `PresetConformance.EveryPresetHasExactlyOneLockedReferent` requires
  exactly one fixed lane per factory preset, but four of the formulae (àgà,
  mò.kóndí, mò.nzòlì, mò.mbénzélé) have several fixed parts, and the only ways
  to exempt a drawn lane — probability, mutation, drift, phrase gating — move
  its marks. A: change the test for the Central African category only (at
  least one locked lane, with the reason cited to p. 230), or B: build the
  drawn parts another way? — **A:** A
- **Q:** Approve the M002/S01 design: presets appended at indices 45–55;
  macros at identity; timeline hit counts equal to their marks; the grid test
  rendering through `Engine::renderRange`; the "African" string in the
  metronomic-tradition test replaced by both new names; the taxonomy doc's caps
  raised to 11 categories and 11 presets per category; every stale count site
  brought current; lane names ≤ 15 ASCII characters; WASM, `presets.json` and
  `counts.json` regenerated together; four tasks? — **A:** yes
- **Decision:** append, never insert — **Why:** `kWebPresetLaneNames`, the mock
  host, the taxonomy mapping, two conformance allow-lists and the WebUI's join
  of the WASM list to `presets.json` are all keyed by preset index
- **Decision:** macros complexity 0.5, density 0.5, syncopation 0, swing 0,
  tension 0, humanize 0 — **Why:** at those values `resolveMacros` leaves a
  Euclidean lane's hits, rotation and probability exactly as set
  (`lerp(a, b, 0) = a`), and swing and humanize 0 satisfy the
  metronomic-tradition test
- **Decision:** the grid test renders through `Engine::renderRange` after
  `resolveConstraints(base, resolveMacros(base))`, the production pipeline —
  **Why:** a test that re-derives patterns mirrors the builder it should check;
  rendering checks what the engine emits
- **Decision:** in Kponingbo and Bondo, which have no drawn lane, the pellet
  bells and the rattle carry `Role::AnchorPulse` — **Why:** `lockPresetReferent`
  then fixes that lane, and `lockReferentLane` derives its fixed pattern from
  the lane's own Euclidean recipe, so its marks do not move; the dì.kétɔ̀ lanes
  stay Euclidean so Rotation still moves the mould
- **Decision:** zɔ̀bɔ̀kɔ̀'s mò.kóngò is a drawn lane, not additive cells —
  **Why:** the note allows either; a drawn lane is immune to the macros, and
  nothing established that additive cells are
- **Decision:** the mock host's two missing presets (Rumba Clave, Clapping
  Music) are added with the eleven — **Why:** its tables are positional, so
  indices 45–55 cannot exist without 43 and 44

## 2026-10-15 — judgment call in M002/S01 task 2

- **Decision:** the grid harness counts Arom's parts, not lanes: a part may
  span several lanes, each of which must sound the part's row exactly —
  **Why:** Arom's `(count)` row counts parts, and bòndò's three drums are one
  part on one model rhythm (exx. 101–103), so a lane-wise sum would give 5
  where he writes 3. The vision's reason for three lanes ("three tone colours
  sounding together" make the 3) was wrong; the three lanes stay, as decided,
  because they are three tone colours, and the count row is matched exactly
