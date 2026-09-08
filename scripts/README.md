---
class: gated
---

# Scripts directory map

One-line purpose for every script in this directory, grouped by role. This map is
machine-enforced: `check-scripts-readme.sh` fails CI when a top-level script or
subdirectory is missing from this file, or when an entry here names a script
that no longer exists. Add the entry in the same change that adds the script.

**Platform split**: everything at the top level is POSIX shell or Node — it
runs on macOS for local development and on the Linux/macOS CI runners. The
Windows-only PowerShell machinery lives entirely in the `cubase/` and
`S08-install/` subdirectories and targets the self-hosted Cubase runner, never
a development Mac.

## Build & deploy (local development, macOS)

- `build.sh` — incremental CMake build of the VST3 plugin; by default also
  deploys the bundle to `~/Library/Audio/Plug-Ins/VST3` for Cubase testing and
  verifies the installed copy byte-matches the build. `--help` lists flags
  (`--clean`, `--debug`, `--test`, `--no-deploy`).
- `build-wasm.sh` — emscripten build of the engine to `poly_engine.{js,wasm}`
  for the guide site and webui.
- `lib/` — sourceable helpers shared by other scripts, not entry points
  (`lib/ensure-emsdk.sh` guarantees the pinned emcc toolchain is on PATH).

## Site verification pipelines

- `site-verify-local.sh` — full local pipeline: build WASM, build the Astro
  site, serve it via `astro preview`, run every Playwright gate against it.
  This is the `e2e` validation token (`.jk/validations.yml`).
- `site-verify-remote.sh` — the same gate suite plus WASM freshness, run
  against a deployed URL; invoked by `deploy-site.yml` after each Pages deploy.

## Quality gates (`check-*`)

Regression guards run by CI and/or the pre-push hook. Guards with a paired
`.mjs` file of the same name follow the house seam: the `.sh` is the guard,
the `.mjs` is a `node --test` proof that the guard is wired into CI and
actually fails on regressions.

- `check-bridge-schema-coverage.mjs` — every bridge action in
  `webui/bridge.schema.json` has a payload definition or a declared
  empty payload.
- `check-doc-conformance.sh` — the doc-conformance guardrail suite for site
  prose: euclidean claims, preset tables, audit ledgers (the `doc-conformance`
  validation token).
- `check-personal-paths.sh` / `check-personal-paths.mjs` — no personal-machine
  paths in tracked files.
- `check-pragma-once.sh` — every C/C++ header carries `#pragma once`.
- `check-realtime-safety.sh` — scans audio-thread code for RT-unsafe
  operations (allocation, locks, blocking calls); suppress false positives
  with `// RT-SAFE-OK`.
- `check-release-workflow.mjs` — contract test locking the shape of
  `release.yml`, which cannot be exercised by a real tag push in CI.
- `check-sample-manifest.sh` — every shipped audio file has a manifest entry
  and every manifest entry has a file.
- `check-scripts-readme.sh` / `check-scripts-readme.mjs` — this README stays
  in sync with the directory (the guard described at the top of this file).
- `check-site-assets.sh` — every asset the site docs reference exists under
  the site's public tree.
- `check-site-readme.sh` / `check-site-readme.mjs` — the site's contributor
  docs stay project-specific, never Starlight template boilerplate.
- `check-snippet-regions.sh` — every `<CodeSnippet>` region cited by site docs
  resolves to a real region marker in the tree.
- `check-spdx-headers.sh` / `check-spdx-headers.mjs` — SPDX license headers on
  all tracked source files.
- `check-wasm-freshness.sh` — the deployed WASM + JS glue hash-matches a fresh
  local build of the engine they claim to be.

## Generators

Emit generated docs/data — edit the source of truth and rerun, never
hand-edit the output (enforced by the jk-standards generated-freshness check).

- `generate-bridge-schema-doc.mjs` — action/message reference tables in
  `webui/bridge-schema.md` from the bridge schema.
- `generate-euclidean-appendix.mjs` — pattern/grouping columns of the
  euclidean reference appendix from its fixture data.
- `generate-param-docs.mjs` — parameter tables in the site's parameter
  appendix and `docs/engine-spec.md` from the engine headers.

## Pre-push gate

- `pre-push-check.sh` — the pre-push hook: blocks direct pushes to main, then
  runs format, RT safety, snippet regions, build, tests (the `gate`
  validation token). Install via `pre-commit install -t pre-push`.
- `run-clang-tidy-changed.sh` — clang-tidy over PR-changed engine C/C++ files;
  shared between CI and local runs.

## Maintainer aids

- `fetch-samples.sh` — fetch the CC0/CC-BY drum one-shots from their pinned
  upstream repos into the site's samples tree.
- `gen-release-notes.mjs` — extract a CHANGELOG section body for
  `gh release create --notes-file`; no CI wiring.
- `install-pluginval.sh` — install pluginval locally for pre-push
  verification (Homebrew first, CI's release asset as fallback).
- `verify-good-first-issue-count.test.js` — contract test that enough
  well-scoped open onboarding issues exist on the GitHub repo.

## Windows Cubase runner (PowerShell)

Both subdirectories target the self-hosted Windows runner that drives
Cubase-in-the-loop nightly testing. Each has its own README owning its
contents; this map covers them as units.

- `cubase/` — launch/quit/diagnose machinery the `cubase-nightly.yml` workflow
  calls to drive Cubase (no scripting API — window polling and
  process control).
- `S08-install/` — run-once provisioning helpers for the runner machine
  itself: plugin install, MIDI Remote, driver dependencies, preflight.
