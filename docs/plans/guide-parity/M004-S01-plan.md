---
class: gated
---

# M004/S01 — One diagram renders from Mermaid at build time

**Slice:** guide-parity M004/S01 (row GP09)
**Ledger:** `docs/plans/guide-parity/ledger.md`
**Design:** `M004-S01-design.md` (approved 2026-09-19)

## Task status

- [x] 1. A test that demands byte-identical renders, and the wiring that makes it pass
- [x] 2. Give the diagram the site's typography
- [x] 3. Install Chromium in the Pages deploy build job
- [ ] 4. Convert one appendix diagram and remove its ASCII original
- [ ] 5. Evidence and slice close-out

## Definition of Done

Copied verbatim from the slice:

- [ ] A Mermaid source block in a site page renders to vector output at build time, not at page load
- [ ] The rendering is deterministic: an unchanged source produces byte-identical output across two builds
- [ ] One existing diagram is converted and renders correctly, with the ASCII original removed

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `guards` | `bash scripts/check-guards.sh` |

---

## Task 1 — A test that demands byte-identical renders, and the wiring that makes it pass

**Produces:** `site/tests/mermaid-render.test.mjs`, the test every later task in
this slice is measured against.

The test is written before the plugin is installed, so its first run fails
because there is nothing to import — that is the correct failure, and it is the
one to observe.

**Files:** create `site/tests/mermaid-render.test.mjs`.

**Steps:**

1. Write a test named `a mermaid block renders to inline SVG` that runs the
   site's own rehype pipeline over a fixed Mermaid source string and asserts the
   result contains `<svg` and does **not** contain `language-mermaid` — i.e. the
   fence was consumed, not passed through.
2. Write a second test named `two renders of one source produce identical bytes`
   that renders the same source twice **in separate child processes** via
   `node:child_process`, and asserts the two stdout strings are equal. Separate
   processes matter: a same-process comparison cannot see state that persists
   across a process boundary, and "two builds" means two processes.
3. Write a third test named `no mermaid runtime reaches the browser` asserting
   the rendered output contains no `<script` tag.
4. Run `npm --prefix site test -- --test-name-pattern mermaid` and **watch all
   three fail** on the missing module. Record the failure text.

**Check:** `site-unit` fails, naming `mermaid-render.test.mjs`, for the missing
dependency — not for a typo in the test.

Having watched the three fail, add the dependency and the wiring **in this same
task**, so the commit lands green. The failure is observed, not committed: the
`/jk:next` rule that no commit may carry a red gate outranks the tidier
one-concern-per-commit split this plan originally had. The evidence file records
the observed failure text, which is where that record belongs.

**Files (wiring):** modify `site/package.json`, `site/package-lock.json`,
`site/astro.config.mjs`; the test file from task 1 gains its import path.

**Steps:**

1. `npm --prefix site install --save-dev rehype-mermaid@3.0.0 playwright@1`.
   `playwright` is a **peer** dependency of `rehype-mermaid` and is a different
   package from the `@playwright/test` the site already has; both are needed.
2. In `site/astro.config.mjs`, import `rehypeMermaid` and add to the
   `defineConfig` object:
   ```js
   markdown: { rehypePlugins: [[rehypeMermaid, { strategy: 'inline-svg' }]] },
   ```
   Place it beside the existing `integrations` key, inside the same
   `defineConfig` call. Do not place it inside the `starlight(...)` options —
   Starlight passes markdown config through Astro, not through its own options.
3. Put the plugin options in `site/src/lib/mermaid-config.mjs`, exporting a
   single `mermaidRehypeOptions`, and have `astro.config.mjs` import it. The
   test imports the same export, so it exercises **the options the build
   actually uses** rather than a copy that can drift. Step 2's inline object
   becomes that import.
4. Run `npm --prefix site test -- --test-name-pattern mermaid` and watch all
   three pass.
5. Run the full `site-unit` to confirm nothing else regressed.

**Check:** `site-unit` passes, including the three new tests.

**Note for the executor:** `rehype-mermaid` needs a Chromium binary matching the
installed Playwright version. If the render throws `Executable doesn't exist`,
run `npx --prefix site playwright install chromium` — this is a local machine
state problem, not a defect in the wiring.

---

## Task 2 — Give the diagram the site's typography

**Consumes:** task 1's wiring. **Produces:** diagrams in the guide's sans face.

Default mermaid output sets `font-family:arial,sans-serif`, which is none of the
site's three faces. The milestone's Demo requires diagrams that match the site's
typography.

