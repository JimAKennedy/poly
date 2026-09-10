# M003/S05 — Remaining framing items

**Slice:** M003/S05, in `docs/plans/theory-audit/ledger.md`

Three unrelated framing items, one per chapter or theory page, closing the last
slice of M003. Two of the three rows were amended during planning because the
audit text behind them is contradicted by the tree; the amendments and their
evidence are recorded in the rows' `Item` cells and summarised under
"Audit corrections" below.

## Task status

- [x] Task 1 — Cite Chapter 5's cyclic-time opening (F35)
- [ ] Task 2 — Give Gamelan Rule 5 its third-part case (F33)
- [ ] Task 3 — Give the Rachenitsa table its Note column and stand-in line (F34)
- [ ] Task 4 — Close the slice, register B11, run the shipping gate

## Definition of Done

- [ ] Gamelan Rule 5 says a third part may double the pokok tones instead of
      interlocking, cited and cross-linked to Rule 6
- [ ] The Rachenitsa patch table carries a `Note` column matching
      `presets.json`, and a line naming the GM sounds and flagging that kaval
      and gadulka have no GM drum equivalent
- [ ] Chapter 5's cyclic-time opening carries a citation

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `node --test site/tests/` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `gate` | `bash scripts/pre-push-check.sh` |

Resolve each through `.jk/validations.yml` rather than trusting this table if
the two ever disagree; the YAML is authoritative.

## Audit corrections

Three claims in `docs/audits/poly_theory_audit.md` are wrong against the tree.
None is a judgement call — each is settled by a file in the repo.

1. **§125 says Rule 5 lists "five styles".** It lists four: *norot*,
   *kotekan telu*, *kotekan empat*, *nyog cag*. Count them in
   `site/src/content/docs/theory-gamelan.mdx`.
2. **§125 asserts *kotekan polos*, "a third player playing only the structural
   pokok tones", with no source.** Nothing in the repo attests the term, and
   the guide's own Rule 3 already uses *polos* for one of the interlocking
   pair, so adopting it as a style name would collide with the page's
   established usage. Task 2 writes the substance and omits the label; the
   sourcing question becomes row B11 in M006, which owns the bibliography.
3. **§176 says the "Note" column should mark tupan and kaval as GM stand-ins.**
   The table has no `Note` column, and the premise is backwards: the tupan is a
   double-headed bass drum, so its two lanes map to GM kick and side stick —
   a drum standing in for a drum — which the table's own Role column already
   says by naming them "Tupan bass" and "Tupan rim". The genuine stand-ins are
   lane 3, a **woodblock for the kaval**, an end-blown flute, and lane 4, a
   **hi-hat for the gadulka**, a bowed fiddle. The audit names neither and does
   not mention the gadulka lane at all.

The `Item` cells for F33 and F34 carry these corrections, following the B10
precedent from M003/S04.

## Preset facts

Every value below comes from the `Rachenitsa 7/8` record in
`site/public/webui/presets.json`, and is the oracle Task 3's lock asserts
against. Read them from the file rather than copying them from here.

| Lane | Table role | `noteNumber` | `roleLabel` |
|---|---|---|---|
| 1 | Tupan bass | 36 | kick |
| 2 | Tupan rim | 37 | rim |
| 3 | Kaval accent | 76 | woodblock |
| 4 | Gadulka pulse | 42 | hat |

`site/src/content/docs/17-midi-routing-note-map.mdx` corroborates 36 (Kick 1),
37 (Side Stick) and 42 (Closed HH) in its GM Name table. Note 76 is not in that
page's representative subset, so `roleLabel` is its only attestation in the
repo — which is why the lock compares against `roleLabel` for all four lanes
rather than against a hand-written GM name list.

---

## Task 1 — Cite Chapter 5's cyclic-time opening (F35)

Closes **F35**. Produces the first `S05-` case in the M003 lock host, which
Tasks 2 and 3 extend.

**Files:** `site/src/content/docs/05-gamelan.mdx`,
`site/tests/scope-framing.test.mjs`

The opening sentence of `05-gamelan.mdx` reads "In the gamelan traditions of
Bali and Java, time is not a line — it is a circle." The audit's own
recommendation is to ground it in Tenzer (2000)'s discussion of cyclic
structure. `fr-tenzer-2000` already exists in the appendix at tier A and is
already cited twice from `theory-gamelan.mdx`.

