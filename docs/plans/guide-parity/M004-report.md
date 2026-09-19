---
class: gated
---

# M004 — Diagrams are rendered, not drawn

> Every architecture diagram is Mermaid source rendered at build time, and none
> is hand-maintained ASCII art.

Ledger: `docs/plans/guide-parity/ledger.md` · 17 commits, **none untraced**

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M004/S01 | One diagram renders from Mermaid at build time | GP09 | done |
| M004/S02 | All seven files convert, and ASCII cannot return | GP10, GP11 | done |

## What the milestone turned out to be about

The vision assumed 106 lines of architecture diagram. **32 of them were.** The
rest was other kinds of content that Mermaid is the wrong tool for:

| Treatment | Lines | Outcome |
|---|---|---|
| Converted to Mermaid | 32 | 5 fences across 4 files, each render-verified |
| Nested Markdown lists | 20 | 2 directory trees — no exemption needed |
| Exempted, untouched | 49 | 2 UI wireframes, incl. a 44-line plugin-window mockup |
| Exempted, frozen | 5 | a dated remediation record |

Forcing the wireframes into flowcharts would have destroyed useful documents to
satisfy a rule aimed at something else. That judgement, and the exemptions it
produced, is the main thing a reviewer should agree or disagree with.

**The vision's own wording does not hold literally, and the ledger now says so.**
Six of the seven files are `.md` read on GitHub, which renders fences at *view*
time, not build time. Only the site page is built.

## Definition of done

**M004/S01**

- [x] A Mermaid source block in a site page renders to vector output at build time, not at page load
- [x] The rendering is deterministic: an unchanged source produces byte-identical output across two builds
- [x] One existing diagram is converted and renders correctly, with the ASCII original removed

**M004/S02**

- [x] No ASCII architecture diagram remains in the seven files, replaced by Mermaid source; any file still carrying box-drawing characters does so under a stated, greppable exemption — all proved by the guard
- [x] A check fails when ASCII box-drawing characters appear in a diagram position in any governed doc
- [x] That check has been shown to fail by reintroducing one
- [x] Every Mermaid fence in a governed doc is proved to render, including the four in `.md` files that no build touches

## Validation

Every token both slices declare, green on the branch head:

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | pass |
| `site-unit` | `npm --prefix site test` | pass — 304 tests, 0 failures |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass |
| `guards` | `bash scripts/check-guards.sh` | pass — 14 invocations, 3 files exempted |

`jk-standards ledger` → 3 ledgers conform. `npm --prefix site run build` → 47
pages, 1.91s.

## Guards added, and proof each one bites

Nine tests and one guard script were added. **Every one was shown to fail**,
each mutation reverted by an explicit edit rather than `git checkout`, with
`git diff --quiet` confirming identical restoration.

| Guard | Mutation that breaks it |
|---|---|
| renders to inline SVG | written before the dependency existed |
| two renders identical | written before the dependency existed |
| no runtime reaches the browser | written before the dependency existed |
| `--` stays an en dash | remove `dashes: 'oldschool'` |
| font matches the stylesheet | shorten the stack |
| appendix has a Mermaid System Overview | written against the ASCII original |
| every building job installs a browser | remove the install step → names `deploy-site.yml:build` |
| …and cannot go vacuous | break the job-matcher → "stopped checking anything" |
| every fence renders | break `ARCHITECTURE.md`'s fence → names mermaid's lexical error |
| …and cannot go vacuous | collector matches nothing → "stopped checking anything" |
| `check-ascii-diagrams` detects ASCII | append `┌` → names `ARCHITECTURE.md:36` |
| …honours the hatch | 3 files exempted, each with its reason printed |
| …rejects an empty reason | empty it → "marker carries no reason" |
| …cannot go vacuous | break the config parse → "refusing to report a pass" |

## Traceability

