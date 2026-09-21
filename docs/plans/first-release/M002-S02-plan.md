# M002/S02 — The pages move, and the guards follow

**Slice:** M002/S02 in `docs/plans/first-release/ledger.md`
**Rows:** FR10 (the pages are published routes), FR11 (test files assert them by
path), FR12 (the conformance runner), FR13 (the drift map), FR14 (the deep
dives' own bibliography), FR15 (the `jk-standards.yaml` comment)
**Depends:** M002/S01 — nothing may link to a page that has moved.
**Classification:** bounded. A directory move plus path updates in files that
already exist, and one new bibliography assembled from an existing one.

## Task status

- [ ] 1. The pages move, with their imports intact
- [ ] 2. The ten test files follow them
- [ ] 3. The drift map follows them
- [ ] 4. The deep dives carry their own bibliography
- [ ] 5. `jk-standards.yaml` says why this works, and the slice closes

## Definition of Done

Copied verbatim from the slice:

- [ ] No `theory-*` page is a published route
- [ ] Every guard that asserted a deep dive's content still runs, against the
      new path
- [ ] The moved bundle carries the references only it cites, so republishing
      needs no citation repair
- [ ] `jk-standards.yaml` records why a sibling directory falls outside its
      roots, so a later widening is deliberate

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `guards` | `bash scripts/check-guards.sh` |

## Why `site/src/content/theory/`

Every deep dive opens with `import PolyPatch from '../../components/PolyPatch.astro'`.
That path resolves to `site/src/components` from `site/src/content/theory/` —
byte-identical to today — and to a nonexistent root-level `components/` from
`docs/theory/`. A sibling under `site/src/content/` keeps the imports unchanged,
which is what makes "deferred" cheap.

`site/src/content.config.ts` defines exactly one collection, `docs`, via
Starlight's `docsLoader()`. A sibling directory is not a collection, so Astro
builds nothing from it, and republishing later is a config change rather than an
edit to twelve files.

## Corrections this slice carries

Measured before planning; recorded here because two rows claim otherwise.

- **FR11 says twelve test files.** **Ten** reference a deep-dive page.
  `doc-conformance-wiring.test.mjs` and `chapter-euclidean-guardrail.test.mjs`
  matched a `theory-` grep because they name theory *test filenames* or mention
  the theory-audit programme in prose. Neither is affected.
- **FR12 says the conformance runner stops finding the tests after a move.** It
  names `site/tests/theory-*.test.mjs` — **test files, which are not moving**.
  Only the pages move. The row is a non-issue and closes `accepted`.

---

## Task 1 — The pages move, with their imports intact

**Consumes:** M002/S01's work — nothing links in any more. **Produces:** an
unbuilt bundle at the new path.

1. `git mv site/src/content/docs/theory-*.mdx site/src/content/theory/`, twelve
   files. Use `git mv` so the history follows them.
2. Confirm the imports are untouched: every moved file must still read
   `from '../../components/PolyPatch.astro'`, and that path must still resolve
   to `site/src/components/PolyPatch.astro`.
3. Build the site. The build must succeed and must emit **no** `theory-*` route.
   Check the output directory for theory paths rather than trusting the absence
   of an error — a collection that silently picks the directory up would build
   them without complaint.
4. Run `format`, `site-unit`, `doc-conformance`. Expect `site-unit` to fail:
   ten test files still point at the old path, and task 2 is what fixes them.
   **This is the one task in the milestone that ends red by design**, so the
   commit records that explicitly rather than claiming a green gate.
5. Commit with `Slice: M002/S02`, `Rows: FR10`.

## Task 2 — The ten test files follow them

**Consumes:** task 1's moved pages. **Produces:** a green suite.

1. Find every test file that reads a deep dive from disk, by running the suite
   and reading the failures — not by grepping. M001 established that a selector
   grep is a starting point and the suite is the census, and it established it
   the expensive way.
2. Update each path constant to `site/src/content/theory/`. Do not change what
   any test asserts: this is a path move, and an assertion that changes here is
   a scope breach hiding inside a refactor.
3. Run `npm --prefix site test` until green, then run it once more and read the
   count. The suite must have the **same number of tests** as before the move —
   a lower count means a file stopped being discovered rather than being fixed.
4. Run `format`, `site-unit`, `doc-conformance`. Commit with
   `Slice: M002/S02`, `Rows: FR11, FR12` — FR12 closes `accepted` here, with
   the reason recorded in the ledger row itself.

## Task 3 — The drift map follows them

**Consumes:** the moved pages. **Produces:** a green `doc-drift`.

`.github/docs-drift-map.yml` maps twelve theory docs by full path under
`site/src/content/docs/`.

1. Update the twelve `doc:` paths to the new directory.
2. Run `bash scripts/check-doc-discipline.sh`, which is the wrapper that
   actually runs `doc-drift` — the bare `jk-standards all` skips it when no base
   ref is available and exits 0, which is how a violation shipped once before.
3. **Prove the mapping still bites:** touch one of the sources the drift map
   pairs with a theory doc, confirm `doc-drift` names that doc, revert.
4. Run `format`, `doc-discipline`, `guards`. Commit with `Slice: M002/S02`,
   `Rows: FR13`.

## Task 4 — The deep dives carry their own bibliography

**Consumes:** the moved pages. **Produces:** the thing that makes republication
free of citation repair.

The deep dives cite **93** anchors: 66 exclusively theirs, 27 shared with
shipping pages. All 93 go into the moved bundle, so M003 can cut the shipping
appendix to 34 without touching anything the deep dives depend on.

1. Create `site/src/content/theory/theory-references.mdx` carrying those 93
   entries, copied from `site/src/content/docs/appendix-references.mdx` with
   their tiers and links intact. Derive the set by reading the moved pages, not
   by hand: an anchor missed here is a dangling citation on republication, and
   nothing will warn about it in the meantime.
2. Add a test that every anchor the moved pages cite resolves inside the moved
   bundle. This is the guard that makes FR14 checkable rather than asserted, and
   it is what would catch an anchor missed in step 1.
3. Run it, watch it pass, then **prove it bites**: delete one entry from
   `theory-references.mdx`, confirm the test names that anchor, restore.
4. Run `format`, `site-unit`, `doc-conformance`. Commit with
   `Slice: M002/S02`, `Rows: FR14`.

## Task 5 — `jk-standards.yaml` says why this works, and the slice closes

**Consumes:** everything above.

The move works because `doc_roots` and `research_provenance.doc_roots` are both
scoped to `site/src/content/docs` with `.mdx`, so a sibling directory falls
outside both. That is the behaviour this milestone wants, but it follows from a
path list rather than from anything anyone wrote down — and a later widening of
either root would drag the deep dives back into scope silently.

1. Add a comment beside both roots in `jk-standards.yaml` stating that
   `site/src/content/theory/` is deliberately outside them, and why: the pages
   are deferred, not deleted, and their citations resolve against their own
   bibliography rather than the shipping one.
2. Run `format`, `site-unit`, `doc-conformance`, `doc-discipline`, `guards`.
3. Append the closing evidence, tick every definition-of-done box, set FR10,
   FR11, FR13, FR14 and FR15 `done` and FR12 `accepted`, set the slice `done`,
   run `jk-standards ledger`, and commit with `Slice: M002/S02`, `Rows: FR15`.
