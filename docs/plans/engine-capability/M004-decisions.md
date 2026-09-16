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
