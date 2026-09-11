# M006/S04 — Unattested terms

**Slice:** M006/S04, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M006-decisions.md` — B06, B07 and B11 were researched before
planning, and three ledger amendments follow from what that found.

## Task status

- [ ] Task 1 — Close B11 on the evidence

## Definition of Done

- [ ] *Kotekan polos*, and the practice Rule 5 describes without it, are either
      cited to a tier-A or tier-B source, or recorded here as unverifiable and
      deliberately left uncited

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

`.jk/validations.yml` is authoritative if this table disagrees with it.

## The answer, found during planning

The audit calls *kotekan polos* "a third player playing only the structural
pokok tones". That is not what the term denotes. Sources agree **polos is one of
the two interlocking kotekan parts** — the on-beat one, paired with *sangsih* —
and the **pokok is the main melody**, played on calung and ugal, which kotekan
embellishes. Polos does not play the pokok; it is half of what ornaments it.

This is precisely what the guide's own Rule 3 already says, and it is why
M003/S05 refused to adopt the label: it would have collided with the page's
established usage. That refusal was correct.

## Task 1 — Close B11 on the evidence

Closes **B11**.

**Files:** `docs/plans/theory-audit/ledger.md`,
`docs/plans/theory-audit/evidence/M006-S04.md`, this plan

**Change no prose.** Rule 5 is already right: it describes a third part doubling
the pokok tones without the label, grounded on Rule 6 and Construction's Pokok
lane. Nothing in the guide needs correcting, and adding a citation for a term
that does not denote what the audit says it denotes would be worse than leaving
it out.

1. **Confirm `S05-F33` still passes and still forbids the label.** Run
   `node --test site/tests/scope-framing.test.mjs` and check the case is green,
   then prove the forbidden arm still bites by inserting `kotekan polos` into
   Rule 5, watching it fail, and removing it by inverse edit. That arm is now
   the load-bearing part of this row's resolution: it is what stops a future
   editor adopting a term this milestone established is wrong.
2. **Close B11 as `accepted`**, not `done` — the row asked for an attestation
   and the answer is that none exists because the premise is mistaken. Its
   `Item` records what the sources say. Set its disposition to `accept`.
3. **Tick the definition-of-done box**, which offers exactly this outcome:
   "recorded here as unverifiable and deliberately left uncited". Tick Task 1,
   set the slice `Status` to `done`.
4. **Run every token**, then `jk-standards ledger`.
5. **Append the evidence**, recording what the sources establish and that
   `S05-F33` was re-proved to bite.
6. **Commit** with `Rows: B11`.

## Self-review

**DoD coverage.** The single item offers two outcomes — cited to a tier-A/B
source, or recorded as unverifiable and left uncited. Task 1 takes the second,
on evidence rather than for want of effort.

**Row coverage.** B11 → Task 1 step 2, closed `accepted` with its disposition
changed to `accept`.

**Placeholder scan.** No TBDs. The task states outright that no prose changes.

**Name consistency.** `S05-F33` is the case named throughout; `kotekan polos` is
the forbidden phrase it guards.
