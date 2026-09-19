---
class: gated
---

# M004 — decisions

Append-only. Every question asked at planning, its answer, and every choice
made on the user's behalf during execution.

## 2026-09-19 — planning M004/S01 and M004/S02

- **Q:** Six of the seven files are repo docs read on GitHub, not site pages.
  GitHub renders ```mermaid fences natively at view time. How should those be
  handled? — **A:** Fences in `.md`, build-time rendering for the site only.
- **Decision:** `ARCHITECTURE.md` and the five `docs/*.md` files carry
  ```mermaid fences and rely on GitHub's own renderer; the build-time pipeline
  covers only `appendix-plugin-architecture.mdx` — **Why:** it is the one file
  Astro owns, and a build step for files nobody builds would be machinery with
  no reader.
- **Consequence recorded rather than hidden:** the milestone's Vision says
  "rendered at build time". That holds literally for the site page only; for the
  six repo docs, rendering happens at view time in GitHub. The milestone states
  this rather than letting the Vision read as covering all seven.

- **Q:** S01's definition of done requires byte-identical output across two
  builds. How should that be achieved? — **A:** Render at build, pin and
  normalise; commit nothing.
- **Decision:** no rendered artifact is committed; the test renders twice at
  test time and compares — **Why:** a committed SVG would inherit the
  reproducibility problem `#282` records for the WASM artifacts, which
  `CLAUDE.md` already documents as live where the `e2e` gate rewrites
  `webui/poly_engine.{js,wasm}`.

- **Q:** The ledger says 108 lines; its own per-file figures sum to 116, and the
  tree measures 117 today. S02's DoD asserts the 108. — **A:** Restate the DoD
  as a property.
- **Decision:** S02's first DoD item becomes "no box-drawing characters remain
  in the seven files, proved by the guard"; the measured inventory stays in the
  milestone's prose, corrected and dated — **Why:** a count inside a definition
  of done goes stale the moment anyone edits a diagram; the property does not.

- **Q:** `docs/audits/M001-theory-audit-remediation-plan.md` has 5 box-drawing
  characters and sits outside the seven, in a non-exempt directory. What should
  the guard do about it? — **A:** Guard all governed docs; escape-hatch the
  audit file.
- **Decision:** the guard covers every governed doc, and the audit file carries
  an in-band `boxdraw-ok:` marker with its reason — **Why:** narrowing the guard
  to the seven files would leave it structurally unable to fire on a *new* doc
  adding ASCII art, which is the drift it exists to prevent. The marker follows
  the repo's escape-hatch discipline: in-band, greppable, reasoned.

- **Q:** Approve the M004/S01 design as written, or change the rendering
  strategy? — **A:** Approve — `inline-svg`.
- **Decision:** `strategy: 'inline-svg'`, design at `M004-S01-design.md`
  — **Why:** site CSS reaches an inline SVG, which is what lets the diagram
  match the guide's typography as the milestone's Demo line requires.

### Found at design time, not asked

- **`deterministicIds` is inert.** The design was going to set
  `mermaidConfig: { deterministicIds: true }` and name it the reason builds
  reproduce. Rendering with and without the flag produced identical bytes and
  identical `id="mermaid-0"`: `rehype-mermaid` renders each diagram in a fresh
  page context, so the counter starts at zero either way. The flag is omitted
  and the test asserts the property instead. Recorded because the plausible
  version of this design contains a documented falsehood.
- **Byte-identity does not hold across machines.** The SVG embeds
  browser-computed geometry from font metrics (`viewBox="0 0 421.953125 70"`).
  Two builds on one machine agree; two machines need not. No fixture is
  committed and none is compared, so the DoD's claim is the one actually tested.
- **The Pages deploy would have broken after merge.** `deploy-site.yml`'s build
  job runs `npx astro build` with no Playwright browser installed, while
  `site-lint` never builds the site and `site-e2e` already installs Chromium.
  Every PR check would have stayed green and Pages would have failed on merge.
  S01 adds the cached browser-install step to that job.

## 2026-09-19 — executing M004/S01 task 1

- **Q:** The approved design said "add a rehype plugin". The reality is bigger:
  Astro 7 defaults to the Sätteri processor, and `markdown.rehypePlugins`
  requires reinstalling `@astrojs/markdown-remark`, which swaps the processor
  for all 47 pages. Accept? — **A:** Accept the swap, pin the setting.
- **Decision:** `@astrojs/markdown-remark` is a devDependency and
  `markdown.smartypants = { dashes: 'oldschool' }` is pinned by a test
  — **Why:** measured, not assumed: 43 of 49 built pages differed in bytes, but
  once entity style (`&amp;` vs `&#x26;`), `<path/>` vs `<path></path>` and
  inter-tag whitespace were normalised, only **4** differed in visible text, and
  3 of those were `--` widening from an en dash to an em dash. `oldschool` is
  the SmartyPants convention that keeps `--` an en dash, which is the correct
  glyph in a range like `0.0--1.0`.
- **Proved non-vacuous:** removing the `dashes` line fails
  `the markdown processor keeps \`--\` an en dash`; restoring it by an explicit
  edit passes. 298 → 299 tests.

- **Q:** `theory-electronic-breakbeat` writes `("&"s)` for the counting
  syllable. Sätteri rendered `“&“s`, remark renders `”&“s`; both are wrong.
  — **A:** Fix the source to explicit curly quotes.
- **Decision:** the source now reads `(“&”s)` — **Why:** it corrects an error
  that pre-dates this milestone and makes the passage processor-independent.
  After the fix, exactly one page differs from the pre-change baseline in
  visible text, and that difference *is* the correction.

### Judgment call — `site/src/generated/counts.json`

The build regenerated `counts.json` from **43 presets to 45**. The committed
file was stale; the engine has shipped 45 since before this milestone, and
`presets.json` already carried 45. Nothing cross-checks the two, which is why
`site-unit` was green with them disagreeing.

Folded into task 1's commit rather than restored, because restoring it re-dirties
the tree on every `npm run build` — the trap `CLAUDE.md` documents where a
generated artifact and a formatting gate fight each other. The live site was
never wrong: `npm run build` regenerates counts before `astro build`, so Pages
rendered 45 throughout; only the committed artifact and anything reading it were
stale.

**Not fixed here, and worth its own row:** no guard cross-checks
`counts.json.presets` against `presets.json`. This staleness survived from
`50d387e` (M048 S06) undetected, which is the same class as the stale-presets
bug M005/S01 fixed at source.
