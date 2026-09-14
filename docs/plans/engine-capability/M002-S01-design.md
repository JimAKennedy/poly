---
class: gated
---

# M002/S01 — design: kotekan modes and controlled overlap

**Slice:** M002/S01 — `docs/plans/engine-capability/ledger.md`
**Row:** EC06 · **Issue:** [#153](https://github.com/JimAKennedy/poly/issues/153)
**Status of this document:** written for approval before any code, per
`/jk:plan`'s architectural path.

## What the guide asks for

Two rules on `theory-gamelan.mdx` are currently held back by one engine
limitation:

- **Rule 4** — "Strict complementation is only the textbook case… both players
  strike together at cadence points, phrase joins and *angsel* figures… A pair
  with empty intersection everywhere sounds mechanical." Its parenthetical says
  Poly's kotekan implements the strict case and to add doublings by hand
  **until kotekan modes ship**.
- **Rule 5** — "The style of interlock is a named choice. *Norot* oscillates
  around neighbouring tones; *kotekan telu* shares three-pulse cells; *kotekan
  empat* four; *nyog cag* alternates strictly."

M007 of the theory-audit programme additionally found Rule 4's strict predicate
could not be made to fail at all, and marked Rule 1 not checkable, because the
engine derives sangsih as the exact complement so the composite is complete by
construction.

## What the engine does today

`buildLanePattern` in `engine/src/engine.cpp`, inside `region:kotekan`:

```
pattern[s] = !srcPattern[s]          // for s < min(cycle.steps, src.cycle.steps)
pattern[s] = true                    // beyond the source's cycle
if (complementHits == 0) euclidean(...)   // saturation fallback, M070
```

One behaviour, no choice: the strict complement. Four shipped presets use it —
`Kotekan Interlock`, `Ewe Polymetric Ensemble`, `Balinese Kotekan`,
`Afro-Electronic Fusion`.

## Constraints found before designing

Three facts shaped this design and were measured, not assumed.

**1. Poly gives each lane one MIDI note, so *norot* is out of reach.** Norot is
defined by pitch oscillation around neighbouring tones. A one-note lane cannot
express it, and a rhythmic stand-in would attach a traditional style name to
something that is not that style — the class of overclaim the theory-audit
programme spent seven milestones removing. **Three modes ship; norot is
documented as inexpressible and why.**

**2. Both per-lane VST3 parameter families are full.**

> **Corrected 2026-09-14, during task 4.** This section originally claimed the
> new fields could be state-only because "16 of `LaneConfig`'s 35 fields have no
> VST3 parameter, including `hitCount`, `cycle` and `microTimingMs`". That
> measurement was wrong: it grepped field names against the **lane-expression**
> family in `plugids.h` only. A second per-lane family exists —
> `kLaneCoreFields[]` — and `hitCount`, `cycle.steps`, `rotation`, `timeline`
> and `fillEveryNBars` all have parameters there. Very few lane fields are
> genuinely parameterless.
>
> The error mattered because **the WebUI edit path is parameter-ID based**:
> `host.edit('lane.3.timeline', …)` resolves through `resolveParamId`, so a
> field with no parameter ID cannot be edited from the interface at all. The
> design asserted both "state-only" and "WebUI-editable", which this
> architecture does not allow together.

`kParamsPerLane = 16` with all sixteen slots used, and `kCoreParamsPerLane = 12`
with all twelve used — M034 consumed the last two for `fillEveryN` and
`seedLock`. Adding a parameter therefore requires raising a stride, and
`laneCoreParam(lane, offset) = kLaneCoreBase + lane * 12 + offset` shifts every
lane above 0.

**The core family is raised to 14**, and the blast radius was measured before
choosing it: `getState`/`setState` serialize the `SceneState` blob, so presets
and saved projects are parameter-ID independent and nothing about loading
breaks. Only host *automation lanes* targeting per-lane core params on lanes 1–7
shift — and Poly has never been released, the only tag being
`v0.1.0-doccov-baseline` with no GitHub releases.

**3. Existing output must not move except where intended.** The default must
reproduce today's behaviour exactly, so the determinism goldens and three of the
four kotekan presets stay byte-identical.

## Design

Two state-only fields on `LaneConfig`:

```cpp
enum class KotekanMode : uint8_t { NyogCag = 0, Telu = 1, Empat = 2 };

KotekanMode kotekanMode = KotekanMode::NyogCag;  // default = today's behaviour
int kotekanOverlap = 0;                          // structural shared strikes
```

### Mode semantics, stated mechanically

Each mode is defined by what the engine computes, not by an appeal to the
tradition. `src` is the source lane's resolved Euclidean pattern.

| Mode | Rule |
|---|---|
| `NyogCag` | `pattern[s] = !src[s]` — the strict complement, byte-identical to today |
| `Telu` | the interlock repeats on a three-pulse cell: `pattern[s] = !src[s % 3]` |
| `Empat` | the same on a four-pulse cell: `pattern[s] = !src[s % 4]` |

The saturation fallback (`complementHits == 0` → the lane's own Euclidean
pattern) applies unchanged to all three.

### Overlap semantics

`kotekanOverlap` is a count of structural points at which **both** parts strike,
applied after the mode's pattern is derived:

- `0` — no forced overlap. Identical to today.
- `N > 0` — force `pattern[s] = true` at the first `N` structural points, in the
  order Rule 4 names them: the cycle boundary (step 0) first, then the phrase
  join if the lane has phrase gating, then the midpoint.

Forcing rather than toggling matters: the point is a *shared* strike, so the
step must be on in both parts regardless of what the complement said.

### Why this makes the guide's rules checkable

- Rule 4's strict reading becomes falsifiable: a patch with
  `kotekanOverlap > 0` has a non-empty intersection, so a predicate asserting
  overlap at structural tones can fail on a patch that sets it to 0.
- Rule 5 becomes a setting for three of its four styles, with norot's absence
  stated rather than silently implied.
- Rule 1 becomes checkable because the composite is no longer complete by
  construction — `Telu` and `Empat` leave gaps the strict complement did not.

## Surface

| File | Change |
|---|---|
| `engine/include/poly/types.h` | `KotekanMode` enum; two `LaneConfig` fields |
| `engine/src/engine.cpp` | `region:kotekan` branches on mode; overlap applied after |
| `engine/src/sanitize.cpp` | clamp mode to the enum, overlap to `[0, cycle.steps]` |
| `engine/include/poly/state_io_*.h` | write both; read guarded by a new `kKotekanModeStateVersion = 19` |
| `engine/include/poly/state_io_envelope.h` | `kCurrentStateVersion` 18 → 19 |
| `engine/tools/emit_presets.cpp` | emit both; `schemaVersion` 4 → 5 |
| `site/scripts/generate-presets-json.mjs` | version guard 4 → 5 |
| `site/tests/presets-json-schema.test.mjs` | expected version and field assertions |
| `plugin/source/webui/bridge_serialization.cpp`, `web_ui_view.cpp` | bridge both fields |
| `webui/` | a mode selector and an overlap stepper on the lane pane |
| `engine/src/presets.cpp` | `Balinese Kotekan`'s guiro lane: `Telu`, overlap 1 |

## Migration

The established pattern, followed exactly: `kCurrentStateVersion` goes to 19 and
the read path guards the new fields behind `kKotekanModeStateVersion`, as
`kFillEveryNBarsStateVersion = 17` and `kLaneSeedLockStateVersion = 18` already
do. A v18 state loads with `NyogCag` and overlap 0 — which is what it played
before — so the migration is lossless by construction rather than by conversion.

## Risks

**The one worth arguing about:** `Telu` and `Empat` are given a purely rhythmic
reading — a cell length over which the interlock repeats — while the terms also
carry pitch meaning in the tradition. Keeping the traditional names is only
defensible if the guide says plainly what is and is not modelled, so that is not
left as a caution here: it is a **required, specified disclosure** and a
definition-of-done item on S02.

The disclosure must do three things:

1. State that Poly models **only the rhythmic dimension** of *telu* and *empat*
   — the cell length on which the interlock repeats — and not their pitch
   content.
2. Direct the reader to the sources for that pitch dimension by name:
   [Tenzer 2000](/appendix-references/#fr-tenzer-2000), already annotated in the
   appendix as the authoritative analysis of kotekan varieties, and
   [Vitale 1990](/appendix-references/#fr-vitale-1990), annotated as showing
   real kotekan is not a pure set complement — which is Rule 4's own substance.
3. Name *norot* as not expressible in Poly, and say why: it is defined by pitch
   oscillation around neighbouring tones, and a Poly lane carries one note.

Written in the voice M003 used for its five simplification disclosures —
at the point of use, not in a footnote. Without it this design imports exactly
the overclaim the theory-audit programme existed to remove; the alternative
considered and rejected was dropping the traditional names for `Cell3`/`Cell4`,
which would lose the reader the connection to the literature the guide is built
on.

**Second:** `Balinese Kotekan`'s golden fixture moves. That is intended and
reviewed in one place; the other three kotekan presets keep `NyogCag` and stay
byte-identical.

## What S02 then consumes

The mode reaching `presets.json` is what lets a patch table carry a `Kotekan`
column naming the mode, which is what makes Rules 1, 4 and 5 checkable. S02
removes Rule 4's parenthetical, restores its strict predicate, flips Rule 1 to
checkable, and writes the norot and telu/empat disclosures.
