# Working in the Poly guide site

This directory is the source for the [poly.jk.digital](https://poly.jk.digital) guide
_Layered Repetition as Sonic Architecture_ — a Starlight (Astro) documentation site that
ships prose chapters, an interactive in-browser Poly WebUI, and conformance tests. See
`README.md` for the full layout and command reference before starting.

## Development

Run everything from `site/`. Start the dev server with:

```
npm run dev
```

`dev` (and `build`/`start`) first run `generate-presets`, `generate-counts`, and
`generate-params`, which regenerate the JSON under `src/generated/` and `src/data/` from the
Poly engine. Do not hand-edit that generated data — change the engine or the generator
scripts (`scripts/generate-*.mjs`) instead.

The interactive pages embed the real Poly WebUI. `npm run build` runs `copy-webui` to copy
`webui/` (and the compiled wasm engine, when present) from the repo root into
`public/webui/`.

## Before you commit

- **Prose or preset/parameter claims:** run `npm test` (the `node --test` suites in
  `tests/`). They assert that the guide's prose, preset tables, and Euclidean/timing claims
  stay consistent with engine behavior — a wrong claim fails the build.
- **Interactive WebUI pages:** run `npm run test:e2e` (Playwright specs in `tests-e2e/`),
  which drive the embedded engine in a real browser.
- **Site metadata, sidebar, redirects:** edit `astro.config.mjs`. Keep the `site:` value at
  `https://poly.jk.digital` so absolute links and Open Graph metadata resolve.

## Authoring content

Guide pages are `.mdx` in `src/content/docs/`, exposed as routes by file name. Numbered
chapters, `theory-*` deep-dives, and `appendix-*` reference pages are wired into the sidebar
in `astro.config.mjs`; add new pages there too. Shared UI lives in `src/components/`, styling
in `src/styles/`.

## Note

`CLAUDE.md` is a symlink to this file — edit `AGENTS.md` and both stay in sync.
