# M004/S05 — named-rule conformance checklist: design

**Slice:** M004/S05, in `docs/plans/theory-audit/ledger.md`
**Classification:** architectural — a new conformance mechanism and a new
escape-hatch contract, both of which outlive this milestone.

## The problem

`site/tests/theory-patch-conformance.test.mjs` hand-writes one test per known
defect. It has nine tests, one per theory page, each encoding a single
rule-satisfaction predicate from the 2026-07-30 conformance review. All nine
pass — the review's defects are remediated.

What it cannot do is notice a *new* contradiction. Three limits:

1. **One predicate per page.** Eleven theory pages state 92 numbered rules
   between them; nine are asserted. `theory-gamelan` states ten rules and the
   suite checks one of them.
2. **Theory pages only.** Chapter patches are uncovered entirely, which is
   where F37, F40 and F41 live.
3. **First patch only.** `parsePolyPatch` takes the first `<PolyPatch>` in a
   file. Chapter 2 has two (Ewe, Manding) and Chapter 5 has two (Balinese
   kotekan, Javanese colotomy); the second of each is unreachable.

Two theory pages — `theory-electronic-breakbeat` (9 rules) and
`theory-minimalism` (8 rules) — carry no assertion at all.

## What this slice builds

The mechanism, plus enough coverage to prove it and to unblock its siblings.
The rest of the 92 rules is M007's, named in this slice's definition of done so
the boundary is explicit rather than implied by silence.

### 1. The checklist, declared as data

```js
const CHECKLIST = [
  {
    page: '02-sub-saharan-africa.mdx',
    patch: 'Ewe-Inspired Polymetric Ensemble',
    rules: [
      { id: 'ssa-construction-2', description: '...', predicate: (patch) => ... },
    ],
  },
];
```

Iterating a declared list rather than writing a `test()` per defect changes
what absence looks like. Today an unchecked page is invisible; with a
checklist, a page with no entry is a missing row in a structure the suite
prints. That is the difference between "we checked and it passed" and "nobody
looked", which is the distinction this slice exists to make.

Rule ids are stable strings, because the divergence marker refers to them and a
marker pointing at a renamed rule must fail rather than silently pass.

### 2. Multi-patch parsing

`parsePolyPatch(src, title)` — the existing signature keeps working when
`title` is omitted, so the nine current tests are untouched. This is the whole
change needed to reach Chapter 2's Manding patch and Chapter 5's Javanese one.

### 3. The divergence marker

An MDX comment immediately above the patch it excuses:

```mdx
{/* patch-divergence-ok: ssa-construction-2 — this patch is deliberately the
    four-lane teaching reduction; the fuller construction is on the theory
    page. */}
```

It satisfies exactly the named rule for exactly that patch. Not the page, not
the next rule, not by proximity.

**Why this form.** It matches the four properties `escape-hatch-discipline`
requires, and the three markers the repo already uses — `citation-tier-ok`,
`boundary-ok`, `RT-SAFE-OK`:

- **In-band**, so it cannot drift from what it excuses. A registry would.
- **Greppable** on one fixed token: `grep -rn patch-divergence-ok` *is* the
  audit report.
- **Reasoned**, and the reason is a claim a reviewer can check, not "false
  positive".
- **Counted** — the suite prints the live marker total, so the number cannot
  shrink into silence. M002's `citation-tier-ok` count is the precedent.

The narrowest scope that works is one rule on one patch, so the marker takes a
rule id rather than excusing a whole page.

### 4. Falsifiability, per rule

Every checklist entry must be shown to fail when the lane it guards is removed,
with the failure recorded in the evidence file. This is the slice's fourth
definition-of-done item and it is not ceremony: three of M003's locks passed on
the day they were written, and for those the deletion test was the only thing
distinguishing a guard from a decoration.

A rule whose predicate cannot be made to fail is not checkable, and belongs in
M007/S01's not-checkable column with its reason — not in the checklist.

## Coverage this slice takes

| Page | Patch | Why |
|---|---|---|
| `02-sub-saharan-africa.mdx` | Ewe-Inspired Polymetric Ensemble | F37 |
| `03-afro-cuban.mdx` | Cuban Son Ensemble | F38, F40 |
| `05-gamelan.mdx` | Balinese Kotekan Interlocking | F41 |
| `theory-electronic-breakbeat.mdx` | its construction patch | no assertion today |
| `theory-minimalism.mdx` | its construction patch | no assertion today |

The two theory pages are in scope because they are the only pages with zero
coverage, and closing that blind spot while the machinery is being built costs
less than a slice of its own later. They also exercise the checklist on theory
pages, so M007 inherits a mechanism proven on both page kinds rather than on
chapter patches alone.

Chapter 2's Manding patch and Chapter 5's Javanese patch are reachable once
multi-patch parsing lands, but carry no checklist entry in this slice — no row
requires them, and adding entries nobody asked for is how a slice widens.

## What happens when the checklist finds something else

The 83 unchecked rules make an untriaged finding likely even within this
slice's five patches. When one appears:

1. Add a `patch-divergence-ok` marker with the reason `found by the checklist,
   not yet triaged`.
2. Open a `B` row in the ledger naming the page and the rule.
3. Continue.

The suite stays green, the finding is greppable and tracked, and no slice
widens to chase it — the treatment B10 and B11 received. M007/S03 burns the
markers down, and its B14 fails on any marker still carrying that placeholder
reason, so the backlog cannot become permanent by inattention.

## What this design does not do

- It does not change any patch. Corrections are S01, S02 and S03's work; this
  slice builds what they are verified by.
- It does not touch the nine existing tests. They pass, they encode real
  review defects, and rewriting green tests to fit a new shape risks losing a
  predicate for no gain. M007 may fold them into the checklist once the triage
  says what each one is asserting.
- It does not decide which of the 92 rules are checkable. That is M007/S01,
  deliberately, because the answer wants looking at all of them at once rather
  than five at a time.

## Risks

- **The checklist becomes a place rules go to look checked.** A predicate that
  asserts something trivially true reads the same as one that bites. The
  per-rule mutation proof is the mitigation, and it is a definition-of-done
  item rather than a convention.
- **Rule ids drift from the pages' numbering.** A page that renumbers its rules
  leaves the checklist pointing at the wrong text. Ids are descriptive
  (`ssa-construction-2`), not positional (`rule-2`), which makes the mismatch
  legible; M007/S01's triage case is what would catch it.
- **The marker gets used to make red go away.** Counted output and B14's
  placeholder check are the mitigations, plus the reason field being a claim
  rather than a category.
