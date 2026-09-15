---
class: gated
---

# M007 — Decisions

Append-only. One entry per planning session or in-flight judgment call, so the
milestone's review can see what shaped it without reconstructing it from diffs.

## 2026-09-15 — planning M007

- **Decision:** Both slices classified **bounded**; no design document.
  — **Why:** S01 declares a token wrapping eleven existing commands; S02 adds
  one assertion and one in-band marker. Neither introduces a mechanism.

- **Re-measured before planning.** Eleven guards run in CI and from no
  documented local command — six `check-*.sh` and five `check-*.mjs` — which is
  exactly what `GAP03` recorded. M006 added `check-doc-discipline.sh`, which is
  reachable through the `doc-discipline` token, so the count did not drift.

- **Q:** The eleven split across two CI jobs — eight in `code-quality`, three in
  `site-lint`. How should they be declared locally? — **A:** One `guards` token.
- **Decision:** A single `guards` token running all eleven. — **Why:** A
  developer asking what CI will run on their change gets one answer, and a slice
  touching anything guarded names one thing. Mirroring the CI job split would
  map a red job to one command, but that split is a fact about CI's job layout
  rather than about the guards, and each guard names itself when it fails.

- **Q:** How should a genuinely-unrunnable guard be declared?
  — **A:** An in-band marker in the script.
- **Decision:** `# local-unrunnable: <reason>` in the guard's own header, which
  the reachability check honours. — **Why:** It matches the escape-hatch
  discipline this repo documents: in-band, greppable, reasoned, so
  `grep -rn local-unrunnable scripts/` enumerates every exemption and that
  listing *is* the audit. A list in the test file would put the reason away from
  the script it excuses, which is how a registry drifts from the code.
  `check-wasm-freshness.sh` is the real case: it compares a deployed URL's
  artifacts and has nothing to check from a clean checkout.

- **Q:** Should the pre-push hook run the new guards? — **A:** Runnable, not
  automatic.
- **Decision:** `scripts/pre-push-check.sh` is left alone; the token is declared
  and named in `CLAUDE.md`. — **Why:** M006 established the hook is not at fault
  and does exactly what it documents, and this milestone's vision is that a
  developer *can* run every check CI will run — not that every push pays for all
  of them. The trade is real and worth naming: wiring it into pre-push would
  have caught M002's ship failure automatically, and leaving it out means a
  slice has to name the token. S02's reachability check is what stops a guard
  going unreachable again; it does not stop someone forgetting to run one.
