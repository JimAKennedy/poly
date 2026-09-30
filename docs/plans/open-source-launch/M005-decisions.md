# M005 — decisions

Every question `/jk:auto` asked before running, every answer, and every choice
taken on the owner's behalf. Append-only.

## 2026-09-29 — planning M005/S01; M005/S02 deferred

Measured before asking, on `main` at `19e7aa3`. `scripts/gen-release-notes.mjs`
prints the whole changelog section for a version; for 0.2.0 that is 15,327
words, and `release.yml` publishes it verbatim as the Release body.
`scripts/check-release-workflow.mjs` asserts the generator emits a non-empty
body for 0.1.0 and for the version `CMakeLists.txt` declares, and fails loud
on a missing section. The 0.2.0 section opens with `### Fixed`. No
`known-issue` label exists. The six open issues are CI, tooling and
documentation matters; none describes something a musician meets.

- **Q:** OS28: where do the musician-facing notes live — a block at the top
  of each changelog version, one file per version under
  `docs/release-notes/`, or a root `RELEASE_NOTES.md`? — **A:** a block at
  the top of each changelog version.
- **Decision:** each `## [x.y.z]` section opens with `### For musicians`;
  the generator emits that block and a link to the full section, and fails
  loud when a version has no block — **Why:** one file, no new document to
  register, and the engineering entries stay exactly where they are, below.
- **Q:** the word ceiling for the emitted body? — **A:** 400.
- **Decision:** the contract test counts the generator's whole output,
  link included, and fails above 400 words — **Why:** room for what Poly
  does, the hosts, a short known-issues list and how to report, and no more.
- **Q:** OS29: what does the Known issues list contain? — **A:** what a
  musician meets, plus labelled issues.
- **Decision:** hand-written items — only Cubase measured, unsigned builds
  warn on both platforms, the editor is a fixed 1160×760 — plus a link to
  open issues carrying a `known-issue` label, which this run creates —
  **Why:** none of today's open issues is something a musician meets, and a
  label lets a future one join the list without editing prose.

### Deferred — M005/S02, at the boundary after M006/S01

S02 depends on M006/S01, a published Release, and its deliverables are
listings the owner submits on sites the repository does not control. Two
questions wait there: **which outlets** beyond KVR Audio (the ledger names
Rekkerd and Bedroom Producers Blog as the category's), and **the listing
copy** beyond OS21's sentence. S02 is not planned in this run; the loop stops
after S01 and says so.

### Taken on the owner's behalf

- **A version without a `### For musicians` block fails the release.** The
  contract test that proved the generator on 0.1.0 now proves the opposite:
  0.1.0 was never tagged, M001 kept its section as history, and tagging it
  would publish June's engineering notes, so the generator refusing is the
  right answer. The test is renamed to say so.
- **The emitted body ends with one link** to `CHANGELOG.md` at the release
  tag on GitHub, without a section anchor: the anchor changes when M006
  dates the heading, and a file link is stable.
- **The block's headings are fixed** — `What Poly is`, `Supported hosts`,
  `Known issues`, `Report a problem` — and the contract test requires all
  four, so a future version cannot drop one quietly.
- **`RELEASING.md` gains the rule** in one paragraph, because it is the
  maintainer document the README points at.
- **The Supported hosts text is the README's host table in prose**, the
  same three facts, so the two cannot disagree; the test does not compare
  them, because the notes are prose and the table is a table.

## 2026-09-30 — M005/S02 at its boundary: descoped

The boundary was reached: M006/S01 published `v0.2.0-rc.1`, and the two
deferred questions — which outlets, and the listing copy — were put to the
owner, with a third the first cut raised, whether anything should be
announced against a release candidate.

- **Q:** which outlets, when, and who writes the copy? — **A:** none: "I
  want to avoid advertising the plug-in for now and keep it to friends and
  family. So let's descope S02."
- **Decision:** S02 is `accepted` as not done, OS30 `accepted`, the plan
  file records the path and carries no tasks, and the milestone's vision
  says its last clause waits for a public-launch decision — **Why:** the
  owner's call on the product's exposure; a listing is an announcement, and
  none is wanted yet.

### Taken on the owner's behalf

- **The descope is `accepted`, not deleted.** The rows and the DoD stay in
  the ledger unticked, so a later decision to launch reopens a slice whose
  shape is already written rather than re-planning from the vision.
- **The changelog gains no entry for S02.** Nothing shipped; the milestone's
  entry at ship time says the slice was descoped.
