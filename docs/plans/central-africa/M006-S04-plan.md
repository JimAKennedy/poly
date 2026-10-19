---
class: gated
---

# M006/S04 — The editor fits a laptop

**Slice:** central-africa M006/S04 (rows CA47–CA48)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M006-decisions.md` (2026-10-18)

## Task status

- [x] 1. A 24-step ladder stays inside its card
- [ ] 2. The Try it frame gives the editor its height

## Definition of Done

Copied verbatim from the slice:

- [ ] A 24-step lane's step stack stays inside its card at a laptop-sized
      window
- [ ] The Try it frame gives the editor the height it is laid out for, or
      the editor is laid out for the frame it gets

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `webui-e2e` | `npm --prefix webui test` |
| `e2e` | `bash scripts/site-verify-local.sh` |

After every `e2e` run restore `webui/poly_engine.{js,wasm}` with
`git checkout HEAD -- webui/poly_engine.js webui/poly_engine.wasm`.

## Task 1 — A 24-step ladder stays inside its card

Rows: CA47.

**Files**

- Modify `webui/ui.css`
- Create `webui/tests/laptop-layout.spec.mjs`
- Create `docs/plans/central-africa/evidence/M006-S04.md`

**Steps**

1. Write the spec at viewport 1200×520 on the WASM host with
   `preset=Aka%3A%20Ngbolu` (its third lane has 24 steps): for every
   `.strip`, the top edge of its `.ladder`'s highest button
   (`.ladder button:last-child`, the stack being `column-reverse`) is at or
   below the bottom edge of that strip's `svg.ring`, and the ladder's bounding
   box lies inside the strip's.
2. Run it and watch the 24-step strip fail: its bars spill upward over the
   ring.
3. In `ui.css`, change `.ladder button`'s `min-height: 6px` to `min-height:
   1px` and add `overflow: hidden` to `.ladder`.
4. Run and watch it pass, and run the existing layout specs
   (`expanded-lane-layout.spec.mjs`, `ui.spec.mjs`) at 1280×840 to confirm a
   16-step ladder still renders. Mutate: restore `min-height: 6px`, watch the
   case fail; restore. Record.
5. Run the three tokens. Write the evidence. Tick this task. Commit with
   `Rows: CA47`.

## Task 2 — The Try it frame gives the editor its height

Rows: CA48.

**Files**

- Modify `site/src/styles/custom.css`
- Create `site/tests-e2e/try-it-frame.spec.ts`
- Append to the evidence; modify this plan and the ledger

**Steps**

1. Write the spec, run by `e2e`: at viewport 1440×900, open
   `/02b-central-africa/`, open the Aka: Ngbolu card's Try it, and in the
   iframe assert no `.strip` overflows its own box (`scrollHeight <=
   clientHeight + 1`) and that each ladder's top button is below its ring,
   as in Task 1. Assert the frame's height is at least 760 px.
2. Run it (`npx --prefix site playwright test try-it-frame`, against the
   built site as `site-verify-local.sh` serves it) and watch it fail at
   560 px.
3. In `custom.css`, set `.poly-modal .poly-modal-frame` `height:
   min(760px, 92vh)`, and widen the modal's container so the frame is at
   least 1160 px wide where the viewport allows (`width: min(1200px,
   96vw)`), keeping its existing mobile rules. Let the frame scroll.
4. Run and watch it pass; re-run `control-audit.spec.ts`, whose layout
   probes cover mobile and tablet. Record.
5. Run the three tokens. Tick the DoD boxes here and in the ledger, set
   CA47–CA48 and the slice to `done`, run `jk-standards ledger`. Append the
   evidence. Tick this task. Commit with `Rows: CA48`.

## Self-review

- **DoD.** 24-step stack in its card → Task 1. Frame height → Task 2.
- **Rows.** CA47 → Task 1 at about 1200×520, red first. CA48 → Task 2 at
  1440×900.
- **Placeholders.** None.
- **Names.** `laptop-layout.spec.mjs`, `try-it-frame.spec.ts`.
