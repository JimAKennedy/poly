# M003/S01 — About This Guide

**Slice:** M003/S01
**Ledger:** `docs/plans/theory-audit/ledger.md`

The guide has never said what it is. The audit supplies a four-paragraph
repositioning statement and judges it defensible; this slice gives it a home and
makes it reachable from every page that needs it.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; each task's own commit ticks its box.

- [ ] Task 1 — The About page, its scope exclusions, and the lock host (F36)
- [ ] Task 2 — Reachability from the introduction and all twelve theory pages (F24)

## Definition of Done

Copied verbatim from the slice. Both tasks below argue against *this* text.

- [ ] An "About This Guide" page carries the audit's repositioning statement
- [ ] It is reachable from the introduction and from all twelve `theory-*.mdx`
      pages
- [ ] It names the guide's deliberate scope exclusions, including the
      son-clave/rumba-clave precedence debate

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

Run `node --test site/tests/scope-framing.test.mjs` in the inner loop once
Task 1 creates it.

**Gate ordering.** `format` runs the `ledger` pre-commit hook, which fails while
a slice claiming `done` has no evidence file. Task 2 closes the slice, so its
order is: `site-unit` and `doc-conformance` first, then the evidence, then
`format`.

## Context

**The statement is the audit's, verbatim.** `docs/audits/poly_theory_audit.md`
§5 ("Proposed repositioning statement") supplies four paragraphs, beginning
"This guide is not a comprehensive review of ethnomusicological literature."
Reproduce them unchanged. F24's point is that *this* statement is the one the
audit judged defensible — a paraphrase would be a different claim wearing its
authority.

**The guide already says some of this, in one place.**
`theory-counterpoint-overview.mdx` opens with "**None of this is original
research.** Every deep dive is a summary of published scholarship…". The About
page is the global statement; the overview's is a local one about the deep dives
specifically. Do not delete or duplicate the overview's paragraph — Task 2 links
it upward instead, so the two agree by reference rather than by copy.

**Twelve theory pages, eleven of them uniform.** Every `theory-*.mdx` except
`theory-counterpoint-overview.mdx` opens with an italic preamble of the shape:

> `*Companion to [Chapter 3: Afro-Cuban](/03-afro-cuban/). This page assumes …*`

That preamble is where the About link goes on those eleven. The overview has no
such line and takes its link in its own opening instead. There is no shared
preamble component — the only import these pages share is `PolyPatch` — so a
component would still need importing in all twelve files and saves nothing.
Twelve direct edits is the honest shape.

**Cross-links point at pages, never at anchors that do not exist yet.** The
exclusions section links to `/06-indian-classical/` and to the theory pages as
*pages*. M003/S02–S05 have not landed, so any anchor naming a scope note or a
disclosure they will add is a forward reference — the defect class M002 spent
six slices removing. Link the page; let the reader find the section.

**Scope exclusions, not simplifications.** M003/S03 marks the Manding
flattening, the layakari conflation and the gamelan rules *in place*, and
M003/S02 states the Carnatic absence in Chapter 6. The About page names what is
out of scope and points at where each is detailed. It does not restate them, so
there is one source of truth per exclusion and nothing to drift.

