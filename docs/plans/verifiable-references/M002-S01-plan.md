# M002/S01 — The manifest exists and cannot drift from the bibliography

**Slice:** M002/S01 in `docs/plans/verifiable-references/ledger.md`
**Rows:** VR04 (nothing records obtainability or accuracy), VR05 (the archive
root is a personal path)
**Classification:** bounded. A new hand-authored JSON beside the existing
`site/src/data/euclidean-appendix.json`, and new tests following
`site/tests/presets-json-schema.test.mjs`, which already validates a JSON file's
shape against a closed enum and derives its expected count from a source of
truth rather than a literal. No new subsystem, no interface change.

## Task status

- [x] 1. The schema and a skeleton record for all 107 anchors
- [x] 2. Completeness, proved in both directions
- [ ] 3. The archive root comes from the environment, never from git

## Definition of Done

Copied verbatim from the slice:

- [ ] `site/src/data/references.json` has a declared shape carrying, per entry:
      anchor id, obtainability, description verdict, archive filename, and
      ISBN/DOI where one exists
- [ ] A test fails when a bibliography anchor has no manifest record, and when a
      manifest record names an anchor that does not exist
- [ ] The test is shown to fail in both directions before being trusted

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `guards` | `bash scripts/check-guards.sh` |

`guards` is owed because of VR05: `check-personal-paths` is the guard this slice
must stay green against, and it is the one that caught the absolute archive path
in the vision's first draft.

## The shape, as agreed

Keyed by anchor so duplicate anchors are impossible by construction rather than
by assertion. Every field the definition of done names has a home.

```json
{
  "schemaVersion": 1,
  "entries": {
    "ref-9": {
      "obtainability": "open-access",
      "evidence": "D-Scholarship@Pitt record; 403s to automation, owner loaded it in a browser",
      "checked": "2026-09-20",
      "description": "verified",
      "note": "African pianism, not drumming — the title the guide printed did not exist",
      "identifier": { "doi": null, "isbn": null },
      "price": null,
      "archiveFile": "ref-9-oluranti-2012.pdf"
    }
  }
}
```

**Closed enums.**

- `obtainability`: `open-access`, `borrowable`, `purchasable`, `library-only`,
  `browser-only`, `unobtainable`, `unassessed`
- `description`: `verified`, `mismatch`, `unverified`

**Conditional requirements**, each enforced by task 1's shape test:

- `price` is non-null exactly when `obtainability` is `purchasable` — a
  purchasable verdict without a price is not a decision anyone can act on, which
  is VR06's whole point
- `note` is non-empty when `description` is `mismatch` — VR07 requires what the
  source actually is to be recorded, so M004 need not repeat the work
- `checked` is a `YYYY-MM-DD` string whenever `obtainability` is not
  `unassessed`, and null while it is

## Ground truth, measured today

| | |
|---|---|
| numbered entries (`ref-N`) | 43 |
| Further Reading entries (`fr-…`) | 64 |
| total anchors | 107 |
| entries carrying a DOI | 2 |
| entries carrying any URL | 44 links across 43 entries |
| Further Reading entries carrying a URL | **0** |

The last row is VR08's scope and is the reason the manifest carries an
`identifier` field at all: for 64 entries there is currently no route to the
work of any kind.

---

## Task 1 — The schema and a skeleton record for all 107 anchors

**Consumes:** nothing. **Produces:** the manifest tasks 2 and 3 assert against,
and the record set M002/S02 fills.

Every record starts `unassessed` / `unverified`. That is the honest starting
state and it is the point of VR04: silence currently implies "checked", and a
skeleton makes the absence of a verdict explicit rather than invisible.

1. Create `site/tests/references-manifest.test.mjs`. Assert the shape: top-level
   `schemaVersion` is 1, `entries` is a plain object, and every record has
   exactly the eight declared keys with values of the declared types, both enums
   closed, and the three conditional requirements above.

   Follow `presets-json-schema.test.mjs` on one specific point: a pattern or
   lookup that stops matching must **throw**, not silently assert nothing. An
   enum read from a file that has been renamed is a broken check, not a passing
   one.

2. Run `npm --prefix site test`. Watch it fail because
   `site/src/data/references.json` does not exist.

