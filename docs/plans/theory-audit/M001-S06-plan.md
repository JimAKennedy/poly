# M001/S06 — Attribution care and remaining hedges

**Slice:** M001/S06
**Ledger:** `docs/plans/theory-audit/ledger.md`

Seven claims across five chapters state contested or unsourced things in the
guide's own voice. None is a factual error — that was S02–S05 — so nothing here
changes what the guide asserts about rhythm. What changes is *whose* claim each
one is: an attribution, a hedge, or a citation, so a reader can tell the guide's
distillation from its sources.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; each task's own commit ticks its box.

- [ ] Task 1 — Chapter 4: Fela/Allen co-attribution and Allen's timing (F09, F10)
- [ ] Task 2 — Chapter 8: attribute Reich's framing to Reich (F11)
- [ ] Task 3 — Chapter 9: attribute the swing claim to Linn (F13)
- [ ] Task 4 — Chapter 13: hedge the Amen-break superlative (F14)
- [ ] Task 5 — Chapter 12: cite Monson for Roach's independence (F15)
- [ ] Task 6 — Mark the lcm translation at all three sites (F16)

## Definition of Done

Copied verbatim from the slice. Every task below argues against *this* text.

- [ ] Ch 4 credits Fela and Allen jointly rather than assigning the rhythmic
      vocabulary to one of them
- [ ] Ch 4's Allen timing claim reads as characterisation, not measurement
- [ ] Ch 8 attributes the "known for centuries" framing to Reich rather than
      asserting it in the guide's own voice
- [ ] Ch 9's techno-vs-house swing claim is attributed to Linn and cross-refers
      to Butler's contrary reading
- [ ] Ch 13's Amen-break superlative is hedged
- [ ] Ch 12's Roach independence claim carries a scholarly citation
- [ ] The lcm-as-sam / lcm-as-gong translation is marked as the guide's own
      informal translation at all three sites
- [ ] Each of the seven is locked by a named case in `theory-audit-claims.test.mjs`

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

`site-unit` is what runs `site/tests/theory-audit-claims.test.mjs`, so it is the
token that proves each lock. Run `node --test site/tests/theory-audit-claims.test.mjs`
directly during the inner loop; run all three before claiming a task done.

## Context

**Where the locks live.** `site/tests/theory-audit-claims.test.mjs` holds a
`FINDINGS` array of claim objects (from line 45), each asserted by
`assertClaim` in `site/tests/helpers/prose-claims.mjs`. A claim carries:

| Field | Meaning |
|---|---|
| `id` | stable case identifier, e.g. `S06-F09` |
| `file` | the `.mdx` filename under `site/src/content/docs/`, for the message |
| `rule` | the one-line authority the claim agrees with |
| `forbidden` / `forbiddenRegex` | phrases or patterns that must be ABSENT |
| `present` / `presentRegex` | phrases or patterns that must be PRESENT |

`forbidden` and `present` match after `normalizeProse` — lower-cased, curly
quotes folded, punctuation between words ignored, word order preserved. So a
`present` phrase must reproduce the corrected wording's word order but need not
match its punctuation. `forbiddenRegex` and `presentRegex` test the raw source
instead, which is what to use for markup such as `<sup>` citation links.

**The coverage test.** `theory-audit-claims covers at least the S02, S03, S04,
and S05 findings` (line 288) holds a hard-coded list of required case IDs. It is
additive: a case not on the list still runs, but the list is what stops a case
being deleted silently. **Every task below adds its own IDs to that list in the
same commit**, so no commit leaves the list lagging the cases it added. Rename
the test to name S06 as well when Task 1 first extends it.

**Two references already exist; do not add them.**
`fr-butler-2006` (Butler, *Unlocking the Groove*) and `fr-monson-1996` (Monson,
*Saying Something*) are both already in `appendix-references.mdx`. F13 and F15
are inline-citation work only.