| Commit | Subject | Slice | Rows |
|---|---|---|---|
| `46de726` | docs(plans): front-load M004's decisions, design S01, plan both slices | M004/S01, M004/S02  | — |
| `3609ac0` | feat(site): mermaid diagrams render to inline SVG during the build | M004/S01  | — |
| `600a486` | feat(site): diagrams render in the guide's sans face, not Arial | M004/S01  | — |
| `5ce0351` | ci(site): the Pages build installs the browser its diagrams need | M004/S01  | — |
| `472cded` | docs(site): the System Overview is a rendered diagram, not ASCII art | M004/S01  | — |
| `404dd7f` | docs(plans): close M004/S01 — the pipeline renders and reproduces | M004/S01  | GP09  |
| `4d8af00` | docs(plans): re-decompose M004/S02 so no commit carries a red gate | M004/S02  | — |
| `85a0259` | docs(plans): M004/S02 is 32 lines of diagram, not 106 | M004/S02  | — |
| `c3443d8` | docs(site): the repository layout is a list, not a drawn tree | M004/S02  | — |
| `27e5f2d` | docs: the two architecture diagrams are mermaid source | M004/S02  | — |
| `b5ed63e` | docs: the UI Overview wireframe stays, with a stated reason | M004/S02  | — |
| `8505b21` | docs: the last three files, each treated on what it actually contains | M004/S02  | — |
| `c602bbe` | docs(plans): row GP11 — nothing validates the fences GitHub renders | M004/S02  | — |
| `88d6856` | docs: refresh testing-strategy's status anchor after the diagram conversion | M004/S02  | — |
| `f7834fc` | feat(scripts): ASCII architecture diagrams cannot come back | M004/S02  | — |
| `07d9512` | test(site): every mermaid fence is proved to render, including the four GitHub draws | M004/S02  | GP11  |
| `0546db1` | docs(plans): close M004/S02 — 32 lines of diagram, 3 stated exemptions | M004/S02  | GP10, GP11  |

No commit on this branch lacks a `Slice:` trailer.

## What a reviewer should look at twice

**The plan was wrong twice and the repo caught it both times.** `/jk:next`
forbids committing with a red gate, which contradicted the plan's "commit the
failing test". After that was repaired, `doc-conformance`'s own coverage check
from M007/S03 fired: a guard script no command runs is itself a finding, so the
guard could not be committed before the conversions it demanded. S02 was
re-decomposed mid-slice so that no commit carries a red gate.

**A commit landed red, once.** `c602bbe` was committed while `doc-discipline`
failed, because the validation run and the commit were issued in one block and
nothing gated on the exit code. Fixed in `88d6856`. The more useful half is that
the violation *could not* have been caught earlier: the status-anchor arm
compares a doc's `Status:` date against its last commit, so pre-commit
validation is blind to it by construction.

**A claim I made was wrong and changed the answer.** Task 2's commit message
said invalid Mermaid "renders an error graphic rather than failing a build".
Measured: `astro build` exits 1 and emits no page. The website was never at
risk. Correcting it is what produced row GP11 — the real gap was the four
fences in `.md` files that no build touches.

**A mutation proof that did not mutate.** The first attempt at the guard's first
arm used an anchor that did not exist in the file, so nothing changed and the
guard passed. `git diff` was clean, which exposed it. It reads exactly like a
guard that does not work.

**S02's definition of done was corrected twice.** It first asserted a count
(108) that contradicted its own breakdown; then it demanded "no box-drawing
characters", satisfiable only by mangling wireframes.

**Two stale counts fixed in passing.** `counts.json` was committed at 43 presets
where the engine ships 45 — the live site was never wrong, because the build
regenerates it, but nothing cross-checks the two, and that gap is recorded and
**not** fixed here. `check-guards.sh` printed a hardcoded "13 guard invocations";
it is now computed.

**A dependency mistake, reverted.** Declaring `js-yaml` at latest installed v5,
which drops the default export and broke an existing test. Pinned to the `^4`
already in the lockfile.

## Decisions

The contents of `M004-decisions.md`, verbatim, follow.

---


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

## 2026-09-19 — re-decomposing M004/S02 mid-slice

