# M004/S03 — The README is written for the person downloading

**Slice:** M004/S03 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS22 (README order, signing section, screenshot, internal IDs),
OS23 (`CLAUDE.md` names VSTGUI), OS24 (`.bg-shell/manifest.json` tracked and
ignored), OS25 (nothing tells a contributor which docs to read)
**Depends:** M003/S01 (done), M004/S02.
**Decisions consumed:** `M004-decisions.md`, 2026-09-29 — `RELEASING.md` is
created; `xattr` stays until M009; the docs index gets a guard and a
`cannot_drift` entry; `check-front-door.mjs` gains the ID assertions.

## Task status

- [ ] Task 1 — The stale manifest is untracked and its excludes are gone
      (OS24)
- [ ] Task 2 — `CLAUDE.md` names the WebView editor (OS23)
- [ ] Task 3 — `docs/README.md` tells a contributor what to read, and a guard
      keeps it complete (OS25)
- [ ] Task 4 — The README reads for the downloader, signing moves to
      `RELEASING.md`, and no internal ID remains (OS22); the slice closes

## Definition of Done

- [ ] The README's order is: positioning, screenshot, try it in the browser,
      download, DAW setup — then contributing and building
- [ ] Signing-secret provisioning moves to a maintainer document the README
      links to
- [ ] No internal decision or milestone ID appears in `README.md` or
      `CONTRIBUTING.md`, and a guard fails if one returns
- [ ] A contributor can tell from one index which documents under `docs/` are
      for them
- [ ] The two stale artefacts are gone

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `guards` | `bash scripts/check-guards.sh` |

## Task 1 — The stale manifest is untracked and its excludes are gone (OS24)

**Files:** delete `.bg-shell/manifest.json` from the index; modify
`.pre-commit-config.yaml`.

1. `git rm --cached .bg-shell/manifest.json`; `git ls-files .bg-shell` prints
   nothing. The file stays on disk, ignored by `.gitignore:327`.
2. In `.pre-commit-config.yaml`, change both `exclude:` patterns from
   `'^(\.gsd/|\.bg-shell/|.*\.uidesc$)'` to `'^(\.gsd/|.*\.uidesc$)'`.
3. Run `format` (pre-commit reads the changed config) and `guards`. Evidence.
   Tick the box. Commit with `Rows: OS24`.

## Task 2 — `CLAUDE.md` names the WebView editor (OS23)

**Files:** modify `CLAUDE.md`.

1. Line 18, `- VST3 SDK 3.7+, VSTGUI 4`, becomes
   `- VST3 SDK 3.7+; the editor is a choc WebView (`webui/`), not VSTGUI, which is switched off in CMake`.
   The ownership-transfer convention's "VST3/VSTGUI" becomes "VST3". The
   "Active roadmap is public GitHub milestones + CHANGELOG.md" sentence
   becomes: the roadmap is `ROADMAP.md`, planned work is the delivery
   ledgers under `docs/plans/`, and shipped work is `CHANGELOG.md`.
2. `grep -n -i vstgui CLAUDE.md` prints nothing that describes current
   state. Run `format` and `doc-discipline`. Evidence. Tick. Commit with
   `Rows: OS23`.

## Task 3 — `docs/README.md` tells a contributor what to read, and a guard keeps it complete (OS25)

**Files:** create `docs/README.md`, `scripts/check-docs-index.mjs`; modify
`.github/docs-drift-map.yml`, `scripts/check-guards.sh`, `scripts/README.md`.

1. Write `scripts/check-docs-index.mjs` (node:test): every `docs/*.md` other
   than `README.md` itself, and every immediate subdirectory of `docs/`,
   appears in `docs/README.md` as a backticked token (`name.md` or
   `name/`); every such token in the index resolves to a real file or
   directory. Run: red (no index).
2. Write `docs/README.md` with `class: gated` frontmatter and three
   sections:
   - **For users** — none; users read the guide at poly.jk.digital, say so.
   - **For contributors** — `engine-spec.md`, `euclidean-rhythm-guide.md`,
     `midi-note-mapping.md`, `preset-taxonomy.md`, `testing-strategy.md`,
     `cubase-workflow.md`, `sample-sourcing.md`, `pr-af-review.md`,
     `create-golden-disk-image.md`, the three `windows-*.md` runner
     documents, one line each saying what it is for.
   - **Delivery records** — `plans/` (the ledgers, plans, evidence, decisions
     and reports; the file is the state, git the history), `audits/`,
     `reviews/`, `verification/`, `internal/`, and the four `archived`
     documents (`ui-guide.md`, `webui-migration.md`, `webui-drag-spike.md`,
     `webui-capture-timeline-uat.md`) named as history.
   Two sentences at the top: this directory is mostly delivery records, and
   a contributor needs only the second section.