**The lock host is new, and must be wired.** M001's cases live in
`theory-audit-claims.test.mjs` and M002's in `citation-tier.test.mjs`; both
declare themselves theirs. M003 gets `site/tests/scope-framing.test.mjs`. Task 1
adds it to `scripts/check-doc-conformance.sh` **and** to the `REQUIRED` array in
`site/tests/doc-conformance-wiring.test.mjs`. Nothing in CI runs
`npm --prefix site test` (poly issue #272), so a host that is not in the runner
is a lock that never runs in CI — which is exactly what M002/S06 had to correct
for `citation-tier.test.mjs` after five slices of it running nowhere.

**Claim helpers.** `site/tests/helpers/prose-claims.mjs` exports
`assertClaim(assert, claim, source)` and `registerClaimTests({ test, assert,
claims, loadSource, label })`. A claim is `{ id, file, rule, forbidden,
forbiddenRegex, present, presentRegex }`; `forbidden`/`present` match under
prose normalisation, the `*Regex` arms against raw source. Use the regex arms
for anything markup- or link-shaped.

**Reverting a test mutation** while a file holds uncommitted work from the same
task: use an inverse edit, not `git checkout --`. M002/S03 lost three edits that
way.

## Task 1 — The About page, its scope exclusions, and the lock host

**Creates:** `site/src/content/docs/about-this-guide.mdx`,
`site/tests/scope-framing.test.mjs`
**Modifies:** `site/astro.config.mjs`, `.github/docs-drift-map.yml`,
`scripts/check-doc-conformance.sh`,
`site/tests/doc-conformance-wiring.test.mjs`,
`docs/plans/theory-audit/ledger.md`
**Rows:** F36

### Steps

1. **Write the failing case host.** Create `site/tests/scope-framing.test.mjs`
   with a header comment stating that it hosts M003's scope-and-framing cases,
   that M003/S01 creates it, and that later M003 slices extend it. Import
   `registerClaimTests` from `./helpers/prose-claims.mjs`, resolve `DOCS` as
   `join(dirname(fileURLToPath(import.meta.url)), '..', 'src', 'content', 'docs')`,
   and define `loadSource = (file) => readFile(join(DOCS, file), 'utf8')`.

   Add one claim to a `CLAIMS` array:

   - `id: 'S01-F36'`, `file: 'about-this-guide.mdx'`
   - `rule`: ledger F36 — whether rumba clave predates or postdates son clave is
     debated (Acosta 2004, Moore 2006); the audit judges the guide may
     legitimately sidestep it under the repositioning frame, so the sidestep is
     declared on the About page rather than left silent
   - `present`: `['rumba clave predates or postdates', 'Acosta', 'Moore']`

   Register it with `registerClaimTests({ test, assert, claims: CLAIMS, loadSource })`.

2. **Run it and watch it fail** — `node --test site/tests/scope-framing.test.mjs`
   fails because `about-this-guide.mdx` does not exist. That is an *error*
   opening the file, not an assertion failure. Both prove the case is live, but
   the plan expects the file-not-found form here and the assertion form from
   step 4 onward; if step 4 leaves you still seeing file-not-found, the page is
   in the wrong directory.

3. **Create the page**, `site/src/content/docs/about-this-guide.mdx`, with
   front-matter `class: gated`, a `title` of `"About This Guide"`, and a
   `description`. Under a `## What this guide is` heading, reproduce the audit's
   four paragraphs from `docs/audits/poly_theory_audit.md` §5 **unchanged**,
   introduced by one sentence of the guide's own saying the statement is adopted
   from the 2026 external music-theory audit.

4. **Add the exclusions section** under `## What this guide does not cover`:

   - The son-clave/rumba-clave precedence debate, in full: whether rumba clave
     predates or postdates son clave is debated in the literature — Acosta
     (2004) and Moore (2006) — and this guide does not take a position, because
     its patterns and alignment rules are unaffected by the answer.
   - A short list of the other deliberate exclusions, each linking to the page
     where it is detailed rather than restating it: the Carnatic tala system
     (link `/06-indian-classical/`), and the pedagogical simplifications marked
     on the theory pages (link `/theory-counterpoint-overview/`).

   Link pages, never anchors. M003/S02–S05 have not landed and their headings do
   not exist yet.

5. **Run the case and watch it pass.**

6. **Declare the page in the drift map.** A new `class: gated` doc must be
   mapped or declared or `doc-completeness` fails — only `class: archived` is
   exempt. Add a `cannot_drift` entry for
   `site/src/content/docs/about-this-guide.mdx` in `.github/docs-drift-map.yml`,
   alongside the `introduction.mdx` entry it sits next to in the guide, with a
   reason of the same shape: the page states the guide's scope and provenance
   rather than tracking any code surface, so touch-correlation has nothing to
   enforce — its accuracy is gated by cases `S01-F24` and `S01-F36` instead.

7. **Add the sidebar entry** in `site/astro.config.mjs`:
   `{ label: 'About This Guide', slug: 'about-this-guide' }`, immediately after
   the `Introduction` entry and before `Using Poly`, so it reads as front matter
   for the whole guide rather than as an appendix.

8. **Wire the new host.** Add `site/tests/scope-framing.test.mjs` to the
   `REQUIRED` array in `site/tests/doc-conformance-wiring.test.mjs` **first**,
   run that test and watch it fail with `check-doc-conformance.sh dropped
   required guardrail: site/tests/scope-framing.test.mjs`, then add the same
   path to the `TESTS` array in `scripts/check-doc-conformance.sh` and watch it
   pass. Adding `REQUIRED` first is what proves the wiring test defends the new
   host rather than merely passing beside it.

9. **Run** `format`, `site-unit` and `doc-conformance`, and read each exit code.
   `doc-conformance`'s test count must rise — the new host is now inside it.

10. **Close F36** in `docs/plans/theory-audit/ledger.md`: set its `Status` to
   `done`, name case `S01-F36` in its `Verification` cell, and correct its
   `Lands in` cell to the files this task modified. Use no `|` in any cell.

11. **Append to** `docs/plans/theory-audit/evidence/M003-S01.md`, creating it, in
    the format `/jk:next` section 4 gives. Name no commit SHA.

12. **Commit** with trailers `Plan: docs/plans/theory-audit/ledger.md`,
    `Slice: M003/S01`, `Rows: F36`.

## Task 2 — Reachability from the introduction and all twelve theory pages

**Modifies:** `site/src/content/docs/introduction.mdx`, all twelve
`site/src/content/docs/theory-*.mdx`, `site/tests/scope-framing.test.mjs`,
`docs/plans/theory-audit/ledger.md`,
`docs/plans/theory-audit/evidence/M003-S01.md`
**Consumes:** Task 1's About page and lock host
**Rows:** F24

### Steps

1. **Write the failing case.** Add `S01-F24` to `scope-framing.test.mjs` as a
   standalone `node:test` case, not a `CLAIMS` entry — it spans thirteen files
   rather than one. It must:

   - assert `about-this-guide.mdx` exists under `DOCS`
   - read `introduction.mdx` and every `theory-*.mdx` under `DOCS`, and assert
     each contains a link to `/about-this-guide/`
   - fail naming **every** file that lacks the link, not just the first, and
     report the count — thirteen near-identical edits is exactly where one gets
     missed

   Assert the theory-page set is discovered by globbing `theory-*.mdx` rather
   than hard-coded, so a thirteenth theory page added later is covered by
   construction. Assert the discovered count is `12`, so a glob that silently
   matches nothing cannot pass vacuously.

2. **Run it and watch it fail**, naming `introduction.mdx` and all twelve theory
   pages — thirteen files, none of them linking yet.

3. **Link from the introduction.** In `introduction.mdx`, add a sentence in the
   opening section, before `## Reading Order`, pointing at
   `[About This Guide](/about-this-guide/)` and saying in one clause what it
   covers — what the guide is, what it is not, and what it leaves out.

4. **Link from the eleven companion pages.** In each `theory-*.mdx` except
   `theory-counterpoint-overview.mdx`, extend the existing italic
   `*Companion to …*` preamble with a clause pointing at
   `[About This Guide](/about-this-guide/)`. Keep it inside the italic run so
   the preamble stays one visual unit.

5. **Link from the overview page.** `theory-counterpoint-overview.mdx` has no
   companion preamble. Add the link to its existing "**None of this is original
   research.**" paragraph, so the local statement points up at the global one
   rather than competing with it. Do not delete or reword that paragraph.

6. **Run the case and watch it pass**, and confirm it reports twelve theory
   pages discovered.

7. **Prove the lock bites.** Remove the link from one theory page, confirm
   `S01-F24` fails naming that file specifically, then restore it **with an
   inverse edit, not `git checkout --`**, and confirm the case passes again.

8. **Run** `site-unit` and `doc-conformance` and read their exit codes.

9. **Close F24 and the slice.** Set F24's `Status` to `done`, name case
   `S01-F24` in its `Verification` cell, and correct its `Lands in` cell to the
   files this task modified. Then tick all three Definition-of-Done boxes in the
   slice and in this plan's copy, tick this plan's Task 2 box, and set the slice
   `Status` to `done`.

10. **Append the evidence**, then run `format` last per the gate ordering, and
    `jk-standards ledger`.

11. **Commit** as one unit with trailers
    `Plan: docs/plans/theory-audit/ledger.md`, `Slice: M003/S01`, `Rows: F24`.
