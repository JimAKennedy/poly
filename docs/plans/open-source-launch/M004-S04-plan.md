# M004/S04 — The site sends readers to the download

**Slice:** M004/S04 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS26 (the construction banner), OS27 (the hero offers no download
and no try-it), OS41 (the guide's install section names bundles the release
does not contain)
**Depends:** M004/S02.
**Decisions consumed:** `M004-decisions.md`, 2026-09-29 — the banner
becomes a pre-release notice; *Try it* is `/01-foundations/`; the hero's
three actions replace both existing ones; the guide's install section loses
the Audio Unit.

## Task status

- [ ] Task 1 — The banner says pre-release and the hero offers download, try
      it and the guide (OS26, OS27)
- [ ] Task 2 — The guide's install section describes the zip that ships
      (OS41); the slice closes

## Definition of Done

- [ ] No page carries the construction banner
- [ ] The home page's hero offers the download and the in-browser engine beside
      the guide
- [ ] The guide's install section names the bundle a release actually contains

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `guards` | `bash scripts/check-guards.sh` |

## Task 1 — The banner says pre-release and the hero offers download, try it and the guide (OS26, OS27)

**Files:** create `site/tests/front-door.test.mjs`; modify
`site/src/components/Banner.astro`, `site/src/content/docs/index.mdx`.

1. Write `site/tests/front-door.test.mjs`:
   - `no component carries the construction banner text`: no file under
     `site/src` contains "Under active construction".
   - `the banner is a pre-release notice linking the Releases page`:
     `Banner.astro` contains `pre-release` (case-insensitive) and
     `https://github.com/JimAKennedy/poly/releases`.
   - `the hero offers download, try it and the guide`: parse `index.mdx`'s
     frontmatter `hero.actions` (a small YAML read of the `actions:` block:
     each `- text:` with its `link:`) and assert exactly three actions, in
     order: text `Download` → `https://github.com/JimAKennedy/poly/releases`;
     text `Try it in the browser` → `/01-foundations/`; text `Read the guide`
     → `/introduction/`.
   Run: all three red.
2. `Banner.astro`: the span becomes "🎧 The first release is in preparation
   — builds will appear on the <a href="https://github.com/JimAKennedy/poly/releases">Releases page</a>;
   the guide is complete and verified." Keep the styles.
   `index.mdx`: replace the two actions with the three above; `Download`
   with `icon: download`, `Try it in the browser` with `icon: rocket` and
   `variant: secondary`, `Read the guide` with `icon: open-book` and
   `variant: minimal`.
3. Run the test: green. Run `npm --prefix site run build` once to prove the
   frontmatter still builds (Starlight validates hero actions at build).
   Run `bash scripts/check-site-assets.sh`: green.
4. Run `format`, `site-unit`, `doc-conformance`, `guards`. Evidence. Tick.
   Commit with `Rows: OS26, OS27`.

## Task 2 — The guide's install section describes the zip that ships (OS41); the slice closes

**Files:** modify `site/src/content/docs/guide-using-poly.mdx`,
`site/tests/host-table.test.mjs`, `docs/plans/open-source-launch/ledger.md`.

1. Add to `host-table.test.mjs`: `the guide's install section names the
   bundle a release contains` — `guide-using-poly.mdx` contains
   `poly_plugin.vst3`, does not contain `Poly.vst3` or `Poly.component`, and
   does not say Poly ships in two formats. Run: red.
2. Rewrite `### Install` in `guide-using-poly.mdx`: download the zip for your
   platform from the Releases page (`poly-<version>-macos.zip` or
   `poly-<version>-windows.zip`); it extracts to a `poly_plugin.vst3` bundle;
   copy the whole bundle into the VST3 folder (the two paths as today);
   restart the DAW. Remove the "two formats" sentence, the Audio Unit copy
   instructions, the AU paragraph, the `killall AudioComponentRegistrar`
   sentence, and in "Build from Source" the Audio Unit build block and the
   `auval` paragraph; keep the VST3 build block. One sentence notes that
   Logic Pro is not supported and points at the note below Load Poly (S03 of
   M003 wrote it).
3. Run the test: green. Run `bash scripts/check-doc-conformance.sh`.
4. Run `format`, `site-unit`, `doc-conformance`, `guards`. Tick every DoD
   box here and in the ledger; set the slice `done`; OS41 `done`. Evidence
   with the DoD-to-task table. Run `jk-standards ledger`. Commit with
   `Rows: OS41`.

## Self-review

| DoD | Task |
|---|---|
| No page carries the construction banner | 1, held by the test |
| The hero offers the download and the engine beside the guide | 1, held by the test |
| The install section names the bundle a release contains | 2, held by the test |

| Row | Task | Verification produced |
|---|---|---|
| OS26 | 1 | the banner is a release notice; a test forbids the construction text |
| OS27 | 1 | actions are download, try it, guide; `check-site-assets` passes |
| OS41 | 2 | the zip as it ships; `Poly.vst3` forbidden by a test |

No placeholders.
