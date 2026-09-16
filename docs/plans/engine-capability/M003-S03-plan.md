---
class: gated
---

# M003/S03 — The measured jembe profile

**Slice:** `M003/S03` in `docs/plans/engine-capability/ledger.md`
**Depends:** `M003/S01`

## Task status

- [ ] 1. The jembe profile ships as cited data
- [ ] 2. The page stops disclaiming it, and close-out

## Definition of Done

Copied verbatim from the slice.

- [ ] A measured jembe profile ships as data, with its source cited in the guide's
      bibliography at a tier the citation check accepts
- [ ] `theory-sub-saharan-africa` construction step 5 no longer says Poly ships no
      measured profile
- [ ] The profile's values are reachable from a preset

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

## Task 1 — The jembe profile ships as cited data

Modifies `engine/src/presets.cpp` and the engine tests.

1. Add a jembe profile to the catalogue S01 task 4 created, citing
   `fr-polak-2010` — already in `site/src/content/docs/appendix-references.mdx`
   at `data-tier="A"`, described there as "measured microtiming showing jembe
   subdivision is systematically uneven", so no new bibliography entry is owed.
2. **Do not invent measured numbers.** Polak's finding is that jembe subdivision
   is systematically uneven with a stable short-long shape; encode that shape,
   and state in the comment that the ratios are Poly's rendering of a published
   *finding* rather than a transcription of a published table. If the exact
   published ratios are wanted later, that is a data refinement against a source
   in hand, and the comment should say so. This is the same honesty the
   `theory-brazilian` attribution line already applies to its patch values.
3. Apply the profile to the Sub-Saharan preset's jembe lane, and assert in the
   engine tests that its onsets are uneven, tempo-relative, and deterministic.
4. Run `format` and `unit`. Commit.

## Task 2 — The page stops disclaiming it, and close-out

Modifies `site/src/content/docs/theory-sub-saharan-africa.mdx`,
`site/tests/`, the evidence file, the ledger, and this plan.

1. Write the failing site case first, in the file that already holds this page's
   prose guards: construction step 5 must not say Poly ships no measured
   profile, and must cite `fr-polak-2010` in a link the research-provenance
   check resolves.
2. Rewrite construction step 5. Three things change and the decision file
   records why: the disclaimer goes; the profile is named as what it is; and the
   sentence pointing readers at per-step micro-timing as the place a profile
   goes is corrected, because micro-timing is absolute milliseconds and the
   paragraph itself calls the profile "particular ratios, stable across tempi".
   Keep the paragraph's existing and correct distinction between a systematic
   profile and random jitter — this adds a third layer to it rather than
   replacing it.
3. Run `site-unit` and `doc-conformance`, and watch the new case pass. Mutate the
   page to restore the disclaimer and watch it fail, then restore.
4. Run the whole validation set, append the evidence, tick the boxes, set `EC10`
   and the slice `done`, run `jk-standards ledger`, commit with
   `Slice: M003/S03` and `Rows: EC10`.
