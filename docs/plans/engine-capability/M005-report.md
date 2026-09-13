---
class: gated
---

# M005 — Review report

Status: current (2026-09-13)

Generated from `docs/plans/engine-capability/ledger.md`, git, and
`M005-decisions.md` for the review that precedes `/jk:ship`.

**Vision:** The path from `engine/src/presets.cpp` to
`site/src/generated/presets.json` tells the truth — it rebuilds when the engine
changes, and it carries enough of a lane that a consumer can tell an authored
pattern from a generated one.

**Branch:** `milestone/M005-preset-pipeline`, cut from `main` at `56eb65e`.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M005/S01 | The generator rebuilds its emitter | PIPE01 | done |
| M005/S02 | `presets.json` carries the pattern | PIPE02 | done |

## Definition of done

**M005/S01**

- [x] Editing `engine/src/presets.cpp` and running the generator produces JSON
      that reflects the edit, with no explicit build step
- [x] A stale `presets.json` fails the site suite mechanically, rather than
      depending on someone noticing the count is wrong
- [x] The generator still succeeds from a clean tree, where the build directory
      does not yet exist
- [x] The hardcoded preset count in `presets-json-schema.test.mjs` is gone,
      derived from `kFactoryPresetCount` instead

**M005/S02**

- [x] A lane running in timeline mode carries its step pattern in
      `site/src/generated/presets.json`
- [x] `schemaVersion` is bumped, and the generator rejects a JSON written at the
      previous version rather than reading it as if the field were absent
- [x] A site test answers, from `presets.json` alone, whether `Cuban Son
      Montuno`'s clave is the son clave or `E(5,16)`
- [x] Every existing consumer of the file still passes

## Validation

Run on the current head, not when each slice landed.

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | pass |
| `unit` | `ctest --test-dir build --build-config Release` | pass, 592 tests |
| `engine-isolation` | `ctest --test-dir build-engine` | pass, 469 tests |
| `site-unit` | `npm --prefix site test` | pass, 277 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass, 256 tests |

## Traceability

Every commit on the branch carries a `Slice:` trailer. **No untraced commits.**

| Commit | Subject | Slice | Rows |
|---|---|---|---|
| `b51fe09` | docs(plans): plan M005's two slices and record its decisions | M005/S01, M005/S02 | — |
| `19342bc` | fix(site): always build the preset emitter, never trust a stale binary | M005/S01 | — |
| `acdffd0` | test(site): derive the expected preset count from the engine, closing M005/S01 | M005/S01 | PIPE01 |
| `7e23f62` | feat(presets): emit timeline onsets in presets.json, schemaVersion 4 | M005/S02 | — |
| `44f5553` | test(site): assert the shipped claves are authored, not Euclidean, closing M005/S02 | M005/S02 | PIPE02 |

## What a reviewer should look at twice

1. **A mutation proof silently did nothing, and I nearly recorded it as
   evidence.** The first attempt to mutate the clave's onsets edited them as a
   single-line string against a pretty-printed file. Nothing matched, both runs
   stayed green, and that reads exactly like a passing proof. Caught before it
   was written down, and redone JSON-aware. A mutation round that cannot fail is
   the same defect as a predicate that cannot fail — this programme has now hit
   both, in M007 and here.
