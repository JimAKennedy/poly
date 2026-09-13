---
class: gated
---

# M005 — Decisions

Append-only. One entry per planning session or in-flight judgment call, so the
milestone's review can see what shaped it without reconstructing it from diffs.

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

## 2026-09-13 — shipping M005 (CI failure)

- **Q:** `site-lint` fails on doc-drift: `emit_presets.cpp` changed without
  `docs/preset-taxonomy.md`. How should this land? — **A:** Document the schema
  change.
- **Decision:** `docs/preset-taxonomy.md` gains a "JSON schema version" section
  recording that `presets.json` is emitted at schemaVersion 4 and what versions
  3 and 4 added, with the existing "Adding a new category" step 3 pointing at
  it. — **Why:** The doc already tells a reader *when* to bump the version and
  never said what the current one is, so someone following that procedure had
  no way to know. Real content rather than a `Docs-Not-Affected` suppression,
  and narrower than weakening the drift rule — which would have let a genuine
  taxonomy change through unnoticed.
- **Finding, not fixed here:** the local gate cannot catch this class of
  failure. `jk-standards all` reports `doc-drift: no --base or GITHUB_BASE_REF
  — skipped` when run locally, and `scripts/pre-push-check.sh` sets no base
  either, so both `doc-discipline` and the pre-push gate are structurally blind
  to doc-drift. Seven gates ran green before the push and this one never
  executed; it was found only by setting `GITHUB_BASE_REF` by hand after CI
  failed. M001 passed the same rule incidentally — it changed
  `docs/preset-taxonomy.md` only because adding two presets forced the count
  updates. Worth an issue against the pre-push script.