**Files:** modify `site/src/lib/mermaid-config.mjs`; modify
`site/tests/mermaid-render.test.mjs`.

**Steps:**

1. Add a test named `rendered diagrams use the site's sans face` asserting the
   rendered SVG contains the site's sans stack and does **not** contain
   `arial,sans-serif`. Run it and watch it fail — the current output has Arial.
2. Read the sans stack from `site/src/styles/custom.css` rather than retyping
   it; if it is only expressible as a literal, put the literal in one place that
   both the config and the test import.
3. Set `mermaidConfig: { fontFamily: <that stack> }` in
   `mermaidRehypeOptions`.
4. Run the test and watch it pass.
5. Re-run the identical-bytes test from task 1. It must still pass: changing the
   font changes the geometry numbers, but not their stability.

**Check:** `site-unit` passes, including the typography test and the unchanged
determinism test.

---

## Task 3 — Install Chromium in the Pages deploy build job

**Consumes:** nothing from earlier tasks. **Produces:** a deploy job that can
build a site containing a Mermaid diagram.

`deploy-site.yml`'s `build` job runs `npx astro build` with no Playwright
browser installed. `site-lint` never builds the site; `site-e2e` already
installs Chromium. Without this task every PR check stays green and Pages breaks
**after** merge.

**Files:** modify `.github/workflows/deploy-site.yml`.

**Steps:**

1. In the `build` job, after `Install dependencies` and before `Build`, add the
   three steps `site-e2e` already uses, in the same order and with the same
   pinned action SHAs: get the Playwright version, cache `~/.cache/ms-playwright`
   keyed on it, then `npx playwright install --with-deps chromium`. Copy the SHAs
   from the `site-e2e` job in `ci.yml` rather than writing new ones.
2. Add a comment above the block saying why a docs build needs a browser:
   `rehype-mermaid` renders diagrams through Chromium at build time.
3. Run `bash scripts/check-guards.sh` — it runs the workflow guards.
4. Confirm by reading that `scripts/check-release-workflow.mjs` targets
   `release.yml` only, so no locked workflow contract is affected by this edit.

**Check:** `guards` passes.

**This task cannot be proved locally beyond the guards** — the deploy job runs
only on `main`. Say so in the evidence rather than claiming a green deploy.

---

## Task 4 — Convert one appendix diagram and remove its ASCII original

**Consumes:** tasks 1 and 2. **Produces:** the DoD's "one existing diagram is
converted".

**Files:** modify `site/src/content/docs/appendix-plugin-architecture.mdx`;
modify `site/tests/mermaid-render.test.mjs`.

**Steps:**

1. Read `appendix-plugin-architecture.mdx` and pick **one** diagram — the first
   fenced block containing box-drawing characters. Record which, by its heading,
   in the evidence.
2. Add a test named `the plugin-architecture appendix has a mermaid diagram and
   no ascii art in that section` that reads the `.mdx` file and asserts it
   contains a ```mermaid fence, and that the converted section contains no
   box-drawing characters. Run it and watch it fail.
3. Replace that one ASCII block with a ```mermaid fence expressing the same
   relationships. Preserve every label and every arrow direction — this is a
   transcription, not a redesign. If the ASCII is ambiguous about a relationship,
   **stop and ask** rather than inventing one.
4. Run the test and watch it pass.
5. Build the site once (`npm --prefix site run build`) and confirm it succeeds
   with the diagram present. Record the build time delta if it is noticeable.
6. Run the full validation set.

**Check:** `site-unit`, `doc-conformance`, `doc-discipline`, `guards`, `format`
all pass, and `npm --prefix site run build` succeeds.

**Leave the other diagrams in this file alone.** They are S02's work, and
converting them here would widen the slice.

---

## Task 5 — Evidence and slice close-out

**Files:** create `docs/plans/guide-parity/evidence/M004-S01.md`; modify the
ledger.

**Steps:**

1. Append one evidence entry per task: token, exit code, headline counts, date.
   Name no commit SHA — this file ships inside the commit it would describe.
2. Record the determinism measurement as a number, not an adjective: the two
   renders' byte lengths and whether they matched.
3. Record that task 3 is unproven locally and why.
4. Set row GP09 to `done` and slice M004/S01 to `done` in the ledger, and tick
   all three definition-of-done boxes.
5. Run `jk-standards ledger`, then the full validation set.

**Check:** `jk-standards ledger` passes with the slice `done` — which is the
stricter claim: every DoD box ticked, the evidence file on disk, the row closed.