2. **S01's guard is not protected in CI.** The derived count lives in
   `presets-json-schema.test.mjs`, which is not in `check-doc-conformance.sh` —
   one of the seven files [#272](https://github.com/JimAKennedy/poly/issues/272)
   names as unprotected. It bites locally and in the pre-push gate, not in CI.
   PIPE01's definition of done said "fails the site suite" and is met, but
   nobody should read CI protection into it. S02's case was deliberately placed
   in a protected file instead, and the doc-conformance count rising 255 → 256
   is the evidence it runs there.
3. **PIPE01's fix is a deletion, not a detector.** `ensureEmitter` now always
   builds. cmake is incremental, so an unchanged tree costs a no-op — cheaper
   and more honest than any mtime comparison against C++ sources.
4. **The `schemaVersion` bump moved six places, and deliberately not a
   seventh.** `webui/tests/startup.spec.mjs` has a `schemaVersion` that a grep
   for the word finds and that looks like it belongs; it is the WebUI bridge's
   `POLY_SCHEMA_VERSION`, a different number, and was left alone.
5. **The PR this milestone's ledger arrived in had to be rebased mid-run.**
   #294 landed while #295 was open. It was rebased and force-pushed with
   `--force-with-lease` after checking it carried no reviews, and the full local
   gate was re-run on the rebased tree before the push.

## Decisions

Verbatim from `M005-decisions.md`.

## 2026-09-13 — starting M005

- **Decision:** The run waited for PR #295 to merge before cutting
  `milestone/M005-preset-pipeline`. — **Why:** M005 existed only on that PR, so
  a branch cut from the default branch would have had no milestone to execute
  against. The wait was spent front-loading decisions and writing both plans,
  which carried onto the branch as untracked files.
- **Decision:** #295 was rebased onto `ec64e4e` and force-pushed with
  `--force-with-lease` rather than updated through GitHub's update-branch.
  — **Why:** #294 landed while #295 was open, leaving it BEHIND against a base
  that requires branches to be current. Rebase is this repo's convention for a
  stale PR. Checked first that the PR carried no reviews and no review
  comments, so no reviewer's anchors were discarded; the full local gate was
  re-run on the rebased tree before the force-push, because a rebased tree has
  never been checked in that combination.
- **Decision:** Both slices classified **bounded**; no design document.
  — **Why:** S01 deletes one early return and retargets one assertion. S02 adds
  a field to an emitter that already serialises fifteen per-lane fields and
  bumps a version it already guards. Neither introduces a mechanism.

## 2026-09-13 — planning M005/S01

- **Q:** How should a stale `presets.json` fail mechanically, given S01's fix
  prevents staleness at source? — **A:** Derive the count from the header.
- **Decision:** `presets-json-schema.test.mjs` reads `kFactoryPresetCount` from
  `engine/include/poly/presets.h` and asserts `presets.json` matches; the
  regenerate-and-diff alternative was rejected. — **Why:** The header parse is
  cheap, needs no compiler, and would have caught the exact M001 failure (43
  emitted against 44 in the engine). Registering `presets.json` with
  `jk-standards`' `generated-freshness` would catch content drift too, but would
  make `jk-standards all` build `poly_presets_emit` — turning a seconds-long doc
  gate into one needing a full engine compile, which is disproportionate to a
  guard sitting behind a root cause this same slice removes. The limit is
  recorded rather than hidden: it catches count drift, not a preset whose
  contents changed while the count held.
- **Decision:** The plan requires the count guard to be mutation-proved twice —
  once for a wrong count, once for a renamed constant. — **Why:** A regex that
  matches nothing reads exactly like one that passes. `CLAUDE.md` records this
  repo being bitten by precisely that (`[[:space:]]` parsed as a literal set,
  rule green and matching nothing), and the theory-audit programme spent M007
  removing predicates that could not fail. One assertion, two ways to be
  vacuous, so two proofs.
- **Decision:** No site test currently reads a C++ header, so this introduces
  new coupling. Accepted rather than avoided. — **Why:** The alternative is the
  hardcoded number that had to be hand-edited twice in M001/S02, once per preset
  added. The coupling is one regex against one `static constexpr` line, and it
  fails loudly if that line is renamed.

## 2026-09-13 — planning M005/S02

- **Q:** What shape should the timeline pattern take in `presets.json`?
  — **A:** Onset positions.
- **Decision:** Emit `"onsets": [0, 3, 6, 10, 12]`, only when `timeline` is
  true. — **Why:** Compact against up to 32 booleans per lane across 43 timeline
  lanes in a file the site downloads, and it is the form the guide itself uses
  when stating a pattern — `theory-afro-cuban.mdx` says "hits 0, 3, 6, 10, 12
  for 3-2" — so a test comparing the file against the prose compares like with
  like. `fixedPatternLength` already carries the cycle length, so nothing is
  lost. Emitting it only for timeline lanes makes the field's absence meaningful
  in itself: it says the pattern is derived rather than authored.
- **Decision:** The `schemaVersion` bump touches six places, and the plan names
  all six. — **Why:** Three are functional and must move together or the
  generator rejects its own output — the emitter, the generator's `!== 3` guard,
  and the schema test. Three name version 3 in comments only. A seventh,
  `webui/tests/startup.spec.mjs`, uses an unrelated `POLY_SCHEMA_VERSION` for
  the WebUI bridge; the plan says explicitly not to touch it, because a grep for
  `schemaVersion` finds it and it looks like it belongs.
- **Decision:** S02's plan halts if it is reached before S01 has landed.
  — **Why:** It must regenerate `presets.json`, and doing so against a stale
  emitter is exactly the failure S01 exists to remove. The ledger already
  declares the dependency; the plan makes it a stop rather than something to
  discover.
