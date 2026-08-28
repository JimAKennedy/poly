# Poly Roadmap

This is the public, issue-backed roadmap for Poly — a polymetric drum pattern
generator that outputs MIDI via VST3. It organizes the currently open work by
theme and priority so contributors can see where the project is headed and where
help is most welcome.

**How this fits together:**

- **Shipped work** is recorded in the [CHANGELOG](CHANGELOG.md).
- **Release milestones** are tracked as
  [GitHub milestones](https://github.com/JimAKennedy/poly/milestones); the
  milestone, slice, and task decomposition behind them lives in delivery ledgers
  under `docs/plans/`, where each slice carries its definition of done and the
  validations it owes.
- **Forward work** is tracked as
  [GitHub issues](https://github.com/JimAKennedy/poly/issues) and grouped below.
- **New here?** See [CONTRIBUTING.md](CONTRIBUTING.md) and jump straight to the
  [good first issue](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)
  list.

> This roadmap is a living document. Issue numbers and priorities change as work
> lands — treat the linked GitHub issue list as the source of truth and this file
> as the map.

## Priority 1 — Keep the build green (bugs, CI, test stability)

Correctness and a reliable CI signal come first — these keep contributions
mergeable.

| Issue | Title | Labels |
|-------|-------|--------|
| [#172](https://github.com/JimAKennedy/poly/issues/172) | Cubase nightly failure | `cubase-nightly-failure` |
| [#142](https://github.com/JimAKennedy/poly/issues/142) | Sanitizer nightly failure: TSAN-PLUGIN | `sanitizer-failure` |
| [#91](https://github.com/JimAKennedy/poly/issues/91) | `appendix-euclidean-reference.mdx`: E(3,16) row shows incorrect pattern | `bug` |
| [#89](https://github.com/JimAKennedy/poly/issues/89) | `tests-e2e/reich-play.spec.ts` flaky: timing threshold too tight | `bug` |

## Priority 2 — Documentation and onboarding

Well-scoped, high-leverage work that makes the project easier to understand and
contribute to. Many of these are good candidates for a first contribution.

| Issue | Title | Labels |
|-------|-------|--------|
| [#100](https://github.com/JimAKennedy/poly/issues/100) | `appendix-presets.mdx` documents only 14 of 43 factory presets | `documentation` |
| [#111](https://github.com/JimAKennedy/poly/issues/111) | Convert website architecture diagrams from ASCII to build-time Mermaid | `documentation`, `enhancement` |

## Priority 3 — Rhythm engine and musicality enhancements

The long-horizon direction: deepening the groove engine with feel, phrasing, and
tradition-specific musical grammar. Larger design-led work — good for
contributors who want to dig into the engine.

| Issue | Title | Labels |
|-------|-------|--------|
| [#158](https://github.com/JimAKennedy/poly/issues/158) | Metric-position-aware ghost note placement (funk grammar) | `enhancement` |
| [#157](https://github.com/JimAKennedy/poly/issues/157) | Cell-aware swing and long-beat feel for additive (aksak) meters | `enhancement` |
| [#156](https://github.com/JimAKennedy/poly/issues/156) | Ship exact non-Euclidean timelines as first-class presets (clave, teleco-teco, Clapping Music, bell variants) | `enhancement` |
| [#155](https://github.com/JimAKennedy/poly/issues/155) | Coupled call-and-response phrase gating (`responseSourceLane`) | `enhancement` |
| [#154](https://github.com/JimAKennedy/poly/issues/154) | Phrase-position-aware fills and a tihai generator | `enhancement` |
| [#153](https://github.com/JimAKennedy/poly/issues/153) | Kotekan modes: norot/telu/empat variants and controlled polos-sangsih overlap | `enhancement` |
| [#152](https://github.com/JimAKennedy/poly/issues/152) | Timeline-aware constraint: clave/bell attraction-avoidance mask for mutation and fills | `enhancement` |
| [#151](https://github.com/JimAKennedy/poly/issues/151) | Correlated (1/f) humanize instead of white-noise timing jitter | `enhancement` |
| [#150](https://github.com/JimAKennedy/poly/issues/150) | Non-isochronous subdivision profiles (samba/jembe feel templates) | `enhancement` |
| [#149](https://github.com/JimAKennedy/poly/issues/149) | Tempo-dependent, wider-range swing (jazz ratios beyond 2:1) | `enhancement` |

## Finding something to work on

The best entry points are labeled
[good first issue](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)
— scoped, self-contained, and reviewable without deep engine context. If none
are open right now, the Priority 2 documentation items above are a good place to
start. Read [CONTRIBUTING.md](CONTRIBUTING.md) for the fork-branch-PR workflow,
build setup, and code-style expectations, then comment on the issue to claim it.
