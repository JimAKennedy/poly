# M005 — The release notes are for musicians

**Review-gate report.** Generated from the ledger, `git log` and
`M005-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** The first thing a musician reads about a release is short, says
what Poly does, which hosts it supports and what is known to be wrong — and the
launch is listed where musicians look. Amended 2026-09-30: the owner is keeping
Poly to friends and family for now and does not want it advertised, so the
listings (S02) are descoped and accepted as not done; the vision's last clause
waits for a decision to launch publicly.

**Branch:** `milestone/M005-release-notes` · **Ledger:** `docs/plans/open-source-launch/ledger.md`

## The milestone ran in two passes

S01 shipped alone as [#346](https://github.com/JimAKennedy/poly/pull/346),
squashed into `main` as `0c339cc`, so that M006's first tag would carry the
musician body; that ordering was the owner's decision on 2026-09-29. S02
waited on that tag. When it existed and the run reached S02's deferred
questions, the owner descoped the slice. This branch now holds the descope
and this report.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M005/S01 | A release body a musician reads | OS28, OS29 | done |
| M005/S02 | The launch is listed where musicians look | OS30 | accepted (descoped 2026-09-30) |

OS28 and OS29 are `done`; OS30 is `accepted`, not done.

## Definition of done

S01, all satisfied:

- [x] The owner has decided where musician-facing notes live
- [x] The Release body for the first version names what Poly does, the
      supported hosts, the known issues, and how to report one
- [x] The engineering narrative is kept, reachable from the notes, and not the
      Release body
- [x] `check-release-workflow.mjs` asserts the body's source

S02, none claimed — the slice is accepted as not done:

- [ ] A product page exists on KVR Audio linking the Release and the site
- [ ] The release has been submitted to at least two outlets that cover free
      rhythm plugins
- [ ] Each listing uses OS21's sentence

## What changed

**The Release body is the changelog's For musicians block** (OS28). Each
version's section opens with the block; `gen-release-notes.mjs` emits it
and one link to the full changelog at the tag, and fails loud on a version
without it, which the never-tagged 0.1.0 section demonstrates in the
contract. The 0.2.0 block is 301 words against the owner's 400-word ceiling.

**The notes say which hosts work and what is known to be wrong** (OS29).
Cubase Pro 15 on macOS and Cubase 14 on Windows, nothing else measured,
Logic not supported and why; four known issues plus a `known-issue` label
query. The contract requires both sections.

**Proved by the first release.** `v0.2.0-rc.1`'s published body is this
block, 309 words with M006's `Signed:` line appended
(`docs/plans/open-source-launch/evidence/M006-S01.md`).

**The listings are not made** (OS30). Descoped by the owner: no
advertising while Poly stays with friends and family. The rows and the DoD
stay in the ledger, unticked, so a later launch decision reopens a slice
already shaped.

## Validation

S01's tokens, as re-run for #346 on `01bbeae` and by the ship gate; the
descope commit changes only planning documents.

| Token | Command | Result | On |
|---|---|---|---|
| `format` | `pre-commit run --all-files` | pass | `f23d158` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass | `01bbeae` |
| `guards` | `bash scripts/check-guards.sh` | pass — release contract 33/33 at the time, 37/37 now | `01bbeae` |
| `ledger` | `jk-standards ledger` | pass — 6 conform | `f23d158` |
| pre-push gate | `scripts/pre-push-check.sh` | pass | `01bbeae` |

## Traceability

**Pass one**, squashed into `main` as `0c339cc` by #346; hashes as they were
on the branch before the squash:

| Commit | Slice | Rows | Subject |
|---|---|---|---|
| `ebda595` | M005/S01 | | docs(plans): front-load M005's decisions and plan S01, deferring S02 |
| `adbd31a` | M005/S01 | OS28 | release: the Release body is the changelog's For musicians block, one link from the rest |
| `d416b5c` | M005/S01 | OS29 | release: the notes say which hosts work and what is known to be wrong |
| `01bbeae` | M005/S01 | | docs: the changelog records M005/S01 |

**Pass two**, on this branch:

| Commit | Slice | Rows | Subject |
|---|---|---|---|
| `f23d158` | M005/S02 | OS30 | docs(plans): descope M005/S02 — no advertising for now, by the owner's decision |

**No untraced commits** in either pass.

## What a reviewer should look at twice

### The milestone closes with a slice not done

S02 is `accepted`, which the ledger standard allows, with its definition of
done unticked. The vision's last clause is unmet by choice. The report says
so rather than presenting the milestone as complete in its original sense.

### No row cuts the final 0.2.0 tag

The programme has a pre-release and, by M001's decision, a changelog heading
that stays `unreleased` until 0.2.0 itself is tagged. No ledger row does
that tagging. It surfaced while framing S02's questions and is worth a row
somewhere — M007 or M009 are the candidates — before the friends-and-family
build is meant to be final.

### The `known-issue` label has no issues yet

The notes' fourth known-issues item links a query that is empty today. That
is correct, and it stays correct only if a defect a musician meets is given
the label when filed.

## Decisions

Copied from `M005-decisions.md` so the report stands alone.

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
