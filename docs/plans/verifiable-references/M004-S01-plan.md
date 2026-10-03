# M004/S01 — The Wikipedia policy is decided and applied

**Slice:** M004/S01 in `docs/plans/verifiable-references/ledger.md`
**Rows:** VR11 (three Wikipedia entries cited from the theory deep dives)
**Classification:** bounded. One policy paragraph, three entries retired from
the theory bundle and their See-also lines, three manifest records removed,
and a claim test in the existing `citation-tier.test.mjs` shape. Decisions are
in `M004-decisions.md`: the policy is **never cited**, stated once in the
shipping appendix's introduction, and leaving entries are **retired**.

## Task status

- [ ] 1. The policy is written once
- [ ] 2. The three Wikipedia entries are retired

## Definition of Done

Copied verbatim from the slice:

- [ ] The policy is written down: whether Wikipedia may be cited at all, and if
      so for what — with its reason, once, rather than per entry
- [ ] The three Wikipedia entries conform to it
- [ ] Any claim that loses its citation is rewritten to need none, not left
      uncited

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

## Task 1 — The policy is written once

**Files:** modify `site/src/content/docs/appendix-references.mdx`,
`site/src/content/theory/theory-references.mdx`,
`site/tests/citation-tier.test.mjs`.
**Produces:** the `What this bibliography cites` paragraph S02 extends, and
the `POLICY-WIKIPEDIA` claim.

1. Add a claim to `CLAIMS` in `citation-tier.test.mjs`:
   `id: 'POLICY-WIKIPEDIA'`, `file: 'appendix-references.mdx'`, a `rule`
   naming VR11 and the owner's decision, and
   `present: ['The guide does not cite Wikipedia']`.
2. Run `node --test site/tests/citation-tier.test.mjs`; it fails — the
   sentence is absent.
3. In `appendix-references.mdx`, after the "Nothing in this guide is original
   research" paragraph, add a paragraph opening **What this bibliography
   cites.** stating that the guide does not cite Wikipedia, with one sentence
   of reason: it is reviewable and often a fair summary, but it summarises
   other sources, and a claim in this guide names the source itself.
4. In `theory-references.mdx`'s introduction, add one sentence: these entries
   are held to the policy stated in the shipping appendix's introduction.
5. Run the claim file; it passes. Run `site-unit`, `format`,
   `doc-conformance`; all pass.
6. Commit with `Rows:` empty.

## Task 2 — The three Wikipedia entries are retired

**Files:** modify `site/src/content/theory/theory-references.mdx`,
`theory-sub-saharan-africa.mdx`, `theory-minimalism.mdx`,
`theory-brazilian.mdx`, `site/src/data/references.json`,
`site/src/data/references-bibliography.mjs`,
`site/tests/references-archive-naming.test.mjs`,
`site/tests/citation-tier.test.mjs`; the ledger; the evidence file.

1. Add a standalone test to `citation-tier.test.mjs`, **tree-wide**: read
   every `.mdx` under `site/src/content/docs` and `site/src/content/theory`,
   and assert none contains `wikipedia.org`, naming each offending file and
   throwing if no `.mdx` file was read. Run it: red on
   `theory-references.mdx`.
2. In `references-archive-naming.test.mjs`, replace the `ref-43`
   parenthesised-URL case with one against a fixed string via an exported
   `parseLine`: the line
   `` <span id="ref-99" data-tier="A">**[99]**</span> "X." [Link](https://en.example.org/wiki/A_(b)) ``
   parses to `url === 'https://en.example.org/wiki/A_(b)'`. Run it: it fails,
   `parseLine` is not exported. Export it from `references-bibliography.mjs`;
   it passes.
3. Retire `ref-4`, `ref-32`, `ref-43`: delete each line from
   `theory-references.mdx`; delete its link from the See-also line that cites
   it (`theory-sub-saharan-africa.mdx`, `theory-minimalism.mdx`,
   `theory-brazilian.mdx`), keeping the line's punctuation grammatical; delete
   its record from `references.json`, first copying the record verbatim into
   the evidence.
4. Update the theory bundle introduction's entry count to the number of
   anchors the file now defines, measured with
   `grep -c 'id="\(ref\|fr\)-' site/src/content/theory/theory-references.mdx`.
5. Confirm no claim lost its citation: all three were See-also links. Record
   the three lines before and after in the evidence — DoD item 3's proof.
6. Run the claim file (the tree-wide test now passes), `site-unit`, `format`,
   `doc-conformance`; all pass, including `theory-bundle-references.test.mjs`
   in both directions and the manifest completeness test.
7. Append the evidence; tick the DoD; set VR11 and the slice `done`; run
   `jk-standards ledger`.
8. Commit with `Rows: VR11`.

## Self-review

- **DoD 1** (policy written once with its reason) — task 1 step 3, locked by
  `POLICY-WIKIPEDIA`.
- **DoD 2** (the three entries conform) — task 2 step 3, locked by the
  tree-wide test from step 1.
- **DoD 3** (no claim left uncited) — task 2 step 5: the three were See-also
  links, recorded before and after.
- **VR11** — "a claim test pins the outcome": both tests.
- **Names** — `POLICY-WIKIPEDIA`, `parseLine`, `What this bibliography cites`
  match S02's plan.
- **Placeholders** — none.
