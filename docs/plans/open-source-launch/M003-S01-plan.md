# M003/S01 — Cubase is measured, and nothing else is claimed

**Slice:** M003/S01 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS14 (the README claims any VST3 host), OS15 (the per-lane channel
default is undecided)
**Depends:** nothing.
**Decisions consumed:** `M003-decisions.md`, 2026-09-29 — Cubase only; the
macOS row is Cubase Pro 15 from the owner, the Windows row Cubase 14 from
nightly run 36511596046; the per-lane default stays and chapter 17 opens with
the one-instrument case.

## Task status

- [ ] Task 1 — The README and chapter 17 carry one host table, Cubase in it
      and nothing else claimed (OS14)
- [ ] Task 2 — Chapter 17's routing starts with one drum instrument and
      keeps per-lane channels as the advanced case (OS15); the slice closes

## Definition of Done

- [ ] Cubase has been loaded, routed and played on both shipping OSes, with the
      host version and the exact routing steps recorded — Windows from a named
      nightly run, macOS from the owner's session
- [ ] The README and guide chapter 17 carry the same host table: Cubase as
      supported, every other host as untested, and no "should work" claim
- [ ] The default channel layout is decided against what Cubase did

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `site-unit` | `npm --prefix site test` |

The task's own check, run first: `node --test site/tests/host-table.test.mjs`.

## The host table

Identical in `README.md` (under `## DAW compatibility`) and in
`site/src/content/docs/17-midi-routing-note-map.mdx` (a new `## Hosts`
section placed before `## Per-Lane MIDI Channel`):

```markdown
| Host | Version | Platform | Status | Routing |
|---|---|---|---|---|
| Cubase | Pro 15 | macOS | supported | Instrument track for Poly; a drum instrument whose MIDI input is Poly on all channels, or a MIDI Send from the Poly track |
| Cubase | 14 | Windows | supported | The same; exercised nightly by the Cubase harness |
| Other VST3 hosts | — | — | untested | Routing MIDI out of a plugin is where hosts differ most; nothing has been measured |
```

Below the table, one sentence in both places: "Poly is built and tested in
Cubase. Other hosts may work and have not been tried; a measured host table
for them is planned." No "should work with any VST3-compatible host".

## Task 1 — The README and chapter 17 carry one host table, Cubase in it and nothing else claimed (OS14)

**Files:** create `site/tests/host-table.test.mjs`; modify `README.md`,
`site/src/content/docs/17-midi-routing-note-map.mdx`; create
`docs/plans/open-source-launch/evidence/M003-S01.md`.

1. Write `site/tests/host-table.test.mjs` in the style of the other
   `site/tests/*.test.mjs` files (node:test, read both files from the repo
   root). Tests:
   - `README.md and chapter 17 carry the same host table`: extract the
     first markdown table whose header row is exactly
     `| Host | Version | Platform | Status | Routing |` from each file and
     assert the two arrays of trimmed rows are deep-equal.
   - `the table names Cubase as supported on both platforms`: rows whose
     Host is `Cubase` exist for Platform `macOS` and `Windows`, each with
     Status `supported` and a non-empty Version.
   - `every Status is supported or untested`: no other word appears in the
     Status column.
   - `neither file claims any VST3 host works`: neither file matches
     `/should work with any VST3/i` or `/any VST3-compatible host/i`.