Cite Tenzer, not Geertz. The audit attributes the framing's origin to Geertz's
*Negara* (1980), but that work is not in the appendix, and adding a bibliography
entry is M006's job, not this slice's.

Chapter pages may cite `fr-` anchors: seven already do, including
`07-balkan.mdx`. No new numbered entry is needed and no renumbering is
involved.

1. **Write the failing case.** Add to the `CLAIMS` array in
   `site/tests/scope-framing.test.mjs`:

   ```js
   {
     id: 'S05-F35',
     file: '05-gamelan.mdx',
     rule: "Chapter 5's cyclic-time opening carries a citation",
     presentRegex: [
       /time is not a line[\s\S]{0,400}#fr-tenzer-2000/,
     ],
   },
   ```

   The bounded `[\s\S]{0,400}` span is what ties the citation to the opening
   rather than to any later paragraph. Confirm the helper's field names against
   the neighbouring claims before writing — `site/tests/helpers/prose-claims.mjs`
   is the contract, and `normalizeProse` lowercases and matches substrings.

2. **Run it and watch it fail.** `node --test site/tests/scope-framing.test.mjs`
   must fail naming `S05-F35`. It must fail *before* the prose changes; if it
   passes here, the regex is matching something else and the case is worthless.

3. **Add the citation** to the opening sentence of `05-gamelan.mdx`, in the form
   the page's other citations use — an inline link to
   `/appendix-references/#fr-tenzer-2000`. Match the surrounding sentence's
   voice; do not restructure the paragraph.

4. **Run it and watch it pass.**

5. **Check.** `node --test site/tests/` (`site-unit`) and
   `bash scripts/check-doc-conformance.sh` (`doc-conformance`) both green, then
   `pre-commit run --all-files` (`format`).

6. **Commit** with `Rows: F35`, ticking Task 1 above and setting F35 `done` in
   the ledger with `S05-F35` named in its `Verification`. Append the gate
   results to `docs/plans/theory-audit/evidence/M003-S05.md`. Do not name a
   commit SHA in the evidence file.

---

## Task 2 — Give Gamelan Rule 5 its third-part case (F33)

Closes **F33**. Consumes the `CLAIMS` array Task 1 extended.

**Files:** `site/src/content/docs/theory-gamelan.mdx`,
`site/tests/scope-framing.test.mjs`

Rule 5 currently names four interlock styles and argues that choosing one and
staying with it is the reliable default. It says nothing about a third part.
The gap the audit identified is real and matters to a Poly user adding a third
melodic lane — it is only the label that is unsourced.

Write the substance: a third part may double the pokok tones rather than
interlock with the pair. Cite `fr-tenzer-2000`, whose appendix annotation is
"The authoritative analysis of kotekan varieties", and cross-link Rule 6, which
is the rule that establishes pokok as what the interlock elaborates. Do **not**
write *kotekan polos*.

Cross-link Rule 6 on the same page, not Chapter 5's patch. Chapter 5's patch has
no pokok layer today — that absence is F41, which M004/S03 fixes and which is
still `open`. A link claiming a pokok lane there would be false at the moment it
was written.

1. **Write the failing case.** Add to `CLAIMS`:

   ```js
   {
     id: 'S05-F33',
     file: 'theory-gamelan.mdx',
     rule: 'Rule 5 offers a third part doubling the pokok, cited and linked to Rule 6',
     present: ['pokok'],
     presentRegex: [
       /third (?:part|lane)[\s\S]{0,300}pokok/,
       /third (?:part|lane)[\s\S]{0,300}#fr-tenzer-2000/,
     ],
     forbidden: ['kotekan polos'],
   },
   ```

   The `forbidden` arm is the one that keeps the decision from eroding: it
   fails if a later edit reintroduces the unattested label. Note that `pokok`
   already appears in Rule 6, so the bare `present: ['pokok']` arm is
   pre-satisfied and cannot fail first — the two `presentRegex` arms are what
   must drive the red. Check this by running step 2 and reading which assertion
   fails.

2. **Run it and watch it fail**, for the right reason: one of the
   `presentRegex` arms, not the `present` arm.

