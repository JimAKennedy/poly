---
class: gated
---

# M001 — Decisions

Append-only. One entry per planning session or in-flight judgment call, so the
milestone's review can see what shaped it without reconstructing it from diffs.

## 2026-09-12 — starting M001

- **Q:** The ledger is on PR #291, not on main, so M001 has no base to cut from.
  How should this run proceed? — **A:** Merge #291 on green, then run.
- **Decision:** Wait for #291's checks, merge, cut
  `milestone/M001-already-deliverable` from the updated default branch.
  — **Why:** `/jk:assess` prescribes that the ledger reach the default branch
  before any milestone branch is cut from it, and it keeps the assessment
  reviewable on its own rather than bundled into M001's pull request.

- **Decision:** Both slices classified **bounded**; no design document.
  — **Why:** S01 repeats the change shape M007 applied to 39 rules on these same
  pages, and S02 follows four existing hand-authored-timeline precedents in
  `presets.cpp`. Neither introduces a mechanism.

## 2026-09-12 — planning M001/S01

- **Q:** `theory-balkan` Rule 7 says "Humanize ≤ 0.15", but the parameter is
  0–50 ms and every other rendering of Humanize is in ms. What goes in the
  column? — **A:** Milliseconds, and restate the rule.
- **Decision:** The column renders ms; Rule 7 becomes "Humanize ≤ 7.5 ms",
  which is 0.15 of the parameter's 50 ms range. — **Why:** `0.15` is
  uninterpretable without knowing the range, and the Rice 1994 citation sources
  "near-mechanical unison" rather than the number, so this is a units fix and
  not a change to a sourced claim. Confirmed beforehand that no existing test
  locks the phrase.

- **Q:** `theory-electronic-breakbeat` Rule 4 says swing is one bus value with
  kick straight, but calls mixed per-lane swing "a legitimate advanced move".
  How strict should the predicate be? — **A:** Strict — one bus value.
- **Decision:** Assert the swung lanes share a single non-zero swing value and
  that kick and clap are 0. — **Why:** The patch is titled "Rule-Checked Jungle
  Frame" and should model the rule's default rather than its exception; the
  lenient reading barely constrains the table, which is the vacuity M007 spent
  its length removing.

- **Decision:** Corrected slice M001/S01's first definition-of-done item before
  planning against it. It read "with values that agree with
  `site/src/generated/presets.json`". — **Why:** None of the eleven theory
  patches carries a `preset` prop; they are hand-authored illustrations, so no
  value in them derives from a factory preset. The corrected item requires the
  values to be consistent with the page's own rules and roles, which is the
  contract every other rule on these pages already has. Found while
  front-loading, before any task ran.

- **Decision:** Added a fourth definition-of-done item to M001/S01 — that Rule
  7's bound is stated in the unit the column uses. — **Why:** The Humanize
  answer above makes a prose change part of EC01, and a definition of done that
  did not mention it would let the slice close with the rule still stated in a
  unit its own column does not use.

- **Decision:** `theory-gamelan` Rule 9's predicate reads the rule as a
  monotonic trend, so ties in register or rate pass and only a strict inversion
  fails. — **Why:** The rule describes a pyramid, not a total order; a strict
  reading would fail any patch with two lanes in the same register, which the
  page's own patch has and the rule does not forbid.

- **Decision:** `theory-sub-saharan-africa` Rule 7's stratum bands are declared
  as named constants beside the predicate. — **Why:** The rule names strata
  ("bell high", "dunun low") without numeric boundaries, so the test has to
  choose them; declaring them makes the choice reviewable instead of burying it
  in arithmetic.

## 2026-09-12 — planning M001/S02

- **Q:** Correcting `Cuban Son Montuno`'s clave delivers an exact son clave.
  Rumba clave and Clapping Music have no preset home at all. How far does this
  slice go? — **A:** Correct, plus two new presets.
- **Decision:** Fix the clave lane in place and add `Rumba Clave` and
  `Clapping Music`, taking the factory count from 43 to 45. — **Why:** It closes
  everything #156 names that does not already ship, and the cost is
  check-enforced rather than vigilance-dependent: a `static_assert` dimensions
  `kWebPresetLaneNames` to `kFactoryPresetCount`, and `count_drift` catches the
  two hardcoded counts in `docs/preset-taxonomy.md`.

- **Decision:** `Rumba Clave` is categorised `Latin / Brazilian` and
  `Clapping Music` `Minimalist / Compositional`. — **Why:** Those are the
  categories the neighbouring presets already use — `Cuban Son Montuno`, and
  `Reich Phasing` / `Reich Phase Process` respectively — and the taxonomy
  requires every preset to sit in one of the existing ten.

- **Decision:** All three patterns are taken from the guide rather than derived:
  son clave from `theory-afro-cuban.mdx`, rumba clave from `03-afro-cuban.mdx`,
  Clapping Music from `08-minimalism.mdx`. — **Why:** The repo already states
  each one, so authoring them from memory would risk contradicting the pages the
  presets are meant to illustrate.

- **Decision:** Row EC05 closes in S02's final task, not incrementally.
  — **Why:** A row closes when its item is done, and #156 is not satisfied until
  all three patterns ship and the guide stops presenting the workaround as the
  only route.
