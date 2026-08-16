# Poly Guide Site

The contributor-facing source for **[poly.jk.digital](https://poly.jk.digital)** — the
online guide _Layered Repetition as Sonic Architecture_, a book-length treatment of
polymetric drumming built around the Poly VST3 instrument.

This is a [Starlight](https://starlight.astro.build/) (Astro) documentation site. It
publishes the prose chapters, an interactive in-browser WebUI powered by the Poly engine
compiled to WebAssembly, and a battery of conformance tests that keep the prose, the
preset library, and the engine in agreement.

## What lives here

- **`src/content/docs/`** — the guide itself, as `.mdx`. Numbered chapters (`01-foundations`
  through `18-editors-and-views`), companion `theory-*` deep-dives, and `appendix-*`
  reference pages (parameters, presets, Euclidean tables, timing model, plugin and website
  architecture). The sidebar and site metadata are defined in `astro.config.mjs`.
- **`src/components/`, `src/styles/`, `src/audio/`, `src/lib/`** — Astro components, custom
  CSS, and the client-side audio/playback helpers the interactive pages use.
- **`src/generated/`, `src/data/`** — JSON derived from the engine (presets, note-map counts,
  parameter definitions). Regenerated, not hand-edited (see the `generate-*` scripts below).
- **`public/webui/`** — the embedded Poly WebUI, copied from the repo-root `webui/` by the
  `copy-webui` script at build time so the guide can run the real engine in the browser.
- **`tests/`** — Node `node --test` conformance suites that assert the prose and preset
  tables match engine behavior (Euclidean claims, preset taxonomy, SMF export, etc.).
- **`tests-e2e/`** — Playwright end-to-end specs that drive the embedded WebUI in a real
  browser (audio probes, control audits, parity checks).

## Commands

All commands run from `site/`:

| Command                     | Action                                                                       |
| :-------------------------- | :--------------------------------------------------------------------------- |
| `npm install`               | Install dependencies                                                         |
| `npm run generate-presets`  | Regenerate `presets.json` from the engine                                    |
| `npm run generate-counts`   | Regenerate note-map count data                                               |
| `npm run generate-params`   | Regenerate parameter-definition data                                         |
| `npm run copy-webui`        | Copy the built Poly WebUI (and wasm engine, if present) into `public/webui/` |
| `npm run dev`               | Regenerate data, then start the local dev server at `localhost:4321`         |
| `npm run build`             | Regenerate data, copy the WebUI, and build the production site to `./dist/`   |
| `npm run preview`           | Preview the production build locally before deploying                        |
| `npm test`                  | Run the `node --test` conformance suites in `tests/`                          |
| `npm run test:e2e`          | Run the Playwright end-to-end suites in `tests-e2e/`                          |

`dev`, `start`, and `build` run the `generate-*` steps first, so the interactive pages
always reflect the current engine.

## Deploying

`npm run build` emits a static site to `site/dist/`, which is published to
[poly.jk.digital](https://poly.jk.digital). The `site:` value in `astro.config.mjs` must
match that canonical URL for absolute links and Open Graph metadata to resolve.

## More about Poly

- Engine and plugin source, architecture, and roadmap live in the repository root — see the
  top-level `README.md`, `ARCHITECTURE.md`, and `CHANGELOG.md`.
- Contributor conventions for working inside this site are in `AGENTS.md`
  (`CLAUDE.md` is a symlink to it).