3. **Extend Rule 5** in `theory-gamelan.mdx` with a sentence in the rule's
   existing voice, naming the practice descriptively, citing
   `[Tenzer 2000](/appendix-references/#fr-tenzer-2000)`, and linking Rule 6 by
   the anchor convention the page already uses for internal rule references.
   Check how Rule 4's parenthetical refers to Poly's own features and match that
   register. Keep it to one or two sentences; this is an `enrich`, not a rewrite.

4. **Run it and watch it pass.**

5. **Prove the `forbidden` arm bites.** Temporarily insert the string
   `kotekan polos` into the new sentence, run the case, watch it fail naming the
   forbidden phrase, then remove it **by an inverse edit** — never
   `git checkout --`, which has destroyed uncommitted work in this programme
   before. Confirm with `git diff --stat` that the file is byte-identical to
   the post-step-3 state.

6. **Check.** `site-unit`, `doc-conformance`, `format` — all green.

7. **Commit** with `Rows: F33`, ticking Task 2, setting F33 `done` with
   `S05-F33` in its `Verification`, and appending to the evidence file.

---

## Task 3 — Give the Rachenitsa table its Note column and stand-in line (F34)

Closes **F34**. Consumes the `CLAIMS` array; produces the presets-backed lock
that Task 4's gate run covers.

**Files:** `site/src/content/docs/07-balkan.mdx`,
`site/tests/scope-framing.test.mjs`

The Rachenitsa patch table has columns Lane, Role, Steps, Hits, Rotation,
Subdivision, Velocity, Swing — and no note information at all, so a reader
cannot tell what any lane will sound like on a GM kit.

Add a `Note` column carrying the bare note number, and put the GM sound names
in a line beneath the table rather than inside the column. The column header
and its bare-number contents deliberately match what `PresetTable` emits for
`Note` (`String(lane.noteNumber)`, per `site/tests/preset-table-conformance.test.mjs`),
so that if `07-balkan.mdx` is ever migrated from the hand-written `PolyPatch`
table to the generated `PresetTable`, the column is a drop-in rather than a
conflict. `PresetTable` is currently used by `appendix-presets.mdx` alone;
migrating chapters is not this slice's work.

1. **Write the failing case.** This one needs the preset as its oracle, so it
   does not fit the `CLAIMS` array's prose-only shape. Add a separate `test(...)`
   to `site/tests/scope-framing.test.mjs`, below the `registerClaimTests` call,
   named `S05-F34`. It must:

   - read `site/public/webui/presets.json` and find the `Rachenitsa 7/8` record
   - parse the `| Lane |` table under `<PolyPatch title="Rachenitsa Groove"` in
     `07-balkan.mdx`
   - assert the header row contains `Note`
   - assert each of the four lanes' `Note` cell equals `String(noteNumber)` for
     the corresponding lane in the preset
   - assert the prose beneath the table names each lane's `roleLabel`, and that
     it names both `kaval` and `gadulka` as having no GM drum equivalent

   Assert against the preset's values, never against numbers copied from this
   plan. The "Preset facts" table above is for orientation; a lock that hard-codes
   36/37/76/42 stops being a conformance check the moment the preset changes.

2. **Run it and watch it fail** — it must fail on the missing `Note` header,
   since no part of this exists yet.

3. **Add the `Note` column** to the Rachenitsa table, one bare note number per
   lane, read from `presets.json`.

4. **Add the stand-in line** immediately beneath the table. It must say that the
   tupan lanes map to GM kick and side stick, which is a drum standing in for a
   drum; that the kaval is an end-blown flute and the gadulka a bowed fiddle,
   neither of which a GM drum kit can produce; and that their lanes are
   rhythmic placeholders — a woodblock and a hi-hat — not the instruments named
   in the Role column. Write it in the page's existing explanatory voice, like
   the "Notice that Swing is set to 0 on every lane" paragraph that follows the
   table.

5. **Run it and watch it pass.**

6. **Prove the oracle bites.** Change one `Note` cell to a wrong number, run the
   case, watch it fail naming that lane and the divergence, then restore it by
   an inverse edit and confirm with `git diff --stat`.

7. **Check.** `site-unit`, `doc-conformance`, `format`. Run
   `node --test site/tests/preset-table-conformance.test.mjs` explicitly too and
   confirm it is unaffected — it guards `appendix-presets.mdx`, not this page,
   and this task must not perturb it.

