# M003/S03 — A guard keeps it true

**Slice:** M003/S03 in `docs/plans/first-release/ledger.md`
**Rows:** FR20 (nothing prevents a bad entry returning), FR23 (`accepted` — the
10–20 target, declined)
**Depends:** M003/S02 — the guard asserts the state that slice produces.
**Classification:** bounded. One new test file in `site/tests/`, following
`theory-bundle-references.test.mjs` and `webui-single-view.test.mjs`, both
written earlier in this programme.

## Task status

- [ ] 1. The guard, and the slice closes

## Definition of Done

Copied verbatim from the slice:

- [ ] A test fails if an appendix entry is cited from nowhere
- [ ] A test fails if an entry is not Tier A, or is `library-only`
- [ ] Both arms are shown to fail before being trusted

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

## Two amendments the definition of done needs

Both were taken by the owner during planning and are recorded in
`M003-decisions.md`.

**The tier arm allows B, and forbids C.** The DoD says "not Tier A". One cited
entry is Tier B — `fr-linn-attack-2020`, the Attack Magazine interview in which
Roger Linn describes the MPC's timing in his own words. It is a primary source,
cited in preference to commentary quoting him second-hand, and Tier B is the
honest label for an interview. Tier C is what actually marks unusable material —
YouTube, marketing blogs, study guides — and every Tier C entry in the guide was
cited only from the deep dives, which are now unpublished. So the guard requires
**A or B** and fails on **C**.

**The guard never pins a count.** FR19's 34 is the number the appendix is
expected to land on, not a property worth enforcing: an entry legitimately added
tomorrow would fail a guard pinned to 34, and the check would then be teaching
people to edit the number rather than the bibliography. The properties are what
is asserted.

---

## Task 1 — The guard, and the slice closes

**Consumes:** M003/S02's reduced appendix. **Produces:** FR20's verification.

1. Add `site/tests/appendix-entry-rules.test.mjs`. It reads
   `site/src/content/docs/appendix-references.mdx`, every other shipping `.mdx`
   in that directory, and `site/src/data/references.json`, and asserts three
   properties:
   - **cited**: every anchor defined in the appendix is cited by at least one
     shipping page, naming any that are not
   - **tier**: every defined anchor is `data-tier="A"` or `"B"`, naming any that
     are `C` or absent
   - **obtainable**: no entry's manifest record is `library-only`, naming any
     that are

   Follow the two guards this programme already wrote on the point that matters:
   an empty input **throws** rather than passing. A zero-length anchor set or an
   unreadable manifest is a broken check, not a green one — the vacuity this
   repo has shipped once before and records in `presets-json-schema.test.mjs`.

   Do **not** read the theory bundle. Its entries are governed by
   `theory-bundle-references.test.mjs`, and a guard that scanned both would pass
   when a shipping entry was cited only from an unpublished page.

2. Run `npm --prefix site test`. All three arms pass immediately, because
   M003/S02 already produced that state — so this proves nothing, and step 3 is
   what earns them.

3. **Prove each arm bites**, one mutation per property, each reverted and the
   file confirmed byte-identical:
   - add an entry to the appendix that no page cites → the **cited** arm fails,
     naming it
   - change one entry's `data-tier` to `C` → the **tier** arm fails, naming it
   - set one manifest record's `obtainability` to `library-only` → the
     **obtainable** arm fails, naming it

   A fourth mutation is worth running because it is the failure mode these
   guards have hit twice in this programme: empty the appendix entirely and
   confirm the check **throws** rather than reporting three vacuous passes.

4. Run `format`, `site-unit`, `doc-conformance`. Append the slice's evidence
   including the final entry count, tick every definition-of-done box — noting
   beside the tier item that the guard allows B by the owner's decision — close
   FR20 **in the same edit**, set the slice `done`, run `jk-standards ledger`,
   and commit with `Slice: M003/S03`, `Rows: FR20`.

   FR23 is already `accepted` and needs no action: it records the declined
   10–20 target so a later pass does not rediscover it as an omission.