**Do not re-tier `ref-36` or `ref-38`.** `ref-36` is a Brettworks blog post and
`ref-38` an Ethan Hein blog post — exactly the weak sources the audit objects
to. Replacing them is M002's remit (rows F17–F23). This slice hedges the
*claims* those references are attached to and leaves the reference list alone.
Touching it here would put two slices in the same paragraphs.

**Test-first, concretely.** Every task adds its case *before* editing the prose,
runs it, and sees it fail on the `present` phrase that is not there yet. That
failure is the proof the case is wired to the right file and the right sentence.
A case added after the edit proves only that the edit exists.

---

## Task 1 — Chapter 4: Fela/Allen co-attribution and Allen's timing

**Modifies:** `site/tests/theory-audit-claims.test.mjs`,
`site/src/content/docs/04-afrobeat.mdx`
**Rows:** F09, F10

Two claims in one file. F09 is the chapter opening; F10 is the Humanize bullet
in the macros section.

The current sentences are:

> Fela Kuti built the genre in 1960s and 70s Lagos, but it was his drummer Tony
> Allen who created its rhythmic vocabulary

> Tony Allen's timing was precise but not quantised — there was always a human
> push and pull against the grid.

### Steps

1. Add two claims to the `FINDINGS` array, `id: 'S06-F09'` and `id: 'S06-F10'`,
   both `file: '04-afrobeat.mdx'`.

   - `S06-F09`: `forbidden` must include the phrase
     `who created its rhythmic vocabulary`. `present` must include a
     co-attribution phrase you are about to write in step 3. `rule` cites audit
     2.6.1 / §5.11 and Allen's own co-attribution in Allen & Veal (2013).
   - `S06-F10`: `forbidden` must include `timing was precise but not quantised`.
     `present` must include the characterisation-marking phrase from step 3.
     `rule` cites audit 2.6.3 and the absence of any measurement study covering
     Allen's recordings.

2. Run `node --test site/tests/theory-audit-claims.test.mjs` and **watch both
   fail** on the missing `present` phrases. If either fails on `forbidden`
   instead, the phrase you wrote does not match the current prose — fix the
   claim, not the chapter.

