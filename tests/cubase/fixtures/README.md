---
class: gated
---

# Cubase Test Fixtures

This directory holds the committed Cubase project fixture(s) that the
Cubase-in-the-loop nightly (`.github/workflows/cubase-nightly.yml`) opens to
drive a real transport run. The launch script
`scripts/cubase/launch-cubase.ps1` opens the fixture via its `-FixtureCpr`
parameter, and the workflow points `POLY_FIXTURE_CPR` at the path below.

## The fixture

| File | Purpose |
|---|---|
| `poly-4bar.cpr` | One instrument track hosting Poly, with `poly_midi_probe` inserted downstream so it captures Poly's MIDI output. Opened by the nightly to play a fixed 4-bar scenario and dump probe JSONL. |

`poly-4bar.cpr` is a **binary** Cubase project. It is authored **once, on the
runner, inside Cubase** (Cubase is the only tool that can write a `.cpr`) by
following the recipe below, then committed. Everything else in this directory —
this recipe, the directory itself — is authored on the dev machine.

### Why the `.cpr` is not authored on the dev machine

A `.cpr` is a Cubase-version-specific binary (`docs/testing-strategy.md` §3.1):
a project saved by Cubase 14 is the contract for the Cubase 14 runner, and
there is no supported non-Cubase way to synthesize one. The dev-machine half of
this slice is therefore the **recipe**; the binary is a runner-produced artifact
that lands here after the owner follows the recipe.

### The fixture is read-only at run time

The nightly opens the fixture but must never save over it — a run mutating the
committed `.cpr` would make the fixture drift silently. Keep the fixture track
un-armed for record and do not enable auto-save for the fixture project. If a
run needs scratch state, it lives in `_artifacts/`, not here.

## Authoring recipe (runner, in Cubase 14)

Perform once on the `cubase`-labelled runner. Prerequisite: Poly and
`poly_midi_probe` are both built and installed for Cubase to load (the nightly's
`Install plugin for Cubase` step does this; for manual authoring, install both
`.vst3` bundles into `C:\Program Files\Common Files\VST3`).

1. **New empty project.** Cubase → new empty project at 120 BPM, 4/4. Tempo and
   time signature must match the golden the comparison harness diffs against
   (`tests/golden/processor_default_4bars.txt` is `tempo=120`, `4/4`).
2. **Add an instrument track hosting Poly.** Project → Add Track → Instrument →
   Poly. Leave Poly on its **default patch** — the committed golden is the
   default patch, so the fixture must use it for the probe-vs-golden diff to
   line up. Do not change lanes, seed, or macros.
3. **Add a second instrument track hosting `Poly MIDI Probe`.** The probe is a
   VST3 **instrument** (`kInstrumentSynth`, name "Poly MIDI Probe"): Cubase feeds
   an instrument the track's MIDI as `data.inputEvents`, which is exactly the
   note stream the probe captures. It is **not** a MIDI insert — a `kFxAnalyzer`
   never appears in Cubase's MIDI Inserts list, which is why the probe is
   registered as an instrument instead. Project → Add Track → Instrument → Poly
   MIDI Probe. The probe emits only silence; it produces no sound.
4. **Route Poly's MIDI output into the probe track.** Select the probe
   instrument track and set its **MIDI input** to Poly's output (in Cubase 14:
   the probe track's Input Routing → the Poly track / Poly's MIDI out), so Poly's
   generated note-ons/note-offs flow into the probe's event input. This is the
   standard "one track feeds another instrument its MIDI" wiring — no MIDI-insert
   routing is involved. The probe then sees the same event stream the golden was
   generated from. (The transport-driving `poly-test` loopMIDI port is unrelated
   to this: that port carries START/STOP/ready CCs between the Python driver and
   the MIDI Remote surface, not Poly's captured notes. Nothing needs to be routed
   back out to `poly-test`.)
5. **Set the play range to the scenario length.** The driver plays a fixed
   number of bars from bar 1 (default 4, matching the 4-bar golden); set the
   left locator to bar 1 and the right locator to the end of the scenario. The
   scenario length is a driver parameter (`tests/cubase/driver/play_scenario.py`
   `--bars`); keep the fixture's arrangement at least that long.
6. **Confirm the probe output path.** The probe reads `POLY_PROBE_OUTPUT` from
   the environment and writes JSONL there from within `process()` during
   playback (so the file lands before the runner hard-kills Cubase) — no
   per-project configuration is needed. The nightly sets that env var; nothing
   about it is stored in the `.cpr`.
7. **Save as `poly-4bar.cpr`** in this directory
   (`tests/cubase/fixtures/poly-4bar.cpr`), then commit it. The path must match
   `POLY_FIXTURE_CPR` in the workflow and `launch-cubase.ps1`'s `-FixtureCpr`
   argument.

## Regenerating the fixture

Re-run the recipe above whenever the fixture must change (new preset, different
scenario length, a Cubase major-version bump that changes the `.cpr` format).
Because the `.cpr` is version-specific, a Cubase upgrade on the runner requires
re-saving the fixture from the new version — an old-version `.cpr` opened in a
newer Cubase may migrate silently and is not a trustworthy fixture.

## Cross-references

- `scripts/cubase/launch-cubase.ps1` — opens the fixture (`-FixtureCpr`).
- `.github/workflows/cubase-nightly.yml` — sets `POLY_FIXTURE_CPR` to this path.
- `tests/cubase/driver/README.md` — the driver that plays the scenario.
- `tests/cubase/compare_probe_golden.py` — diffs the resulting probe JSONL
  against `tests/golden/processor_default_4bars.txt`.
- `docs/cubase-workflow.md` — the fixture section that frames this for readers.