3. Create `site/src/data/references.json` with one record per anchor, generated
   by reading the anchors out of `appendix-references.mdx` so the set cannot be
   mistyped. Seed the three entries M001 already settled — `ref-6`, `ref-22`,
   `ref-26` — with their real verdicts and evidence, because that work is done
   and recording it as `unassessed` would be false.

4. Re-run. Watch it pass.

5. **Prove the shape test bites**, one mutation per rule, each reverted:
   - an unknown `obtainability` value → red
   - a record missing `description` → red
   - `obtainability: "purchasable"` with `price: null` → red
   - `description: "mismatch"` with an empty `note` → red

6. Run `format`, `site-unit`, `guards`. Commit with `Slice: M002/S01`,
   `Rows: VR04`.

## Task 2 — Completeness, proved in both directions

**Consumes:** task 1's manifest. **Produces:** the guard that makes M002/S02's
completion checkable rather than asserted.

This is the row's actual verification clause: *both arms mutation-proved — an
unrecorded anchor fails, and an orphan record fails.*

1. Add two cases to `site/tests/references-manifest.test.mjs`:
   - every anchor in `appendix-references.mdx` has a record in the manifest,
     failing with the missing anchor ids named
   - every key in the manifest is an anchor that exists in the bibliography,
     failing with the orphan ids named

   The anchor set is read from the `.mdx` at test time, never from a literal
   count. A hardcoded 107 would agree with a stale manifest for exactly as long
   as nobody noticed, which is the failure `presets-json-schema.test.mjs`
   documents this repo shipping once before.

2. Run the suite. Both cases must pass immediately, because task 1 generated the
   manifest from the same anchors — so passing proves nothing yet, and step 3 is
   the step that earns them.

3. **Prove both arms bite**, each mutation applied, tested, reverted:
   - delete one record from the manifest → the missing-anchor case fails, naming
     that anchor
   - add a record keyed `ref-999` → the orphan case fails, naming `ref-999`
   - add a new anchor to a scratch copy of the bibliography → the missing-anchor
     case fails, naming it

4. Run `format`, `site-unit`, `guards`. Commit with `Slice: M002/S01`,
   `Rows: VR04` — the row closes here, once both arms are proved.

## Task 3 — The archive root comes from the environment, never from git

**Consumes:** the manifest. **Produces:** the resolution M003's archive slices
use.

VR05 exists because `check-personal-paths` rejected the absolute archive path in
the vision's own first draft. The fix is structural: nothing machine-specific is
ever written to a tracked file.

1. Add `site/src/data/references-archive.mjs` exporting
   `archiveRoot()` and `resolveArchiveFile(name)`. `archiveRoot()` returns
   `process.env.POLY_REFERENCES_ARCHIVE` when set and non-empty, else
   `<repo>/.references`. `resolveArchiveFile` rejects any name containing a path
   separator or `..`, so a manifest entry can never point outside the archive.

2. Add cases to `site/tests/references-manifest.test.mjs`:
   - with `POLY_REFERENCES_ARCHIVE` set to a temporary directory, `archiveRoot()`
     returns it
   - unset, `archiveRoot()` ends in `.references`
   - every non-null `archiveFile` in the manifest is a bare filename — no
     separator, no `..`
   - `resolveArchiveFile('../etc/passwd')` throws

   Run and watch the first three fail before the module exists.

3. Write the module. Re-run, watch them pass.

4. Add `.references/` to `.gitignore`, and record the variable in
   `docs/plans/verifiable-references/browser-worklist.md` beside the archive
   sentence it already carries, so the one place that tells a human where to put
   a PDF also tells them how the repo finds it.

5. **Prove the guard is real, not incidental**: temporarily add a record whose
   `archiveFile` is an absolute path under a home directory, confirm both the
   bare-filename case fails **and** `bash scripts/check-guards.sh` reports the
   personal path, then revert. A guard that only the test catches leaves the
   committed-path failure mode open.

6. Run `format`, `site-unit`, `guards`. Append the slice's evidence, tick every
   definition-of-done box, set VR05 `done` and the slice `done`, run
   `jk-standards ledger`, and commit with `Slice: M002/S01`, `Rows: VR05`.