3. Edit `04-afrobeat.mdx`. The agreed replacements, reviewed 2026-08-28:

   Chapter opening — replace `but it was his drummer Tony Allen who created its
   rhythmic vocabulary` so the sentence reads:

   > Afrobeat is what happens when West African polymetric stacking meets the
   > extended duration of funk and the harmonic sophistication of jazz. Fela
   > Kuti and his drummer Tony Allen built it together in 1960s and 70s Lagos,
   > and its rhythmic vocabulary came out of that partnership rather than from
   > either alone<sup>[14](/appendix-references/#ref-14)</sup> — a way of
   > playing that distributes energy across many bars, with parts fading in and
   > out, each instrument breathing on its own schedule. Allen worked his parts
   > out against Fela's horn arrangements, and co-attributed the result
   > throughout his own account of it
   > ([Allen & Veal 2013](/appendix-references/#fr-allen-veal-2013)).

   The Allen & Veal citation is deliberate and was reviewed against M002/S03's
   remit: it supports the co-attribution, which is a new claim, while `ref-14`
   is left in place for M002/S03 to re-tier. Two slices touch this paragraph,
   for two different reasons.

   Humanize bullet — replace `Tony Allen's timing was precise but not
   quantised — there was always a human push and pull against the grid.` with:

   > Allen's playing is usually described as precise but unquantised — a human
   > push and pull against the grid. That is a characterisation rather than a
   > measurement: no timing study of his recordings has been published, and
   > Humanize approximates the effect rather than reproducing anything
   > measured.

4. Re-run the test and watch both pass.

5. Add `'S06-F09'` and `'S06-F10'` to the required-ID list in the coverage test
   (line 288), and rename that test to
   `theory-audit-claims covers at least the S02, S03, S04, S05, and S06 findings`.

### Verification

1. `node --test site/tests/theory-audit-claims.test.mjs` — all cases pass.
2. Mutation: revert the chapter-opening sentence to the sole-creator wording,
   re-run, and confirm `S06-F09` fails naming the forbidden phrase. Restore
   with `git checkout -- site/src/content/docs/04-afrobeat.mdx`, which is safe
   here only because that file has no other uncommitted edits at this point.
3. `bash scripts/check-doc-conformance.sh`, `npm --prefix site test`, and
   `pre-commit run --all-files` — all exit 0.

---

## Task 2 — Chapter 8: attribute Reich's framing to Reich

**Modifies:** `site/tests/theory-audit-claims.test.mjs`,
`site/src/content/docs/08-minimalism.mdx`
**Rows:** F11

The current sentence asserts in the guide's own voice:

> The American minimalists of the 1960s and 70s discovered something that West
> African and Indonesian musicians had known for centuries: when two identical
> patterns move at slightly different rates, the interference between them
> generates far more complexity than either pattern contains alone

Three problems, per audit 2.7.1: it is Reich's framing rather than a settled
fact; African stacking uses fixed independent cycles and gamelan uses fixed
hierarchical nesting, so neither is the deliberate drift the sentence implies;
and the analogy was criticised by Agawu.

### Steps

1. Add a claim `id: 'S06-F11'`, `file: '08-minimalism.mdx'`.
   `forbidden` includes `musicians had known for centuries`. `present` includes
   both the attribution phrase and the distinction phrase from step 3. `rule`
   cites audit 2.7.1 and names Reich as the source of the framing.

   Note the S05-F07 case forbids `/\bfor centuries\b/i` but is scoped to
   `02-sub-saharan-africa.mdx`, so there is no conflict — each claim asserts
   against its own file.

2. Run the test and **watch it fail** on the missing `present` phrases.

3. Edit the chapter opening. The agreed replacement, reviewed 2026-08-28 —
   drop the "discovered something that … had known for centuries" clause, keep
   the mechanism sentence, and add a second paragraph:

   > When two identical patterns move at slightly different rates, the
   > interference between them generates far more complexity than either
   > pattern contains alone<sup>[32](/appendix-references/#ref-32)</sup>. Steve
   > Reich called it "music as a gradual process." Poly calls it the Drift
   > parameter.
   >
   > Reich framed his own discovery as arriving at something West African and
   > Indonesian musicians already knew, and that framing has carried into most
   > writing about minimalism since. It is worth holding loosely. West African
   > ensembles stack fixed independent cycles; gamelan nests fixed cycles
   > hierarchically. Neither drifts deliberately, and neither is doing what
   > *Piano Phase* does — the resemblance is in the resulting texture, not in
   > the technique.

4. Re-run and watch it pass.

5. Add `'S06-F11'` to the required-ID list.

### Verification

1. `node --test site/tests/theory-audit-claims.test.mjs` — all pass.
2. Mutation: restore `had known for centuries`, confirm `S06-F11` fails on the
   forbidden phrase, then restore the file.
3. `bash scripts/check-doc-conformance.sh`, `npm --prefix site test`,
   `pre-commit run --all-files` — all exit 0. The doc-conformance run matters
   here specifically: `S03-F02` and `S03-F03` also assert against this file, and
   this edit must not disturb them.

---

## Task 3 — Chapter 9: attribute the swing claim to Linn

**Modifies:** `site/tests/theory-audit-claims.test.mjs`,
`site/src/content/docs/09-electronic.mdx`
**Rows:** F13

Currently stated as fact, with the blog in a bare footnote:

> The fundamental difference between techno and house is often reducible to one
> parameter: swing<sup>[36](/appendix-references/#ref-36)</sup>.

### Steps

1. Add a claim `id: 'S06-F13'`, `file: '09-electronic.mdx'`.
   `forbidden` includes `difference between techno and house is often reducible
   to one parameter`. `present` includes the in-voice attribution to Linn from
   step 3. Add a `presentRegex` matching a link to `#fr-butler-2006`, because a
   cross-reference is markup rather than prose and `present` normalizes it away.
   `rule` cites audit 2.8 (Electronic) and Butler (2006) as the contrary
   reading.

2. Run the test and **watch it fail**.

3. Edit the paragraph. The agreed replacement, reviewed 2026-08-28:

   > Roger Linn, whose drum machines shaped both genres, argues that the
   > difference between techno and house often reduces to one parameter:
   > swing<sup>[36](/appendix-references/#ref-36)</sup>. Techno grooves are
   > typically straight — quantised to the grid with no micro-timing offset.
   > House music inherits its shuffle from disco and funk, displacing off-beat
   > notes slightly late to create a looser, more human feel. Butler's study of
   > electronic dance music treats the distinction as considerably more than a
   > swing setting, resting on tempo, timbre, and the structure of the
   > arrangement as much as on micro-timing
   > ([Butler 2006](/appendix-references/#fr-butler-2006)).

   Leave `ref-36` in place; re-tiering it is M002/S06's work.

4. Re-run and watch it pass.

5. Add `'S06-F13'` to the required-ID list.

### Verification

1. `node --test site/tests/theory-audit-claims.test.mjs` — all pass.
2. Mutation: delete the Butler cross-reference link, re-run, confirm `S06-F13`
   fails on the missing `presentRegex`. Restore.
3. `bash scripts/check-doc-conformance.sh`, `npm --prefix site test`,
   `pre-commit run --all-files` — all exit 0.

---

## Task 4 — Chapter 13: hedge the Amen-break superlative

**Modifies:** `site/tests/theory-audit-claims.test.mjs`,
`site/src/content/docs/13-drum-and-bass.mdx`
**Rows:** F14

Currently absolute:

> That solo — the Amen break — became the most sampled recording in music
> history <sup>[38](/appendix-references/#ref-38)</sup>

The claim is widely repeated and plausible but has never been established
against a catalogue; it is disputed rather than false.

### Steps

1. Add a claim `id: 'S06-F14'`, `file: '13-drum-and-bass.mdx'`.
   `forbidden` includes `became the most sampled recording in music history`.
   `present` includes the hedged construction from step 3. `rule` cites audit
   2.8 (D&B) and records that no sampling census establishes the superlative.

2. Run the test and **watch it fail**.

3. Edit the sentence. The agreed replacement, reviewed 2026-08-28 — replace
   `became the most sampled recording in music history <sup>` with:

   > That solo — the Amen break — is widely described as the most sampled
   > recording in music history<sup>[38](/appendix-references/#ref-38)</sup>,
   > the raw material for an entire genre …

   Note the stray space before `<sup>` closes as part of this edit. The rest of
   the paragraph is about what the break enabled musically and is not in
   dispute; leave it.

4. Re-run and watch it pass.

5. Add `'S06-F14'` to the required-ID list.

### Verification

1. `node --test site/tests/theory-audit-claims.test.mjs` — all pass.
2. Mutation: restore the unhedged sentence, confirm `S06-F14` fails, restore.
3. `bash scripts/check-doc-conformance.sh`, `npm --prefix site test`,
   `pre-commit run --all-files` — all exit 0.

---

## Task 5 — Chapter 12: cite Monson for Roach's independence

**Modifies:** `site/tests/theory-audit-claims.test.mjs`,
`site/src/content/docs/12-jazz.mdx`
**Rows:** F15

The claim is directionally right and entirely uncited:

> Max Roach's revolution was treating each limb as an independent melodic voice.
> Rather than the entire kit serving a single groove function, Roach's snare
> might play a phrase of its own while the bass drum accented the melody and the
> ride kept time above. Each limb had its own rhythmic logic.

### Steps

1. Add a claim `id: 'S06-F15'`, `file: '12-jazz.mdx'`, with a `presentRegex`
   matching a link to `#fr-monson-1996` within the Roach passage. Use a regex
   rather than `present` because the citation is a markdown link.
   `rule` cites audit 2.8 (Jazz) and names Monson (1996) as the authority.

2. Run the test and **watch it fail** on the missing citation link.

3. Add the citation. The agreed replacement, reviewed 2026-08-28:

   > Max Roach's revolution was treating each limb as an independent melodic
   > voice ([Monson 1996](/appendix-references/#fr-monson-1996)). Rather than
   > the entire kit serving a single groove function, …

   The entry already exists in `appendix-references.mdx` at
   `<span id="fr-monson-1996">` — do not add a second entry.

4. Re-run and watch it pass.

5. Add `'S06-F15'` to the required-ID list.

### Verification

1. `node --test site/tests/theory-audit-claims.test.mjs` — all pass.
2. Confirm the anchor resolves: `grep -n 'id="fr-monson-1996"'
   site/src/content/docs/appendix-references.mdx` returns exactly one line.
3. `bash scripts/check-doc-conformance.sh`, `npm --prefix site test`,
   `pre-commit run --all-files` — all exit 0.

---

## Task 6 — Mark the lcm translation at all three sites

**Modifies:** `site/tests/theory-audit-claims.test.mjs`,
`site/src/content/docs/01-foundations.mdx`,
`site/src/content/docs/05-gamelan.mdx`,
`site/src/content/docs/06-indian-classical.mdx`
**Rows:** F16

The guide equates the lcm convergence point of independent lanes with two
tradition-specific terms. That is a useful translation and the audit does not
ask for it to be withdrawn — only for it to be marked as the guide's own, since
neither tradition uses its term for "the point where independent cycle lengths
coincide".

The three sites:

| File | The sentence |
|---|---|
| `01-foundations.mdx` | "The two cycles converge every 84 steps (7 bars of 12) … that convergence point is the structural downbeat of the polymetric phrase." |
| `06-indian-classical.mdx` | "That shared downbeat *is* sam — the moment all cycles agree." |
| `05-gamelan.mdx` | "The convergence points — where all lanes hit simultaneously — become the deep structural markers of the groove." |

`01-foundations.mdx` is where the generic concept is introduced and carries no
tradition term; the other two attach one. All three are named in the ledger row,
so all three carry a note.

### Steps

1. Add one claim per site: `id: 'S06-F16-foundations'`, `'S06-F16-indian'` and
   `'S06-F16-gamelan'`, each with the matching `file`. Each `present` holds that
   site's caveat wording from step 3. `rule` cites audit §1 and states that the
   mapping is this guide's informal translation, not a term either tradition
   uses for lcm convergence.

   Three IDs rather than one because `assertClaim` asserts a single claim
   against a single file: one case cannot span three sources.

2. Run the test and **watch all three fail**.

3. Edit each site. The agreed wording, reviewed 2026-08-28:

   `06-indian-classical.mdx` — rename the section heading from
   `## Polymetric Convergence as Sam` to `## Polymetric Convergence and Sam`,
   so a reader skimming headings is not left with the identity claim the body
   then walks back. Nothing links to the old anchor and no test asserts it;
   both were checked before this was agreed. Then replace `That shared
   downbeat *is* sam — the moment all cycles agree.` with:

   > This guide calls that shared downbeat sam — the moment all cycles agree —
   > but the borrowing is ours: in performance, sam is the first matra of a
   > single tala cycle, a fixed point in one metric frame rather than a
   > coincidence between independent ones.

   `05-gamelan.mdx` — append to the "In Poly, you model colotomic structure…"
   paragraph:

   > Calling them gong strokes is this guide's shorthand: in the gamelan the
   > gong marks the completion of a fixed nested hierarchy, not the coincidence
   > of lanes whose cycle lengths are independent.

   `01-foundations.mdx` — append inside the `<ListenFor>` block:

   > Chapters 5 and 6 name this point with terms borrowed from gamelan and
   > Indian classical practice; the borrowing is this guide's own.

4. Re-run and watch all three pass.

5. Add all three IDs to the required-ID list.

### Verification

1. `node --test site/tests/theory-audit-claims.test.mjs` — all pass.
2. Mutation: delete the caveat from `06-indian-classical.mdx`, re-run, confirm
   `S06-F16-indian` fails and the other two still pass — proving the three cases
   are independently wired. Restore.
3. `bash scripts/check-doc-conformance.sh`, `npm --prefix site test`,
   `pre-commit run --all-files` — all exit 0. `06-indian-classical.mdx` also
   carries `S02-F01` and `S02-F12`, and `05-gamelan.mdx` is asserted by the
   theory-patch suite, so a full run rather than the single file matters here.

---

## Self-review

**Definition of Done → task.**

| DoD item | Satisfied by |
|---|---|
| Ch 4 credits Fela and Allen jointly | Task 1 step 3, proved by `S06-F09` |
| Ch 4's Allen timing reads as characterisation | Task 1 step 3, proved by `S06-F10` |
| Ch 8 attributes the framing to Reich | Task 2 step 3, proved by `S06-F11` |
| Ch 9 attributed to Linn, cross-refers to Butler | Task 3 step 3, proved by `S06-F13`'s phrase and its `presentRegex` |
| Ch 13's Amen superlative hedged | Task 4 step 3, proved by `S06-F14` |
| Ch 12's Roach claim carries a citation | Task 5 step 3, proved by `S06-F15` |
| lcm translation marked at all three sites | Task 6 step 3, proved by the three `S06-F16-*` cases |
| Each of the seven locked by a named case | Every task's steps 1 and 5; the coverage test's required-ID list is what makes deletion fail |

**Row → task.**

| Row | Closed by | Its `Verification` produced by |
|---|---|---|
| F09 | Task 1 | case `S06-F09`, mutation-proved in Task 1 verification 2 |
| F10 | Task 1 | case `S06-F10` |
| F11 | Task 2 | case `S06-F11`, mutation-proved in Task 2 verification 2 |
| F13 | Task 3 | case `S06-F13`, mutation-proved in Task 3 verification 2 |
| F14 | Task 4 | case `S06-F14`, mutation-proved in Task 4 verification 2 |
| F15 | Task 5 | case `S06-F15` plus the anchor check in Task 5 verification 2 |
| F16 | Task 6 | cases `S06-F16-foundations`, `S06-F16-indian`, `S06-F16-gamelan`, mutation-proved in Task 6 verification 2 |

**Placeholder scan.** No "TBD", no "add appropriate error handling", no "similar
to task N". Every helper named — `assertClaim`, `normalizeProse`,
`containsClaim`, `registerCase` — already exists in
`site/tests/helpers/prose-claims.mjs` or the host file. Every anchor named —
`fr-butler-2006`, `fr-monson-1996`, `ref-36`, `ref-38` — was confirmed present
in `appendix-references.mdx` before this plan was written.

**Name consistency.** Case IDs are `S06-F09`, `S06-F10`, `S06-F11`, `S06-F13`,
`S06-F14`, `S06-F15`, and the three `S06-F16-*` variants, used identically in
every task, in the coverage list, and in the tables above. There is no
`S06-F12`: F12 belongs to S02 and is already closed.

**The wording is specified, not left open.** Every correction's replacement
text was drafted and reviewed on 2026-08-28 before execution began, and each
task's step 3 carries it verbatim. The executor writes each claim's `present`
phrase in step 1 by quoting from the agreed replacement, so the case and the
prose cannot disagree. Two boundary questions were settled in that review and
are recorded where they apply: F09 cites Allen & Veal even though M002/S03 will
later re-tier `ref-14` in the same paragraph, because they support different
claims; and F16 renames a section heading, which was checked for inbound links
and test assertions first.