3. Add to `.github/docs-drift-map.yml`'s `cannot_drift` list an entry for
   `docs/README.md` with the reason: an index of the directory, kept
   complete by `scripts/check-docs-index.mjs` rather than by a source
   pairing.
4. Run the guard: green. Wire `run_guard "docs-index" node --test scripts/check-docs-index.mjs`
   into `check-guards.sh` after `front-door contract`; add its bullet to
   `scripts/README.md`. Run `guards`, `doc-discipline` (doc-completeness
   must accept the new document). Evidence. Tick. Commit with `Rows: OS25`.

## Task 4 — The README reads for the downloader, signing moves to `RELEASING.md`, and no internal ID remains (OS22); the slice closes

**Files:** modify `README.md`, `CONTRIBUTING.md`,
`scripts/check-front-door.mjs`, `docs/plans/open-source-launch/ledger.md`;
create `RELEASING.md`.

1. Add to `check-front-door.mjs`: `README.md and CONTRIBUTING.md carry no
   internal decision or milestone ID` — `/\b[DM]0\d\d\b/` must not match
   either file. Run: red (six and two). Add: `README.md's sections run
   positioning, screenshot, try it, download, DAW setup, then contributing
   and building` — the `## ` headings, in order, begin
   `Try it in the browser`, `Download`, `DAW compatibility`, `Guide`,
   `Contributing`, `Building` and the screenshot image appears before the
   first `## `. Run: red.
2. Create `RELEASING.md`: a one-paragraph introduction (for maintainers;
   how a release is cut is `release.yml`, tagged `v*.*.*`), then the
   README's entire "Signing and notarization" section moved verbatim minus
   its decision IDs — the sentence "This is decision **D031** (M030 S03),
   which revises D004's unsigned-forever deferral." becomes "Signing runs
   automatically once the secrets exist; until then releases ship
   unsigned." The secrets table is unchanged.
3. Rewrite `README.md` in this order:
   - `# Poly`, the sentence (from S02), the two badges.
   - `![Poly's editor …](site/public/screenshots/ui-overview.png)` with the
     alt text the guide uses for that image.
   - `## Try it in the browser` — one paragraph: the engine runs in the
     guide's pages; link `https://poly.jk.digital/01-foundations/` for the
     first playable patterns.
   - `## Download` — the current "Installing a release" body (zip from the
     Releases page, extracts to `poly_plugin.vst3`, the two folders, the
     Linux note without its decision ID, the macOS Gatekeeper `xattr`
     subsection unchanged except "see Signing and notarization" becomes a
     link to `RELEASING.md`).
   - `## DAW compatibility` — the host table and its two notes, unchanged.
   - `## Guide`, unchanged.
   - `## Contributing` — the current bullets plus one for
     `docs/README.md` ("which documents under `docs/` are for you"), and one
     sentence pointing maintainers at `RELEASING.md`.
   - `## Building` — the current Building body, its Run tests and
     Engine-only subsections, and the "Supported platforms" paragraph folded
     in without its decision IDs.
   - `## Architecture` and `## License`, unchanged, except the "Active work
     is tracked in the public GitHub milestones" sentence becomes the
     roadmap and the ledgers, as in `CLAUDE.md`.
   Remove the six IDs in the process. In `CONTRIBUTING.md` line 16, "This is
   decision D029 (M054); see `CHANGELOG.md` for the scope statement."
   becomes "See `CHANGELOG.md` for the scope statement."
4. Run the guard: green. Run `node --test site/tests/host-table.test.mjs`
   and `site/tests/positioning.test.mjs`: still green (the table and the
   sentence moved, not changed).
5. Run `format`, `doc-discipline`, `doc-conformance`, `guards`. Tick every
   DoD box here and in the ledger; set the slice `done`; OS22 `done`.
   Evidence with the DoD-to-task table. Run `jk-standards ledger`. Commit
   with `Rows: OS22`.

## Self-review

| DoD | Task |
|---|---|
| The README's order | 4, held by the guard |
| Signing moves to a maintainer document the README links to | 4 — `RELEASING.md`, linked from Download and Contributing |
| No internal ID in README or CONTRIBUTING, guard seen red | 4 |
| One index says which docs are for a contributor | 3 |
| The two stale artefacts are gone | 1 (the manifest) and 2 (the VSTGUI line) |

| Row | Task | Verification produced |
|---|---|---|
| OS22 | 4 | order matches the DoD; the ID guard was red on both files; `RELEASING.md` holds the table |
| OS23 | 2 | the line names the WebView; no current-state VSTGUI reference |
| OS24 | 1 | `git ls-files .bg-shell` empty; excludes dropped |
| OS25 | 3 | the index classifies `docs/`; README links it; `doc-discipline` passes |

Names used throughout: `check-front-door.mjs`, `check-docs-index.mjs`,
`RELEASING.md`, `docs/README.md`. No placeholders.