- **Found:** the plan's original order — write the guard first, watch it fail
  against the un-converted tree, convert afterwards — is impossible in this repo,
  and both escapes from it commit a red gate.
- **Why:** `doc-conformance` carries M007/S03's own coverage check, *every repo
  guard is reachable from a command a developer can run*. It fails the instant a
  guard script exists that no command runs:

  ```
  check-ascii-diagrams.mjs is reachable from no declared token, the pre-push
  gate, pre-commit, or the doc-conformance runner
  ```

  Wiring it in earlier instead makes `guards` red, because the ASCII is still
  there. `/jk:next` forbids committing with a red gate either way.
- **Decision:** the conversions run first; the guard, its wiring, its README
  entry and the audit-file exemption land together in one green commit
  — **Why:** it is the only commit boundary at which no gate is red. This is the
  repo's own discipline catching the plan, which is the check working.
- **What did not change:** the guard was written and run against the full ASCII
  tree *before* any conversion, and that output is preserved and quoted in the
  evidence. What moved is where it is committed, not whether it was seen
  failing. The definition of done asks for a check "shown to fail by
  reintroducing one", and that mutation proof is the stronger claim anyway.

### Found while writing the guard

- **The first config parse was silently over-broad.** `(?:\s+.*\n)+` for the
  `doc_roots` block matched past the end of the list, because `\s` spans
  newlines — it swallowed `file_line_refs.source_roots` and
  `research_provenance.doc_roots`, scanning 175 files instead of 70 and
  **double-counting** `site/src/content/docs` so the appendix reported 22 lines
  where it has 11. Fixed to `[^\S\n]`. The guard now refuses to run at all if the
  parse yields no roots or no exempt dirs, rather than reporting a pass over an
  empty set — the failure `CLAUDE.md` records for the jk-standards rules that
  went green while matching nothing.
- **Measured inventory, from the guard's first run against the full tree:** 106
  lines across 8 files — `docs/euclidean-rhythm-guide.md` 44,
  `docs/testing-strategy.md` 21, `site/src/content/docs/appendix-plugin-architecture.mdx`
  11 (28 before S01 converted the System Overview), `docs/engine-spec.md` 9,
  `ARCHITECTURE.md` 8, `docs/ui-guide.md` 5, `docs/webui-migration.md` 3, and
  `docs/audits/M001-theory-audit-remediation-plan.md` 5.
- **The guard counts arrowheads** (`▲▼◄►`) alongside the U+2500 block. Checked
  first: those glyphs appear in exactly three governed files, all inside
  diagrams, so including them adds no false positives.

## 2026-09-19 — executing M004/S02: what the inventory actually contains

Classifying every remaining block before converting any of them changed the
shape of the slice. Of 106 lines, only **32** are architecture diagrams:

| Category | Lines | Where |
|---|---|---|
| Architecture / flow diagrams | 32 | `ARCHITECTURE.md`, `engine-spec.md`, `webui-migration.md`, `testing-strategy.md` pyramid |
| Directory trees | 20 | appendix `poly/`, `testing-strategy.md` fixtures |
| UI wireframes | 49 | `euclidean-rhythm-guide.md` §UI Overview (44), `ui-guide.md` (5) |
| Frozen audit record | 5 | `M001-theory-audit-remediation-plan.md` |

- **Q:** Two directory trees draw filesystem structure with `├──`. Mermaid has
  no good representation for a file tree. — **A:** Nested Markdown lists.
- **Decision:** both trees become nested bullet lists, path in code style and
  comment as prose — **Why:** not ASCII art at all, so no exemption is needed;
  renders in GitHub and on the site; stays diffable. The cost is the compact
  column alignment, which is real but smaller than the cost of a hatch that
  would also exempt anything added to those files later.

- **Q:** 49 lines are UI wireframes, chiefly a 44-line annotated mockup of the
  plugin window. No flowchart can represent it. — **A:** Keep, exempt with a
  stated reason.
