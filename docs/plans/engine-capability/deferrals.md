---
class: gated
---

# Engine Capability: What the Guide Promises and Poly Cannot Yet Do

Status: current (2026-09-12)

**Source of this document:** the theory-audit programme
(`docs/plans/theory-audit/ledger.md`, seven milestones, complete). That
programme corrected the guide against an external audit and locked every
correction with a test. Along the way it recorded, repeatedly and in the tree,
the places where **the guide is honest about a limitation in Poly itself**.

This document gathers those, so a successor programme has a source it can claim
complete coverage of — the property that made the theory-audit ledger
assessable. It is not a wish list and not a reading of the issue tracker: every
item below is a promise, disclosure or weakened rule that exists in the repo
today, with its location.

## Why this is a programme and not a backlog

Three distinct kinds of debt, all created by the same gap:

1. **Prose that promises a future capability.** Three passages say outright that
   Poly cannot do something yet. Each is a claim the guide makes about its own
   tool, and each stops being true when the capability ships.
2. **A rule weakened to fit the engine.** M004/S03 changed what
   `theory-gamelan` Rule 4 is checked against, because the strict reading
   demands something `Kotekan L`-mode cannot produce. That is the only rule in
   the guide whose *check* was relaxed for a tool limitation rather than a
   scholarly one.
3. **Rules that cannot be checked because the patch table cannot express them.**
   M007's triage marked 43 of 92 numbered rules not checkable. Most are prose
   judgements no predicate settles. A handful are different: the rule is
   mechanical and the table simply has no column for it.

Closing (1) and (2) changes the guide's text. Closing (3) changes what a patch
table can say, which is a docs-and-engine question rather than a prose one.

## 1. Promises in the guide's own prose

### D1 — Kotekan modes

`theory-gamelan.mdx` Rule 4 ends:

> (Poly's Kotekan `L1` implements the strict case; add deliberate doublings via
> a third lane or accent masks **until kotekan modes ship**.)

Tracker: [#153](https://github.com/JimAKennedy/poly/issues/153) — norot, telu,
empat variants and controlled polos–sangsih overlap.

**What shipping it changes.** The parenthetical goes. Rule 5 names four interlock
styles as "a named choice" and Poly can express none of them; with modes, the
rule becomes a setting rather than a description. And D4 below becomes
resolvable.

### D2 — Subdivision profiles

`theory-brazilian.mdx` Rule 6 ends:

> Swing parameters approximate it poorly (they displace only alternate notes);
> **until subdivision profiles ship**, use light swing (0.15–0.25) plus small
> per-lane offsets as an admitted approximation.

Tracker: [#150](https://github.com/JimAKennedy/poly/issues/150) —
non-isochronous subdivision profiles (samba/jembe feel templates).

### D3 — A measured jembe profile

`theory-sub-saharan-africa.mdx` construction step 5, rewritten by M003/S04 (row
F31, and B10 which corrected an earlier overstatement):

> Poly's per-step **micro-timing** offsets are where such a profile would be
> entered, one step at a time; what **Poly does not ship is a measured jembe
> profile** to put in them.

Tracker: [#150](https://github.com/JimAKennedy/poly/issues/150) again — the
mechanism exists (`microTimingMs`, exposed through the WebUI's micro-timing
bars, clamped to ±20 ms); the measured data does not.

**Note the distinction B10 established**, because it scopes the work: this is not
a missing feature but missing content. A profile is data to author, not code to
write — though #150's template mechanism is what makes authoring it useful.

## 2. The rule weakened for the engine

### D4 — Rule 4's strict reading

`theory-audit` row F41 records M004/S03's decision verbatim:

> Rule 4 is checked as construction step 4 specifies it — a lane outside the
> kotekan pair sounding at the cycle boundary — because Rule 4 **as written
> demands pair-overlap that Poly's Kotekan L-mode cannot produce**, as its own
> parenthetical says.

M007 then found the strict predicate could not be made to fail at all under
L-mode, and marked Rule 1 not checkable for the same reason: the engine derives
sangsih as the strict complement, so the composite is complete by construction.

**Two rules on one page are weakened or unverifiable because of one engine
limitation.** D1 is what resolves both.

## 3. Rules the patch table cannot express

M007's triage marked five rules not checkable *specifically* because the page's
patch table lacks a column, each carrying an `absentColumn` annotation that a
test now verifies:

| Page | Rule | Column absent |
|---|---|---|
| `theory-balkan` | 7 — "Humanize ≤ 0.15" | `Humanize` |
| `theory-electronic-breakbeat` | 4 — "swing is a bus, not a per-note gesture" | `Swing` |
| `theory-gamelan` | 9 — density scales inversely with register | `Note` |
| `theory-sub-saharan-africa` | 7 — register and rate separate the voices | `Note` |
| `theory-brazilian` | 6 — the long-short-short-long feel | `Timing` |

These are the cheapest items here: four are a column added to a table whose
values the engine already holds. `theory-brazilian` Rule 6 is not — it needs D2,
because a per-beat profile is not a single timing offset.

**A caution this programme earned.** Two verdicts in this category were wrong
when M007 shipped: `theory-funk-soul` Rule 6 and `theory-jazz` Rule 6 both
claimed an absent column that was present. Row B18 records it. Any assessment of
this section should re-derive the column lists from the tables rather than trust
the table above.

## 4. Deferred by name in the theory-audit ledger

Its Related-issues section says, of each:

- [#156](https://github.com/JimAKennedy/poly/issues/156) — **not closed here.**
  "F31 discloses that Humanize applies random jitter rather than Polak's
  systematic profiles; the exact non-Euclidean-timeline preset work stays on the
  tracker."
- [#157](https://github.com/JimAKennedy/poly/issues/157) — **not closed here.**
  "F32 locks the Balkan long-beat honesty note; cell-aware swing for aksak is a
  real engine change and stays on the tracker."

## What is deliberately not here

- **CI and tooling debt** — [#282](https://github.com/JimAKennedy/poly/issues/282)
  (non-reproducible WASM), [#272](https://github.com/JimAKennedy/poly/issues/272)
  (CI never runs the site suite), [#274](https://github.com/JimAKennedy/poly/issues/274),
  [#267](https://github.com/JimAKennedy/poly/issues/267),
  [#266](https://github.com/JimAKennedy/poly/issues/266). Real, but they
  interlock with nothing here and four of them are single fixes. A ledger's
  traceability costs more than it returns on work that does not interlock.
- **Feature issues with no promise behind them** —
  [#245](https://github.com/JimAKennedy/poly/issues/245),
  [#152](https://github.com/JimAKennedy/poly/issues/152),
  [#154](https://github.com/JimAKennedy/poly/issues/154),
  [#155](https://github.com/JimAKennedy/poly/issues/155),
  [#158](https://github.com/JimAKennedy/poly/issues/158) and others are good
  ideas the guide does not currently claim. Including them would make this
  document a backlog, and a ledger assessed from a backlog cannot claim complete
  coverage of anything.

The line is deliberate: **an item belongs here only if the repo already says
Poly should do it.** That is what makes the set finite and the coverage claim
checkable.

## The coverage claim an assessment can make

Every item above appears in the tree today and is locatable:

- three prose promises, greppable as `until .* ship` and "Poly does not ship"
- one weakened rule, recorded in ledger row F41 and in `RULE_TRIAGE`
- five `absentColumn` verdicts, verified by a test that the column really is
  absent
- two issues the theory-audit ledger names as deferred

A successor ledger should carry one row per item and be able to say, as the
theory-audit ledger says of `F01–F54`, that the set is complete against this
document.
