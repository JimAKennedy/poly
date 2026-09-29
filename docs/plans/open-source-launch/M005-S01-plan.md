# M005/S01 — A release body a musician reads

**Slice:** M005/S01 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS28 (the Release body is 15,000 words of engineering narrative),
OS29 (nothing tells a user what is known to be wrong or which hosts work)
**Depends:** M001/S01 (done), M003/S01 (done).
**Decisions consumed:** `M005-decisions.md`, 2026-09-29 — a `### For
musicians` block at the top of each changelog version; a 400-word ceiling;
hand-written known issues plus a `known-issue` label.

## Task status

- [x] Task 1 — The generator emits the musician block and nothing else, the
      contract holds its shape and ceiling, and 0.2.0 has the block (OS28)
- [ ] Task 2 — The block names the supported hosts and the known issues, the
      label exists, and the slice closes (OS29)

## Definition of Done

- [ ] The owner has decided where musician-facing notes live
- [ ] The Release body for the first version names what Poly does, the
      supported hosts, the known issues, and how to report one
- [ ] The engineering narrative is kept, reachable from the notes, and not the
      Release body
- [ ] `check-release-workflow.mjs` asserts the body's source

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `guards` | `bash scripts/check-guards.sh` |

The task's own check, run first: `node --test scripts/check-release-workflow.mjs`.

## The block

Directly under `## [0.2.0] - unreleased` and before `### Fixed`:

```markdown
### For musicians

#### What Poly is

<the positioning sentence, then two or three sentences: four to eight lanes,
each its own cycle and Euclidean pattern; 45 presets from named drumming
traditions; MIDI only, into the drum instrument you already have; the same
settings always produce the same groove. Install: the README's Download
section, linked.>

#### Supported hosts

<Cubase Pro 15 on macOS and Cubase 14 on Windows, measured; other VST3 hosts
untested; Logic Pro not supported and why, one clause.>

#### Known issues

- Only Cubase has been measured …
- This build is unsigned: macOS quarantine, Windows SmartScreen …
- The editor is a fixed 1160×760 window …
- Anything else: <link to open issues labelled known-issue>

#### Report a problem

<open an issue, linked; security reports per SECURITY.md, linked.>
```

The generator emits everything from the line after `### For musicians` to
the line before the next `### `, then a blank line and
`Full changelog: https://github.com/JimAKennedy/poly/blob/v<version>/CHANGELOG.md`.

## Task 1 — The generator emits the musician block and nothing else, the contract holds its shape and ceiling, and 0.2.0 has the block (OS28)

**Files:** modify `scripts/gen-release-notes.mjs`,
`scripts/check-release-workflow.mjs`, `CHANGELOG.md`, `RELEASING.md`;
create `docs/plans/open-source-launch/evidence/M005-S01.md`.

1. In `check-release-workflow.mjs`:
   - Rename `gen-release-notes emits a non-empty body for shipping version
     0.1.0` to `gen-release-notes fails loud on a version with no For
     musicians block (0.1.0 was never tagged)` and assert it throws with
     exit 1.
   - Change `emits a non-empty body for the version CMakeLists.txt declares`
     to assert the output: is at most 400 words (`split(/\s+/)` of the
     trimmed output); contains the four `#### ` headings `What Poly is`,
     `Supported hosts`, `Known issues`, `Report a problem`; ends with the
     `Full changelog:` link for `v<version>`; and contains no `### ` heading
     and no `(open-source-launch M` tail, which every engineering entry
     carries.
   Run: red on both.
2. Rewrite `gen-release-notes.mjs`: find the version's H2 as today; within
   it find `### For musicians`; if absent, print
   `No "### For musicians" block in the CHANGELOG section for "<version>"`
   to stderr and exit 1; emit the lines after it up to the next `### ` or
   the next H2, trimmed, then the blank line and the link. Update the
   header comment: it is CI wiring now, the body is the musician block, and
   the engineering section is one link away.
3. Write the 0.2.0 block with the `What Poly is` and `Report a problem`
   sections complete, and the `Supported hosts` and `Known issues` headings
   each carrying its first real sentence — `Supported hosts` the Cubase
   sentence, `Known issues` the Cubase-only item — so the changelog never
   holds a placeholder; task 2 completes both sections.
4. Run the contract: green. Run `node scripts/gen-release-notes.mjs 0.2.0`
   and record its word count. `node scripts/gen-release-notes.mjs 0.1.0`
   exits 1 with the message.
5. `RELEASING.md`: a paragraph after the introduction: each version's
   changelog section opens with `### For musicians`, the release body is
   that block and a link, the contract holds it to 400 words and four
   headings, and a version without the block fails the release.
6. Run `format`, `doc-discipline`, `guards`. Evidence. Tick. Commit with
   `Rows: OS28`.

## Task 2 — The block names the supported hosts and the known issues, the label exists, and the slice closes (OS29)

**Files:** modify `CHANGELOG.md`, `scripts/check-release-workflow.mjs`,
`docs/plans/open-source-launch/ledger.md`. The `known-issue` label, through
the API.

1. Add to the contract test's CMake-version case: the `Supported hosts`
   section names `Cubase` and `Windows` and `macOS`; the `Known issues`
   section has at least three list items and links
   `issues?q=is%3Aissue+is%3Aopen+label%3Aknown-issue`. Run: red.
2. `gh label create known-issue --description "A defect a musician will meet in the current release; listed in the release notes" --color D93F0B`;
   read it back with `gh label list`.
3. Complete the two sections in the 0.2.0 block per the shape above: hosts
   from the README's table; the three known items and the label link.
4. Run the contract: green; word count under 400. Run `format`,
   `doc-discipline`, `guards`. Tick every DoD box here and in the ledger;
   set the slice `done`; OS29 `done`. Evidence with the label read-back,
   the final word count and the DoD-to-task table. Run
   `jk-standards ledger`. Commit with `Rows: OS29`.

## Self-review

| DoD | Task |
|---|---|
| The owner decided where the notes live | the decisions file |
| The body names what Poly does, hosts, known issues, how to report | 1 (what, report) and 2 (hosts, known issues), held by the contract |
| The engineering narrative is kept, reachable, and not the body | 1 — untouched below the block; the link; the no-engineering assertion |
| `check-release-workflow.mjs` asserts the body's source | 1 and 2 |

| Row | Task | Verification produced |
|---|---|---|
| OS28 | 1 | the generator prints the block; the ceiling test; the engineering entries unchanged |
| OS29 | 2 | Supported hosts from OS14's evidence; Known issues linking open issues by label |

No placeholders.
