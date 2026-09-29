---
class: gated
---

# What is in `docs/`

Status: current (2026-09-29). Kept complete by `scripts/check-docs-index.mjs`,
which fails when a top-level document or directory here is missing from this
page or when this page names one that is gone.

Most of this directory is delivery record: ledgers, plans, evidence and
reviews that say what was decided and what was proved, kept so a later reader
can trace any change back to its reason. A contributor needs only the second
section below. Users need none of it: the guide at
[poly.jk.digital](https://poly.jk.digital) is written for them.

## For users

Nothing here. Read the guide, and the [README](../README.md) for download and
DAW setup.

## For contributors

Read these before changing the engine, the plugin, the tests or the site:

- `engine-spec.md` — the engine's behaviour as specified: lanes, cycles,
  Euclidean patterns, envelopes, macros, scenes, state serialization.
- `euclidean-rhythm-guide.md` — the Euclidean algorithm as Poly implements it,
  with worked examples.
- `midi-note-mapping.md` — note numbers, channels, the Note Map, and the MIDI
  capture and export path.
- `preset-taxonomy.md` — how the factory presets are classified and what each
  category promises.
- `testing-strategy.md` — the test pyramid from engine unit tests to the
  Cubase nightly, and which gate proves what.
- `cubase-workflow.md` — driving Cubase by hand and from the harness: transport,
  routing, probe, export.
- `sample-sourcing.md` — where the site's audio samples come from and the
  manifest that governs them.
- `pr-af-review.md` — the opt-in prose review workflow and how to trigger it.
- `create-golden-disk-image.md`, `windows-test-runner-setup.md`,
  `windows-test-runner-setup-issues.md`,
  `windows-runner-rehome-and-deelevate.md` — building and operating the
  self-hosted Windows runner the Cubase nightly needs.

## Delivery records

Read these to learn why something is the way it is, not how to change it:

- `plans/` — the delivery ledgers, one directory per programme, with each
  milestone's plans, decisions, evidence and review-gate report. The file is
  the state; git is the history.
- `audits/` — dated audits and the remediation plans they produced.
- `reviews/` — dated external and internal reviews, frozen as written.
- `verification/` — captured outputs that verified a specific milestone.
- `internal/` — planning notes not addressed to contributors.
- `ui-guide.md`, `webui-migration.md`, `webui-drag-spike.md`,
  `webui-capture-timeline-uat.md` — archived: the native editor these describe
  was replaced by the WebView editor, and they are kept as the record of that
  migration.
