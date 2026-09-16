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