8. **Commit** with `Rows: F34`, ticking Task 3, setting F34 `done` with
   `S05-F34` in its `Verification`, and appending to the evidence file.

---

## Task 4 — Close the slice, register B11, run the shipping gate

Closes no findings row; closes the slice. Consumes everything above.

**Files:** `docs/plans/theory-audit/ledger.md`,
`docs/plans/theory-audit/evidence/M003-S05.md`, this plan

1. **Register B11 in M006.** Add a slice `M006/S04 — Unattested terms`, with
   validation `format, site-unit, doc-conformance`, evidence
   `evidence/M006-S04.md`, status `open`, and one row:

   - **B11**, `P2`, disposition `source`, lands in
     `site/src/content/docs/theory-gamelan.mdx` Rule 5, status `open`. Its
     `Item` records that the audit asserted *kotekan polos* as a named interlock
     style with no source, that M003/S05 wrote the practice without the label,
     and that M006 owns finding an attestation or recording the term as
     unverifiable.

   Its definition of done: the term is either cited to a tier-A or tier-B source
   and named in Rule 5, or recorded in the ledger as unverifiable and left out.

   B11 stays `open` — it is work this slice deliberately does not do. Do not
   mark it `accepted` to make the ledger tidy; that would claim a decision was
   final when it has been deferred.

2. **Verify the row parser sees B11.** The ledger guard's row regex is
   `/^[FHB]\d{2}$/` — a row it rejects sets `table = null` and silently skips
   every later row in that table, which is how B01–B09 went unvalidated once
   before. Run `node --test docs/audits/theory-audit-remediation.test.mjs` and
   confirm the count of validated rows rises by one. If it does not, stop: the
   row is malformed and the guard is hiding it.

3. **Tick the definition of done** in the slice and in this plan's copy, and
   tick Task 4 above. Confirm F33, F34 and F35 are all `done`, then set the
   slice `Status` to `done`.

4. **Run every token the slice declares**, in this order:
   `pre-commit run --all-files`, `node --test site/tests/`,
   `bash scripts/check-doc-conformance.sh`, then `bash scripts/pre-push-check.sh`
   for `gate`. `gate` is the full pre-push suite including the native build and
   `ctest`; it is slow, and it is the token this slice owes because S05 is the
   last slice before M003 ships. Read its exit code, not its narrative.

5. **Run `jk-standards ledger`.** With the slice `done` it enforces the stricter
   claim: every DoD box ticked, the evidence file present, every row closed.

6. **Append the final evidence entry**, naming each token and what it returned,
   and stating that no commit SHA is recorded because this file ships inside the
   commit it describes.

7. **Commit** with `Rows:` naming no findings row — use `Rows: —` if the trailer
   requires a value, matching whatever the programme's existing slice-closing
   commits do; check `git log --grep="Slice: M003/S04"` for the form.

---

## Self-review

**DoD coverage.** Item 1 (Rule 5 third part) → Task 2. Item 2 (Note column and
stand-in line) → Task 3. Item 3 (Chapter 5 citation) → Task 1. Task 4 satisfies
none directly; it closes the slice and runs `gate`.

**Row coverage.** F35 → Task 1, verification produced at step 1 (`S05-F35`).
F33 → Task 2, verification at step 1 (`S05-F33`), with step 5 proving the
`forbidden` arm bites. F34 → Task 3, verification at step 1 (`S05-F34`), with
step 6 proving the preset oracle bites. B11 is created by Task 4 and closed by
M006, not here.

**Placeholder scan.** No TBDs, no "similar to task N", no reference to a helper
no task defines. `registerClaimTests` and `normalizeProse` are existing exports
of `site/tests/helpers/prose-claims.mjs`; `expectedCell` and `buildPresetTable`
are named only as context for the `Note` column's format and are not called by
any task here.

**Name consistency.** The three claim ids are `S05-F33`, `S05-F34`, `S05-F35`
throughout, matching the `S04-` convention S01–S04 established. The new M006
slice is `M006/S04` and its row is `B11` in every mention.

**Ordering.** Task 1 goes first because it is the smallest and it establishes
the `S05-` case convention the other two follow. Tasks 2 and 3 are independent
of each other and could swap. Task 4 must be last: it runs `gate` over the
finished tree.
