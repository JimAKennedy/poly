---
class: gated
---

# M002/S06 design — reference tiers and the tier check

Status: current (2026-09-07)

**Slice:** M002/S06 · **Ledger:** `docs/plans/theory-audit/ledger.md`

Written before planning because this slice adds a mechanism rather than editing
prose: a per-entry data format across 99 references, a new check, an escape
hatch, and a definition of "named-theory claim" that has to be mechanised.

## 1. The citation grammar the check rests on

The guide already distinguishes two citation forms, and M002/S02–S05 validated
the distinction across four chapters and three companion pages:

| Form | Meaning | Example |
|---|---|---|
| `<sup>[N](/appendix-references/#ref-N)</sup>` | a **named-theory claim** cites this source | `03-afro-cuban.mdx` line 32, before S02 |
| `([Author Year](/appendix-references/#fr-…))` | a named-theory claim cites a Further Reading work | the form S02–S05 corrected *to* |
| `[N](…#ref-N)–[M](…#ref-M)` in a Sources section | a bibliographic pointer, **not** a claim | `theory-afro-cuban.mdx` "See also refs [10]–[13]" |

That grammar is what makes the check mechanisable: **a superscript is a claim
citation, a plain link is not.** No natural-language judgement is required, and
the Sources listings that deliberately point at low-tier refs are structurally
out of scope rather than exempted one by one.

A superscript may carry more than one reference —
`<sup>[21](…#ref-21), [22](…#ref-22)</sup>` — so every pattern in this slice
uses the generalised form `<sup>[^<]*#ref-\d+[^<]*</sup>`, never the
single-reference form S02 and S03 used. M002/S04 proved directly that the
narrow form fails to match a two-reference block.

## 2. How a tier is declared

Add a `data-tier` attribute to the `<span>` each entry already carries:

```html
<span id="ref-1" data-tier="A">**[1]**</span> Toussaint, G. T. (2005). …
- <span id="fr-clayton-2000" data-tier="A">Clayton, M. (2000). …</span>
```

Chosen over a separate tier manifest because the tier then cannot drift from
the entry it describes — the same reason the escape hatches in this repo are
in-band rather than in a registry. It is an attribute, so it renders nothing,
and both `ref-*` and `fr-*` entries already use the span, so one rule covers
all 99.

Tier values are `A`, `B`, `C`, taking the audit's definitions
(`docs/audits/poly_theory_audit.md` §4): **A** peer-reviewed scholarship or a
primary source, **B** secondary or educational but legitimate, **C** hobbyist
media. The audit classifies about half the entries by name; the rest are
judged against those definitions when the tier is written, and the entry's own
publisher makes the call obvious in nearly every case.

## 3. What the check asserts

`site/tests/citation-tier.test.mjs` — the host M002/S01 created and S02–S05
extended — gains three assertions beyond its existing per-slice claim cases:

1. **Every bibliography entry declares a tier.** Every `<span id="ref-…">` and
   `<span id="fr-…">` in `appendix-references.mdx` carries `data-tier` with a
   value in `A|B|C`. This is Definition-of-Done item 1, and it is what stops a
   new reference arriving untiered.
2. **Every claim citation resolves to Tier A.** For every superscript block in
   every `.mdx` under the docs root, each `#ref-N` it contains must resolve to
   an entry whose `data-tier` is `A` — unless suppressed (§4).
3. **The Lomax attribution resolves.** Definition-of-Done item 2 and row F22.

Assertion 2 is the one F23 asks for. Note it is deliberately *stronger* than
"Tier-B or Tier-C must not be cited": it requires Tier A positively, so an
entry with a malformed or missing tier fails rather than passing by omission.

## 4. The escape hatch

Assertion 2 does not pass on today's tree. Seven superscript citations resolve
to sources that are not Tier A, in chapters this milestone has no slice for:

