---
class: gated
---

# M004 — Decisions

Append-only. One entry per decision, with the reason.

## 2026-09-16 — planning M004 (all seven slices, front-loaded)

**The constraint that shapes every answer below.** Each slice's definition of
done requires a named nightly run with its spec green. The nightly runs on one
self-hosted Windows runner (`JIMW1`), under a repo-wide global lock, and each
dispatch takes over that machine's interactive desktop for roughly eight to
fifteen minutes. `JIMW1` is online and the last four scheduled runs are green;
[#267](https://github.com/JimAKennedy/poly/issues/267) is still open but its
failures stopped on 2026-09-13.

- **Q:** How should the nightly runs each slice owes be obtained? — **A:** Batch:
  build several specs, then one dispatch.
- **Decision:** Specs are built in groups and one dispatched run is cited by
  every slice in that group. — **Why:** One run genuinely shows several specs
  green, so citing it for each is a record rather than a shortcut. The
  alternative costs roughly fourteen serialised desktop takeovers.
- **Decision:** The groups are chosen by what each spec needs from the workflow,
  not by slice number. — **Why:** Four of the seven fit inside the existing
  single-launch flow; three need the workflow to launch Cubase twice, load a
  second instance, or bounce offline. Mixing them would make a green run
  ambiguous about which change caused a failure.

  | Group | Slices | What the workflow needs |
  |---|---|---|
  | 1 | S02, S03, S04, S07 | nothing new — they run inside the existing launch |
  | 2 | S01, S05, S06 | a second launch, a two-instance fixture, an offline bounce |

- **Q:** How should each spec be shown to fail? — **A:** An env-gated mutation
  the workflow can set.
- **Decision:** Each spec reads `POLY_E2E_MUTATE`, a single greppable variable
  naming one perturbation, and one extra dispatch with it set exercises every
  red path at once. — **Why:** It follows this repo's escape-hatch discipline —
  in-band, greppable, one token — and it makes the red path a thing that can be
  re-run on demand rather than a one-off someone did by hand and described. The
  rejected alternative, perturbing real code on a scratch commit per slice,
  doubles the dispatch count and leaves commits to clean up.
- **Decision:** The mutation branch lives in the spec, never in
  `plugin/source/` or `engine/`. — **Why:** A knob that changes shipped
  behaviour under an environment variable is a knob that can be set in
  production. The specs perturb what they *assert against* — a saved state file,
  a preset index, a captured stream — not what the plugin does.

- **Q:** How much should this run attempt? — **A:** All seven, batched.
- **Decision:** Plan all seven now, build in the two groups above, dispatch
  after each group plus one red-path run. — **Why:** It completes the milestone
  and the programme. The cost is stated rather than discovered: the user's
  machine is busy in bursts, and a spec that fails on the runner cannot be
  iterated locally — each fix costs another dispatch.

- **Deferred, to the boundary of group 1's dispatch:** whether a failing spec in
  that first run is fixed and re-dispatched, or the slice is left open and
  reported. That depends on what fails and cannot be answered before it does.
  Reaching that boundary with a red run is a planned pause, not a failure.

- **Judgment call:** `DAW01`'s row text says "`kStateVersion` is at 16". It is
  at 20 as of M003/S01. The row is not edited — it records what was true when
  the ledger was written, and its point (nothing exercises the round-trip in a
  host) still stands. The spec asserts against the current version rather than
  the number in the prose.

## 2026-09-16 — executing M004/S02 task 1 (judgment call on dispatch order)

- **Decision:** Dispatch a nightly after the *first* group-1 spec rather than
  after all four. — **Why:** The three remaining group-1 specs reuse this one's
  CDP attach, its menu route, and its `.strip .stat` read-back. If any of those
  is wrong on the runner, building three more on top multiplies one error into
  four, and each correction still costs a dispatch. Proving the route once is
  cheaper than proving it four times. This refines the batching decision rather
  than reversing it: the group is still cited from one *closing* run.

## 2026-09-16 — executing M004/S02 (a finding from the first dispatch)

- **Finding:** Run
  [35045855242](https://github.com/JimAKennedy/poly/actions/runs/35045855242).
  The preset sweep itself **passed** — all 45 presets selected in a live Cubase,
  no crash, every one matching `presets.json` — and it turned the *export* spec
  red: `track 1 name 'Clap' is not in the expected lane names ['Hi-Hat', 'Kick',
  'Snare', 'Tom']`. The sweep leaves the plugin on the last preset, not the
  fixture's patch.
- **Decision:** The sweep runs last while Cubase is up — after `Play scenario`,
  before `Quit Cubase` — and the workflow says why in both directions. — **Why:**
  It is destructive to host state, so anything downstream that depends on the
  fixture's patch must precede it. It cannot move past the quit either, because
  it attaches over CDP to a live editor, and it cannot precede `Play scenario`
  because that step's output is compared against a golden. There is exactly one
  correct position and the comment now records the two constraints that fix it.
- **Judgment call rather than a halt:** the deferred question was whether a
  failing *spec* is fixed and re-dispatched. This spec passed; a neighbour broke
  on ordering, and leaving the nightly red was never an option. Reordering a
  step I added this session to restore a green board is obviously right.
- **A rule this establishes for S01, S05 and S06.** Each of those also mutates
  host state — reopening a project, loading a second instance, bouncing. The
  ordering constraint found here applies to them, and the group-2 plans should
  place them relative to this sweep rather than discovering the same failure
  three more times.

## 2026-09-16 — executing M004/S02 (the knob could not do what was promised)

- **Finding:** `M004-decisions.md` stated that one extra dispatch with
  `POLY_E2E_MUTATE` set "exercises every red path at once". It could not:
  `applyMutation` compared the variable for equality against a single name, and
  the workflow input took one value, so a red dispatch could only ever turn one
  spec red and would leave the other six green — in a run whose entire purpose
  is to be red.
- **Decision:** The knob is a **list**, comma or space separated, read through a
  shared `mutationActive()` so the seven specs cannot drift on how it parses. —
  **Why:** Caught before three more specs were written against the broken
  convention, which would have made it four places to fix instead of one. Four
  unit cases pin the parsing, including that a substring is not a match — `s02`
  must not activate `s02-malformed-preset`.

## 2026-09-16 — a halt, and what it changed

- **Finding:** Section 2 asked about batching, red paths and scope, and assumed
  the binding constraint was DAW *time*. It is not. For five of the seven slices
  the constraint is that the harness cannot express the slice at all:

  | Slice | Missing affordance |
  |---|---|
  | S01 | nothing makes Cubase **save** a project |
  | S04 | nothing **closes and reopens** the plugin window — only enumeration exists (`diagnose-editor-window.ps1`) |
  | S06 | nothing drives **Export Audio Mixdown** |
  | S07 | an **automation lane** must be in a fixture, and fixtures are authored by hand |
  | S05 | needs a **two-instance `.cpr`**, and `poly-4bar.cpr` was "authored on the runner in Cubase, PR #186 — a `.cpr` is Cubase-version-specific" |

  Each of S01, S04 and S06 would mean writing Cubase UI automation blind, with
  an eight-minute dispatch as the feedback loop, on a machine whose desktop the
  run takes over.

- **Q:** S05 needs a two-instance `.cpr` that this session cannot create. Author
  one, or mark the slice `accepted` with the reason? — **A:** Author a
  two-instance `.cpr` on the runner.
- **Decision:** S05 stays `open`, blocked on that fixture, rather than being
  closed with a reason. — **Why:** The slice is wanted; only its input is
  missing. Recording it as blocked keeps the row honest and keeps the work
  visible, where `accepted` would retire it.

- **Q:** Should the UI-automation slices be built blind at eight minutes a
  cycle? — **A:** No — build them at the runner machine.
- **Decision:** S01, S04, S06 and S07 stay `open` with their plans intact, and
  the affordance each needs is named above so the next run starts from "build
  this" rather than rediscovering the wall. — **Why:** Writing `SendKeys`-style
  menu automation without being able to see the screen is where this milestone
  stops being good value. Someone at the machine closes that loop in minutes.

- **Q:** Spend one dispatch to close S02 properly? — **A:** Yes.
- **Decision:** Done — green run 35046414001, red run 35048220827. S02 is the
  one slice this session can honestly finish, and it is finished.

- **What S03 is, in this light.** Transport motion needs new MIDI Remote CC
  bindings for cycle and tempo; `CC_LOCATE` already binds to `To Left Locator`.
  That is code, not UI automation, so it is buildable from here — but its
  feedback loop is still a dispatch, and it is left `open` with the rest rather
  than started and abandoned mid-slice.

## 2026-09-17 — executing M004/S03 (a finding from the first dispatch)

- **Finding:** [Run 35258279739](https://github.com/JimAKennedy/poly/actions/runs/35258279739).
  The locate fired correctly and the golden comparison failed:
  `probe=45 golden=94`. Locating four seconds into an eight-second passage cut
  the first pass in half, and `--first-pass-only` then handed the golden half a
  performance.
- **Decision:** The replay fires only **after** the pass completes, tail
  included, and the driver flag is `--replay-pass` rather than
  `--locate-after SECONDS`. — **Why:** The correct moment is defined by the
  passage, not by a number, and a number would silently drift from
  `TAIL_SECONDS`. The transport is still rolling through the tail, so this is
  still a locate mid-playback — it just no longer truncates the passage the
  golden describes.

## 2026-10-07 — `/jk:auto M004`, resuming at the runner

**What changed since the halt.** The session now runs *on* `JIMW1`, in the
interactive console session the runner uses, and can screenshot the desktop.
The halt's second answer — "build them at the runner machine" — is therefore
satisfiable by this session: UI automation is checked against a screenshot in
seconds rather than against an eight-minute dispatch.

- **Q:** Driving Cubase takes over the shared desktop, and the runner listener
  can start a job mid-session. How should the desktop be handled? — **A:** Take
  it, and pause the runner.
- **Decision:** The `Runner.Listener` is stopped while this session drives
  Cubase, and restarted before each dispatch this session triggers; the owner
  stays off the keyboard while Cubase is foregrounded. — **Why:** A dispatch
  landing on a desktop someone else is automating would fail for reasons that
  have nothing to do with the spec, and keystrokes sent to the wrong window are
  not recoverable.

- **Q:** S05's two-instance `.cpr` — built by this session driving Cubase, or
  by the owner? — **A:** This session builds it.
- **Decision:** Derived from `poly-4bar.cpr` with a second Poly instance on its
  own track, saved under a new name, and reviewed in the PR. — **Why:** It
  supersedes the 2026-09-16 answer only in who does the authoring; the fixture
  is still authored on the runner in Cubase 14, which is the constraint the
  fixture README records.

- **The deferred question, answered up front.** **Q:** If a slice's spec is red
  in its group's dispatch, fix and re-dispatch, or stop? — **A:** Fix and
  re-dispatch, at most twice per group.
- **Decision:** A red spec is fixed locally against Cubase and the group is
  re-dispatched; a group still red after two re-dispatches halts the run with
  the failing log's headline. — **Why:** Local iteration is now cheap, so the
  dispatch is confirmation rather than discovery; the cap keeps a runner-only
  failure from consuming the machine indefinitely.

## 2026-10-07 — building S01, S04, S05, S06, S07 at the runner (judgment calls)

Each of these resolves something a plan left open or got slightly wrong, in a
way that is obviously right once seen on the runner. None widens a slice.

- **Every M004 spec runs in a Cubase session of its own.** The plans placed
  the new specs as steps inside the nightly's single session. Each of them
  changes host state a later spec would inherit -- an editor cycled, an
  automation lane written, a project saved and reopened, a bounce rendered, a
  different fixture -- which is the ordering trap 2026-09-16's S02 finding
  named. `scripts/cubase/run-session-spec.ps1` starts a fresh Cubase on a
  scratch copy of the fixture, runs one spec and always quits. **Why:** it
  makes each slice's result independent of the others' and of the first
  session's golden comparison, and it is what the fixtures README already
  requires of any run that saves.
- **The host is driven through the MIDI Remote surface, not UI automation.**
  2026-09-16 recorded "nothing makes Cubase save / close and reopen the editor /
  export a mixdown / write an automation lane". The surface already in the repo
  reaches all four: `makeCommandBinding` (File > Save, File > Export Audio
  Mixdown), the instrument slot's `mEdit`, `mAutomationWrite`/`mAutomationRead`,
  and direct access to one parameter. Only two clicks remain -- the Export
  dialog's button and Cubase's moved-project prompts -- each in a script of its
  own (`export-audio-mixdown.ps1`, `dismiss-moved-project.ps1`). **Why:** a CC
  is deterministic where a click is not, and the CC map is unit-testable.
- **S01's reopen is a second session on the saved copy, not a relaunch of the
  fixture.** The plan said "relaunch Cubase on the same fixture"; the fixture
  must never be written, so the save goes to a scratch copy and the reopen
  opens that copy. **Why:** the fixtures README's read-only rule.
- **S06 bounces with Export Audio Mixdown, not `export-midi.spec.ts`'s route.**
  The plan said to reuse that route. It is Poly's own SMF export, rendered from
  the engine outside the host, so it never drives `process()` offline -- the
  thing the row is about. The audio mixdown does, and the probe captures it.
  **Why:** the plan's route cannot test the row; this one does, and the spec
  says so in its header.
- **S07's automation driver is `remote.py automate`, not an extension of
  `play_scenario.py`.** Same directory, same constants and handshake, imported
  from `play_scenario.py`; the transport driver stays byte-for-byte what the
  first session runs. **Why:** the nightly's golden comparison depends on
  `play_scenario.py`, and nothing about automation needs to touch it.
- **S05 changes `poly_midi_probe` so two probes write two files.** The probe
  read one path from `POLY_PROBE_OUTPUT`, so two instances in one Cubase
  overwrote each other. Each instance now claims the lowest free slot; slot 0
  writes the configured path exactly as before, slot 1 writes `probe-2.jsonl`.
  The probe is test tooling under `tools/midi_probe/`, not the shipped plugin.
  **Why:** without it S05's output isolation is unobservable; with slot 0
  unchanged, every existing fixture and step is unaffected.
- **S05's fixture is authored from the runner checkout's `poly-4bar.cpr`.** A
  first version authored from a scratch copy embedded that scratch folder's
  path, which the personal-paths guard rejected and which made Cubase raise a
  "Set Project Folder" picker for its copies. Re-authored by opening the runner
  checkout's fixture in place and saving beside it, so the only path it records
  is the one `poly-4bar.cpr` records. `dismiss-moved-project.ps1` answers the
  picker too, in case a future fixture raises it.
- **The job timeout goes from 30 to 45 minutes.** Six more Cubase launches at
  about a minute and a half each on a run that took seven. **Why:** the ceiling
  exists to bound a hang, not to fail a healthy run.
- **One green dispatch and one red dispatch cover all five slices.** The
  batching decision grouped the slices by what the workflow needed; built at
  the runner, all five landed together, so the groups collapse into one
  closing run. **Why:** the decision's reason -- one run genuinely shows
  several specs green -- holds for five as for four.
