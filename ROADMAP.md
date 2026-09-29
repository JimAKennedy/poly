# Poly Roadmap

This is the public roadmap for Poly — a polymetric drum sequencer that outputs
MIDI via VST3. It names the themes the open work falls under and links the
live issue queries for each, so contributors can see where the project is
headed and where help is most welcome without this file ever listing a number
that a closed issue makes stale.

**How this fits together:**

- **Shipped work** is recorded in the [CHANGELOG](CHANGELOG.md).
- **Planned work** lives in the delivery ledgers under
  [`docs/plans/`](docs/plans/), where each milestone is broken into slices,
  each slice carries its definition of done and the validations it owes, and
  every commit that implements one names it.
- **Open work** is tracked as
  [GitHub issues](https://github.com/JimAKennedy/poly/issues) and grouped by
  label below.
- **New here?** See [CONTRIBUTING.md](CONTRIBUTING.md) and jump straight to the
  [good first issue](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)
  list.

> This roadmap is a living document. The linked queries are the source of
> truth for what is open; this file is the map.

## Keep the build green

Correctness and a reliable CI signal come first — these keep contributions
mergeable.

- [Open bugs](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3Abug)
- [Sanitizer nightly failures](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3Asanitizer-failure)
  — filed by the nightly ASan/UBSan/TSan and fuzz jobs when something trips
- [Cubase nightly failures](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3Acubase-nightly-failure)
  — filed by the self-hosted Cubase harness

## Documentation and onboarding

Well-scoped, high-leverage work that makes the project easier to understand
and contribute to. Many of these are good candidates for a first contribution.

- [Documentation issues](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3Adocumentation)
- [Good first issues](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)

## Rhythm engine and musicality

The long-horizon direction: deepening the groove engine with feel, phrasing,
and tradition-specific musical grammar. Larger design-led work — good for
contributors who want to dig into the engine.

- [Enhancement requests](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3Aenhancement)
- Design-led engine work is planned milestone by milestone in the
  [delivery ledgers](docs/plans/) before it becomes an issue; read a ledger's
  vision section to see what is coming.

## Finding something to work on

The best entry points are labeled
[good first issue](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)
— scoped, self-contained, and reviewable without deep engine context. If none
are open right now, the documentation query above is a good place to start.
Read [CONTRIBUTING.md](CONTRIBUTING.md) for the fork-branch-PR workflow, build
setup, and code-style expectations, then comment on the issue to claim it.