2. Run it: the first, second and fourth tests fail on the current tree (no
   table in either file; the README's "should work" sentence). Record the
   failures.
3. Replace the README's `## DAW compatibility` body with the table and the
   sentence. Add `## Hosts` to chapter 17 before `## Per-Lane MIDI Channel`
   with the same table and sentence, introduced by one line saying Poly
   generates MIDI and needs a host that routes it to a drum instrument.
4. Run the test: green.
5. Write the evidence file with the two Cubase records, each with host,
   version, OS, routing steps and outcome:
   - Windows, Cubase 14: nightly run 36511596046 on 2026-09-29, green,
     which loads Poly in Cubase, plays the transport, toggles a step and
     validates the probe and the SMF export through loopMIDI — the routing
     the harness performs, with its spec names.
   - macOS, Cubase Pro 15: the owner's sessions; routing per the guide's
     MIDI Send option or a drum instrument listening on all channels;
     outcome: plays.
6. Run `format`, `doc-discipline`, `doc-conformance`, `site-unit`. Append the
   token lines to the evidence. Tick the box. Commit with `Rows: OS14`; OS14
   `done`.

## Task 2 — Chapter 17's routing starts with one drum instrument and keeps per-lane channels as the advanced case (OS15); the slice closes

**Files:** modify `site/src/content/docs/17-midi-routing-note-map.mdx`,
`site/tests/host-table.test.mjs`, `docs/plans/open-source-launch/ledger.md`,
the evidence file.

1. Add a test to `host-table.test.mjs`: `chapter 17 opens its routing with
   one instrument on all channels, before any per-lane channel setup`:
   the heading `### Start Here: One Drum Instrument` appears, it precedes
   `### Advanced: One Lane, One Channel`, and the section between them
   matches `/all (MIDI )?channels/i` and does not tell the reader to set
   lanes to Channel 1 (`/set every lane.*channel 1/i` absent).
2. Run it: red (neither heading exists).
3. Restructure the `## Per-Lane MIDI Channel` section of chapter 17:
   - Keep the opening explanation of Auto and Channel 1-16 and the wire-value
     paragraph as they are.
   - Replace `### Single-Instrument Setup (Default)` with
     `### Start Here: One Drum Instrument`: with every lane on Auto, Poly
     sends each lane on its own channel; a drum instrument whose MIDI input
     is set to Poly on all channels (Cubase's default for a new instrument
     track) receives every lane and plays the kick, snare and hats on their
     GM notes; nothing needs changing. The Cubase steps: add an Instrument
     track with Poly; add an Instrument track with the drum instrument; set
     its MIDI input to Poly and leave the channel on Any; press play. Then
     the MIDI Send alternative in one sentence, pointing at the Using Poly
     guide.
   - Rename `### Multi-Instrument Setup` to `### Advanced: One Lane, One
     Channel`, keep its body and the PolyPatch table, and fold the existing
     `### DAW Routing in Cubase` steps under it as the per-lane channel
     filtering setup, with step 1 rewritten so it no longer reads as the
     first thing a reader must do.
   - Remove the sentence "Set every lane's MIDI channel to 1, or leave them
     on Auto if only Channel 1 matters to your instrument."
4. Run the test: green. Run `bash scripts/check-doc-conformance.sh`: the
   chapter's existing claim markers still resolve (the `[verified: …]`
   attestation for note 76 that `scope-framing.test.mjs` names is
   unaffected; do not move it).
5. Run `format`, `doc-discipline`, `doc-conformance`, `site-unit`. Tick
   every DoD box here and in the ledger; set the slice `done`; OS15 `done`.
   Append evidence: the decision, the restructure, the DoD-to-task table.
   Run `jk-standards ledger`. Commit with `Rows: OS15`.

## Self-review

| DoD | Task |
|---|---|
| Cubase loaded, routed and played on both OSes with version and steps recorded | 1 — evidence from the nightly (Windows) and the owner (macOS) |
| README and chapter 17 carry the same table, Cubase supported, others untested, no "should work" | 1 — the table and the test that holds them equal |
| The default channel layout is decided against what Cubase did | 2 — kept; chapter 17 opens with the case where it already works |

| Row | Task | Verification produced |
|---|---|---|
| OS14 | 1 | evidence per platform; both files name Cubase and no host the evidence lacks; `should work` gone, held by a test |
| OS15 | 2 | decided and recorded; no default change so no migration test; the guide leads with the default |

Names used throughout: `host-table.test.mjs`, `### Start Here: One Drum
Instrument`, `### Advanced: One Lane, One Channel`. No placeholders.
