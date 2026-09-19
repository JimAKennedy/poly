---
class: gated
---

# M003/S02 — The sequence reaches a factory preset

**Slice:** `M003/S02` in `docs/plans/guide-parity/ledger.md`
**Depends:** `M003/S01`
**Classification:** bounded — the emitter-and-generator path M003/S01 of the
engine-capability programme established, walked again for a new field.

## Task status

- [ ] 1. A preset carries a sequence, and it reaches `presets.json`
- [ ] 2. The guide names the capability, and close-out

## Definition of Done

Copied verbatim from the slice.

- [ ] A lane's note sequence is expressible in a preset and reaches `site/src/generated/presets.json` under a raised schema version
- [ ] At least one factory preset uses a sequence, and its lanes' pitches are asserted against the generated data
- [ ] The preset count and any per-lane field-count guards are updated rather than bypassed
- [ ] The guide names the capability and says the traditions chapters do not
      yet use it, locked by a `scope-framing` claim *(added at planning — see
      `M003-decisions.md`)*

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `engine-isolation` | `cmake -S . -B build-engine -DCMAKE_BUILD_TYPE=Release -DPOLY_ENGINE_ONLY=ON && cmake --build build-engine --parallel && ctest --test-dir build-engine --output-on-failure` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
## Standing instructions

From `M003-decisions.md`, carried from M001 and M002's findings:

- **Add the render-level case before probing.** M002/S02's probe killed nothing
  because every case tested the helper directly; the render case then found a
  real wiring defect immediately.
- **Name the discriminating case** in the evidence. Most cases survive their
  probe — determinism, range and preset-identity hold for the old behaviour too.
- **Prove a `present`-only site claim by removing the correction**, never by
  adding the old phrasing beside it.
- **Restore a probed file by explicit edit**, not `git checkout`.
- **Rebuild before reading any mutation result**, and check the build succeeded:
  a probe that fails to compile makes `ctest` run the previous binary and report
  everything passing.


## Task 1 — A preset carries a sequence, and it reaches `presets.json`

Modifies `engine/src/presets.cpp`, `engine/tools/emit_presets.cpp`,
`site/scripts/generate-presets-json.mjs`, `site/src/generated/presets.json`,
and `site/tests/presets-json-schema.test.mjs`.

1. Give one factory preset a sequenced lane. Choose a preset where a melodic
   voice is musically defensible and say why in the comment — a sequence on a
   kick lane would demonstrate the field while misrepresenting the preset.
2. Assert in the engine tests that the preset's lane carries the sequence and
   emits its pitches, so the preset is covered by a test rather than by the
   generator alone.
3. Extend `emit_presets.cpp` to emit `noteSequence` and `noteSequenceLength`
   **only for lanes that have one**, following the convention `onsets` and
   `subdivisionProfile` already use: the field's absence says the lane is
   single-pitched.
4. Raise the emitted `schemaVersion` **in the string the emitter writes**, not
   only in its documentation comment. Engine-capability M003/S01 raised the
   comment and the generator's gate caught the mismatch — which is the gate
   working, and worth not repeating.
5. Raise the gate in `generate-presets-json.mjs` and the expectation in
   `presets-json-schema.test.mjs`. That second file is the consumer
   engine-capability M003/S01 missed, because it runs under `site-unit` and that
   slice owed only engine tokens; this slice owes both.
6. Regenerate, confirm the schema version and the preset's sequence in the
   output, and run `format`, `unit`, `engine-isolation`, `site-unit`,
   `doc-conformance`. Commit.

## Task 2 — The guide names the capability, and close-out

Modifies a guide page, `site/tests/scope-framing.test.mjs`, the evidence, the
ledger, and this plan.

**This item was added to the slice at planning**, recorded in
`M003-decisions.md`: the milestone's own text leaves the guide question open,
and the answer taken was to ship ahead while saying so.

1. Choose the page by reading rather than assuming — the appendix describing
   lane parameters is the likely home, and the passage belongs where a reader
   meets lane fields, not in a traditions chapter that does not use them.
2. Write the `scope-framing` claim first. It asserts the new description is
   **present**; there is no falsehood to forbid, so say that in the claim's
   `rule` text as M001/S02's two claims do.
3. The passage must say three things: what a note sequence is, that the onset
   grid still governs timing (so a reader does not expect a sequencer), and that
   the traditions chapters do not yet use it.
4. Prove the claim by **removing** the correction; restore by explicit edit.
5. Run the whole validation set, append the evidence, tick the boxes, set `GP08`
   and the slice `done`, run `jk-standards ledger`, and commit with
   `Slice: M003/S02` and `Rows: GP08`.