| Ref | Source | Cited in |
|---|---|---|
| 3 | Wikipedia, "Euclidean Rhythm" | `01-foundations.mdx` |
| 13 | Sher Music sample pages | `03-afro-cuban.mdx` |
| 20 | Gamelan New Zealand PDF | `05-gamelan.mdx` |
| 32 | Wikipedia, "Steve Reich" | `08-minimalism.mdx` |
| 33 | All Classical Portland | `08-minimalism.mdx` |
| 36 | Brettworks blog | `09-electronic.mdx`, `theory-electronic-breakbeat.mdx` |
| 38 | Ethan Hein blog | `13-drum-and-bass.mdx` |

The audit's Tier-C list named refs 10, 11, 14–17, 25, 26 and 27 — every one of
which S02–S05 has now fixed. These seven were never flagged, so fixing them is
outside what M002 scoped and each needs its own source judgement.

They are therefore **suppressed in band, not ignored**, following this repo's
existing convention (`RT-SAFE-OK`, `boundary-ok`, `doc-coverage-ok`,
`provenance-ok`):

```mdx
{/* citation-tier-ok: Wikipedia cited for the term's popular currency, not for
    the Euclidean result itself, which cites Toussaint (ref-1) in the same
    paragraph. Upgrade tracked in the follow-up issue. */}
```

- **Token:** `citation-tier-ok`, one fixed greppable string, so
  `grep -rn citation-tier-ok site/src/content/docs` enumerates every live
  exemption. That listing is the audit report.
- **Scope:** the marker applies to the citing line when it appears on that line
  or the line immediately above — the narrowest scope that works in MDX, where
  a superscript sits mid-paragraph. No file-level or global form: seven
  exceptions for seven different reasons should not become one blanket waiver.
- **Reason:** required. A marker whose reason does not say *why this source is
  acceptable here, or what replaces it* is a finding in its own right.
- **Count:** the check prints the live suppression count in its output, so a
  rising number is visible rather than silent.

Each of the seven reasons must be written honestly at execution time by reading
the claim. Where the real answer is "this should cite scholarship and does
not", the reason says so and names the follow-up issue rather than pretending
the citation is fine.

## 5. Why not the alternatives

**A ratchet baseline file** (the other pattern the escape-hatch discipline
offers) was rejected: it is machine-written and identity-matched on content
hashes, which suits a large finding set being burned down mechanically. Seven
exceptions, each needing a human-readable justification tied to a specific
claim, are better served by in-band markers a reviewer meets while reading the
prose.

**Narrowing the check to the claims M002 corrected** was rejected because the
check would then pass while Wikipedia is cited for the Euclidean-rhythm claim —
it would assert something weaker than F23 says it asserts, which is the class
of defect this milestone exists to remove.

**Widening the slice to fix all seven** was rejected as another milestone's
work inside one slice, in chapters with no slice of their own.

## 6. F22 — the Lomax attribution

`03-afro-cuban.mdx` line 48 says the habanera rhythm is what "Jelly Roll Morton
called 'the Spanish tinge'". The claim is accurate and uncited, and no Lomax
entry exists anywhere in the appendix — so this needs both a new Further
Reading entry and the inline citation, not a re-point.

Lomax's *Mister Jelly Roll* (1950) is the book built from the Library of
Congress Morton interviews in which Morton uses the phrase. It is a primary
source, so `data-tier="A"`.

## 7. Wiring

The new check is added to `scripts/check-doc-conformance.sh` **and** to the
`REQUIRED` array in `site/tests/doc-conformance-wiring.test.mjs`. That file's
header states the contract: adding a guardrail means adding it to both. The
runner alone leaves the check undefended against the coverage-drop failure mode
that guard exists to catch.

Note this closes the CI gap for this one file only. Nothing in CI runs
`npm --prefix site test`, so `citation-tier.test.mjs` has run nowhere but a
developer's machine for five slices; poly issue #272 covers the general case.

## 8. Task shape

Four tasks, each ending green on its own assertion:

1. Tier all 99 entries and assert every entry declares one.
2. Add the Tier-A claim rule, the `citation-tier-ok` hatch, and the seven
   reasoned suppressions.
3. F22 — the Lomax entry and the inline citation.
4. Wire into the runner and `REQUIRED`; run `gate`; close the slice.

Task 4 owes `gate`, the full pre-push suite, which no slice in this programme
has owed before.
