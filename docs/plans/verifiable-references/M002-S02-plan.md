# M002/S02 — All 107 carry both verdicts

**Slice:** M002/S02 in `docs/plans/verifiable-references/ledger.md`
**Rows:** VR06 (no recorded availability), VR07 (a citation can name a work that
does not exist), VR08 (64 entries with no identifier at all), VR17 (`ref-23`
suspected of naming the wrong title and the wrong journal)
**Depends:** M002/S01 — done. The manifest, its shape test and both
completeness arms exist; this slice fills the records those tests already guard.
**Classification:** bounded. No new mechanism: every task edits one
hand-authored JSON that S01 declared and that four tests already constrain.
Bounded does not mean small — this is the milestone's expensive half.

## Task status

- [x] 1. Numbered refs, Chapters 1–3 (11 entries)
- [x] 2. Numbered refs, Chapters 4–7 (18 entries)
- [ ] 3. Numbered refs, Chapters 8–15 (14 entries)
- [ ] 4. Further Reading — Foundations, Sub-Saharan Africa (15 entries)
- [ ] 5. Further Reading — Afro-Cuban through Balkan (21 entries)
- [ ] 6. Further Reading — Minimalism through Synthesis (17 entries)
- [ ] 7. Further Reading — Funk/Soul/Jazz, Microtiming (11 entries)
- [ ] 8. The browser worklist, and the slice closes

## Definition of Done

Copied verbatim from the slice:

- [ ] Every one of the 107 entries has an obtainability verdict with its
      evidence, and a price where the verdict is "purchasable"
- [ ] Every entry has a description verdict — verified, mismatch, or
      **unverified** — reached against the source, not against the guide's entry
- [ ] Every mismatch is recorded with what the source actually is, so M004 can
      act on it without repeating the work

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |

## The per-entry procedure

Identical for all 107. The owner chose the deepest option available — **open
every source that can be opened** — so metadata agreement is never sufficient on
its own where the text is reachable.

1. **Read what the guide claims**: author, year, title, venue, from the entry.
2. **Resolve an identifier.** Query Crossref by title and author for a DOI; for
   books, Open Library or archive.org for an ISBN. Record it in `identifier`,
   which is VR08's deliverable for the 64 entries that carry none.
3. **Try to reach the text**, stopping at the first success: the DOI, an
   open-access PDF, an archive.org scan, a publisher landing page.
4. **If the text opens**, read enough to establish that the work is about what
   the guide says it is about — not merely that a work by that name exists.
   Record `verified`, or `mismatch` with a `note` saying what the source
   actually is. This is the step that catches the `ref-9` class, where every
   metadata field matched a real dissertation and only the text showed the
   subject was wrong.
5. **If the text cannot be reached**, record the obtainability verdict that is
   true — `browser-only`, `purchasable` with a price, `library-only`,
   `unobtainable` — and `description: "unverified"` with the reason in
   `evidence`. Add a row to the browser worklist in task 8.
6. **Record `evidence` and `checked`** on every entry. Evidence names what was
   consulted, so a later reader can tell a checked entry from a guessed one.

**`unverified` is a recorded verdict, not a failure.** Recording it honestly is
the whole of VR07: the defect this programme exists to end is silence that reads
as diligence.

**Do not edit the bibliography in this slice.** Mismatches are recorded, not
corrected — M004 acts on them. VR17 is the worked example: `ref-23`'s record
gets the real title and journal in its `note`, and `appendix-references.mdx` is
left alone.

## Tasks 1–7 — the passes

Each task covers the sections named in its checklist row, follows the procedure
above for every entry in them, and ends the same way:

1. Update those records in `site/src/data/references.json`.
2. Run `npm --prefix site test`. The shape test enforces the conditional rules —
   a `purchasable` verdict without a price, a `mismatch` without a note, or an
   assessed entry without a check date all fail here, so the gate catches a
   half-recorded verdict rather than a reviewer having to.
3. Run `format`, `site-unit`.
4. Append to `docs/plans/verifiable-references/evidence/M002-S02.md`: the
   section covered, the count, the verdict distribution, and **every entry that
   came out `unverified`, with its reason**. A pass that does not say what it
   failed to reach is indistinguishable from one that reached everything.
5. Commit with `Slice: M002/S02` and the rows that task advanced.

The section boundaries, which sum to 107:

| Task | Sections | Entries |
|---|---|---|
| 1 | Chapters 1, 2, 3 | 11 |
| 2 | Chapters 4, 5, 6, 7 | 18 |
| 3 | Chapters 8, 9, 10, 14, 15, and Chapter 1 (additional) | 14 |
| 4 | Further Reading: Foundations; Sub-Saharan Africa | 15 |
| 5 | Further Reading: Afro-Cuban; Afrobeat; Gamelan; Indian Classical; Balkan | 21 |
| 6 | Further Reading: Minimalism; Electronic and Drum & Bass; Brazilian; Synthesis | 17 |
| 7 | Further Reading: Funk, Soul and Jazz; Microtiming and Groove Science | 11 |

**VR17 is settled in task 2**, which covers Chapter 6 where `ref-23` sits. Its
URL is on `iftawm.org`, which resets every automated connection, so unless that
changes the entry is `browser-only` / `unverified` and the suspected title and
journal go into `note` as the suspicion they are — evidence from two search
results and the journal's own landing-page URL, not from the source.

## Task 8 — The browser worklist, and the slice closes

**Consumes:** the `unverified` entries every earlier task recorded.

1. Add one `## Pending` row per unreachable entry to
   `docs/plans/verifiable-references/browser-worklist.md`, in the format the
   file already uses: anchor, URL, what to check, what to save. Group them by
   host, because hosts fail as a class — one `iftawm.org` session settles every
   entry on that host.
2. Add a test case to `site/tests/references-manifest.test.mjs`: every entry
   whose `description` is `unverified` **and** whose `obtainability` is
   `browser-only` is named in the worklist. This is the generalised form of the
   guard M001/S01 left dominated, and adding it here is what makes that guard's
   retirement a cleanup rather than a loss — record the connection in the
   decisions file, but leave the M001 guard itself alone: retiring it is M003's
   deferred decision, not this slice's.
3. Prove it bites: remove one row from the worklist, confirm the case fails
   naming that anchor, restore.
4. Run `format`, `site-unit`. Append the closing evidence: the final verdict
   distribution across all 107, and the worklist's length.
5. Tick every definition-of-done box, set VR06, VR07, VR08 and VR17 `done`, set
   the slice `done`, run `jk-standards ledger`, and commit with
   `Slice: M002/S02`, `Rows: VR06, VR07, VR08, VR17`.
