---
class: gated
---

# M006/S06 — Every card plays

**Slice:** central-africa M006/S06 (row CA50)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M006-decisions.md` (2026-10-18)

## Task status

- [x] 1. A spec plays every preview card in Try it

## Definition of Done

Copied verbatim from the slice:

- [x] A test opens every preset card on the published site at a laptop
      viewport, plays it, and fails if it does not sound, overflows, or
      throws

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `e2e` | `bash scripts/site-verify-local.sh` |

After every `e2e` run restore `webui/poly_engine.{js,wasm}` to the commit's
own copies (`git checkout HEAD -- webui/poly_engine.js webui/poly_engine.wasm`).

## Task 1 — A spec plays every preview card in Try it

Rows: CA50.

**Files**

- Create `site/tests-e2e/every-card-plays.spec.ts`
- Wire it into the specs `scripts/site-verify-local.sh` runs, if that script
  lists specs rather than running the directory
- Create `docs/plans/central-africa/evidence/M006-S06.md`
- Modify this plan and the ledger

**Steps**

1. Write the spec at viewport 1440×900. Derive the cards from the built
   site: read every `dist/**/index.html`, collect each element's
   `data-poly-preset` with its page path, and fail if fewer than 38 are
   found (the count on 2026-10-18), so a broken extraction is not a vacuous
   pass. For each card: open its page, open Try it on that card, click Play
   in the iframe, wait up to 5 s for `window.__polyAudioProbe` to report
   at least one scheduled note, assert no `.strip` in the iframe overflows
   (`scrollHeight <= clientHeight + 1`), and assert no console error or page
   error was recorded on page or iframe. Name the preset and page in every
   failure message.
2. Prove it can fail: point one card's lookup at a preset name that does not
   exist (temporarily, in the spec's own data, not the site) and watch that
   card fail; and temporarily restore `min-height: 6px` on `.ladder button`
   and watch the Aka cards fail on overflow; restore both. Record.
3. Run it on the real site and watch every card pass.
4. Run the three tokens. Tick the DoD box here and in the ledger, set CA50
   and the slice to `done`, run `jk-standards ledger`. Write the evidence.
   Tick this task. Commit with `Rows: CA50`.

## Self-review

- **DoD.** Every card, laptop viewport, sound, overflow, errors → Task 1.
- **Rows.** CA50: list derived from the pages (step 1), 1440×900, notes,
  overflow, console clean.
- **Placeholders.** None.
- **Names.** `every-card-plays.spec.ts`, `__polyAudioProbe`.
