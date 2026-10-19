---
class: gated
---

# M006/S07 — The banner does not claim the guide is finished

**Slice:** central-africa M006/S07 (row CA51)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M006-decisions.md` (2026-10-18)

## Task status

- [ ] 1. The banner says the theory is still being revised

## Definition of Done

Copied verbatim from the slice:

- [ ] The site banner no longer says the guide is complete and verified, and
      tells the reader that its music theory is still being revised and to
      check the references

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## Task 1 — The banner says the theory is still being revised

Rows: CA51.

**Files**

- Modify `site/src/components/Banner.astro`
- Modify `site/tests/front-door.test.mjs`
- Create `docs/plans/central-africa/evidence/M006-S07.md`
- Modify this plan and the ledger

**Steps**

1. In `front-door.test.mjs`, beside the OS26 banner assertions, add a case
   asserting `Banner.astro` contains the three sentences the owner gave,
   word for word: "The music theory in the guide is constantly being
   revised, corrected, and extended.", "It may contain errors from time to
   time." and "Always verify assumptions directly with the references.", and
   does not contain "complete and verified".
2. Run `node --test site/tests/front-door.test.mjs` and watch the new case
   fail.
3. In `Banner.astro`, replace "; the guide is complete and verified." with
   ". The music theory in the guide is constantly being revised, corrected,
   and extended. It may contain errors from time to time. Always verify
   assumptions directly with the references." Leave the pre-release text and
   the Releases link as they are.
4. Run the test and watch it pass, the OS26 assertions included. Mutate:
   restore the old clause, watch the case fail; restore. Record.
5. Run the three tokens. Tick the DoD box here and in the ledger, set CA51
   and the slice to `done`, run `jk-standards ledger`. Write the evidence.
   Tick this task. Commit with `Rows: CA51`.

## Self-review

- **DoD / rows.** CA51 → Task 1; red first in step 2; OS26 still passes in
  step 4.
- **Placeholders.** None.