- **Decision:** `euclidean-rhythm-guide.md` and `ui-guide.md` each carry a
  `boxdraw-ok` marker saying a UI mockup is not an architecture diagram
  — **Why:** forcing a panel layout into nested subgraphs produces worse
  documentation than it replaces, and the guard still protects every other
  governed doc. The exemption is greppable and counted in the guard's summary.

- **Consequence, applied to the ledger:** S02's first definition-of-done item
  becomes "no ASCII **architecture diagram** remains … any file still carrying
  box-drawing characters does so under a stated, greppable exemption". The
  previous wording — no box-drawing character at all — was satisfiable only by
  mangling content this milestone was never aimed at. This is the **second**
  correction to that item: it first asserted a count (108) that contradicted its
  own breakdown.

**Recorded, not fixed here:** a hand-drawn wireframe can disagree with the UI it
depicts, and these predate the webui migration. That is a genuine drift problem
and the exemption does not solve it — it is simply not a problem Mermaid solves
either. Replacing them with screenshots needs a capture pipeline, alt text and
`check-site-assets` wiring, which is its own slice.

## 2026-09-19 — M004/S02: a correction, and row GP11

**A claim I made was wrong, and it changed the answer.** Task 2's commit message
and the chat summary before it both said invalid Mermaid "renders an error
graphic rather than failing a build". Measured directly by breaking a fence:
`astro build` **exits 1** and emits no page. The commit message cannot be
rewritten, so the correction is recorded here.

What that changes: the website was never at risk from a bad fence. `site-e2e`
builds the site via `site-verify-local.sh`, and the Pages deploy builds it too,
so a syntax error turns the pull request red before it can reach production.

- **Q:** Four Mermaid fences live in `.md` files that nothing validates,
  including `ARCHITECTURE.md`. How much work should this get? — **A:** One row in
  S02, one test.
- **Decision:** row **GP11** and a fourth definition-of-done item, satisfied by
  `site/tests/mermaid-syntax.test.mjs` — **Why:** it is the exact gap and about
  40 lines, reusing the renderer already installed. A slice is the unit a
  reviewer accepts or rejects independently, and nobody would reject this while
  accepting the conversions it protects. Making it a row rather than a
  paragraph in a commit message is what keeps the ledger honest about work found
  during execution.

**Measured, so the trade is on the record:** 47 pages build in 1.91s with all
five diagrams, against 1.66–1.79s before. The browser starts once, so five
diagrams cost about what one did.

**Two limits stated rather than papered over.** Rendering against the pinned
mermaid 11.17.2 is a proxy for GitHub's renderer, not a guarantee — the versions
can diverge, and no local check can close that. And the Pages deploy now depends
on a browser launching, so a Chromium failure in CI blocks a docs deploy rather
than merely failing a test. Both were accepted deliberately; exact-pinning the
renderer was offered and not taken, to avoid a lock someone must unpick on every
upgrade.

### A red gate reached a commit, and why the check could not have caught it earlier

`c602bbe` was committed while `doc-discipline` was failing. Two causes, and only
one of them is a process slip:

1. **Mine.** The validation run and the `git commit` were issued in one block, so
   the exit code was printed but nothing gated on it. Every prior task in this
   milestone ran validation as its own step and read the result first.
2. **Structural, and worth knowing.** The violation *could not* have been seen
   before the commit it came from. `doc-discipline`'s status-anchor arm compares
   a doc's `Status:` date against **the doc's last commit in range**. Task 4's
   pre-commit validation was green because the change was still uncommitted —
   there was no commit to compare against. The failure appeared the moment task
   4 landed.

So for any doc carrying a `Status:` anchor, "validate, then commit" is blind by
construction; the check has to be re-run *after* committing. `docs/testing-strategy.md`
is the only doc in this milestone's scope that carries one.

Fixed by refreshing the anchor to 2026-09-19 while saying plainly in the doc
that nothing in the strategy changed — the anchor moved because the pyramid and
the fixtures listing were converted, not because the strategy was re-reviewed.
