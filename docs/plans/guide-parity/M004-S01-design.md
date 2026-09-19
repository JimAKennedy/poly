---
class: gated
---

# M004/S01 design — One diagram renders from Mermaid at build time

**Slice:** guide-parity M004/S01 (row GP09) — `docs/plans/guide-parity/ledger.md`
**Classification:** architectural — adds a build-time rendering stage and a new
runtime dependency to the site build.

## What this slice establishes

A Mermaid source block in a site page becomes vector output **during
`astro build`**, not in the reader's browser. The slice converts exactly one
existing diagram to prove the path end to end; M004/S02 converts the rest.

The ledger separates the two deliberately: establishing a pipeline is a
different risk from converting content, and a reviewer could accept one and
reject the other.

## Mechanism

`rehype-mermaid` as a rehype plugin in `site/astro.config.mjs`, with
`strategy: 'inline-svg'`. Astro's markdown pipeline already runs rehype, so the
diagram is rendered while the page is built and the SVG is emitted inline into
the HTML. Nothing ships to the browser: no mermaid bundle, no client script.

Resolved dependency chain, measured by installing it:

| Package | Version resolved |
|---|---|
| `rehype-mermaid` | 3.0.0 |
| `mermaid-isomorphic` | 3.1.0 |
| `mermaid` | 11.17.2 |
| `playwright` (peer) | 1.63.0 |

`rehype-mermaid` declares `playwright: 1` as a **peer** dependency and drives a
real Chromium to lay the diagram out. The site already depends on
`@playwright/test ^1.62.1` for its e2e suite, so Playwright itself is not new —
but the `playwright` package proper is a separate entry from `@playwright/test`
and must be declared.

## Determinism — what was proved, and what was not

S01's definition of done requires that an unchanged source produce
byte-identical output across two builds. This was measured before being
designed around, in `scratchpad/mermaid-spike`:

- Two renders **in one process** → identical SHA-256, identical length.
- Two renders **in separate processes** → identical SHA-256, identical length.

**The `deterministicIds` config flag is inert here.** The obvious design was to
set `mermaidConfig: { deterministicIds: true }` and claim that is what makes the
build reproducible. Rendering with the flag and without it produced *the same
bytes and the same `id="mermaid-0"`* — `rehype-mermaid` renders each diagram in
a fresh page context, so the id counter starts at zero either way. The flag is
therefore **not** part of this design: writing it in would have documented a
cause that does not hold, and a future reader would have trusted it.

Determinism comes from per-diagram isolation in `rehype-mermaid`, which is a
property of the library rather than of our configuration. The test asserts the
property directly — render twice, compare bytes — so it keeps biting if that
library behaviour ever changes.

### Byte-identity is per-machine, and that is deliberate

The emitted SVG carries browser-computed geometry derived from font metrics:

```
style="max-width: 421.953125px;" viewBox="0 0 421.953125 70"
```

Those numbers come from laying out `arial,sans-serif` in Chromium. A machine
with different fonts installed computes different numbers, so the output is
**not** byte-identical across machines — only across builds on one machine.

This is why nothing rendered is committed. A committed SVG would churn between
a macOS laptop and an Ubuntu runner with no source change, which is exactly the
undecidable-commit-hygiene problem `#282` records for the WASM artifacts, and
which `CLAUDE.md` already documents as a live hazard where the `e2e` gate
rewrites `webui/poly_engine.{js,wasm}`. Rendering at build time and committing
nothing means there is no artifact that can go stale.

The test therefore renders twice **at test time** and compares, rather than
comparing against a committed fixture. A fixture would be a cross-machine
equality claim, and that claim is false.

## Typography

Default mermaid output sets `font-family:arial,sans-serif`, which is none of the
site's three faces. The milestone's Demo line requires diagrams that "match the
site's typography", so `mermaidConfig.fontFamily` is set to the site's sans
stack. This changes the geometry numbers — harmless, since nothing is committed.

## CI impact — the constraint that shapes this

Three places build or lint the site, and they differ in whether a browser exists:

| Job | Builds the site? | Chromium installed? | Action needed |
|---|---|---|---|
| `site-lint` (`ci.yml`) | no | no | none |
| `site-e2e` (`ci.yml`) | yes, via `site-verify-local.sh` | yes | none |
| `build` (`deploy-site.yml`) | yes, `npx astro build` | **no** | **must install chromium** |

The Pages deploy job runs `npx astro build` with no Playwright browser present.
Adding the plugin without touching that job would leave `main` green on every PR
check and break the deploy — the failure appearing only after merge. The job
gains a browser-install step, cached on the Playwright version the way
`site-e2e` already does it.

`scripts/check-release-workflow.mjs` governs `release.yml` only, not
`deploy-site.yml`, so no locked workflow contract is affected.

## Which diagram converts

`site/src/content/docs/appendix-plugin-architecture.mdx` — the only one of the
seven files Astro owns, and the file the milestone's Demo names. Its 28
box-drawing lines are the largest site-side block. S01 converts **one** diagram
within it and leaves the rest for S02, so the pipeline is proved without this
slice absorbing S02's content work.

## Risks

- **A browser in the build.** `astro build` now needs Chromium. Mitigated by
  installing it in the one job that lacked it, and by the cache both other jobs
  already use. Accepted as the cost of build-time rather than page-load
  rendering, which the DoD requires.
- **Build time.** Each diagram launches a page. One diagram in S01; S02 adds
  the rest and will report the measured cost rather than estimating it here.
- **Upstream behaviour change.** Determinism rests on library behaviour, not on
  a flag we set. The render-twice test is what detects a regression.

## Out of scope

The six non-site files (`ARCHITECTURE.md`, five `docs/*.md`) carry ```mermaid
fences that GitHub renders at view time; they need no build step and are S02's
work. The guard forbidding ASCII art is also S02's.
