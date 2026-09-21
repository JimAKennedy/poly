# M002/S01 — The site stops pointing at them

**Slice:** M002/S01 in `docs/plans/first-release/ledger.md`
**Rows:** FR06 (the nav group), FR07 (the chapter callouts), FR08
(`about-this-guide`), FR09 (`appendix-references`)
**Classification:** bounded. Prose and config edits to files that already
exist. No new mechanism.

## Task status

- [ ] 1. The navigation stops offering a theory section
- [ ] 2. The twelve chapter callouts go
- [ ] 3. The four remaining links into the deep dives go

## Definition of Done

Copied verbatim from the slice:

- [ ] No navigation entry offers a theory deep dive
- [ ] No chapter carries a `:::note[Theory deep dive]` callout
- [ ] No prose anywhere on the site tells a reader the deep dives exist
- [ ] The site builds with no broken internal link

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## The surface, measured

The ledger undercounts this. Sixteen `.mdx` files link into the deep dives, not
twelve:

| Where | Count | Form |
|---|---|---|
| chapters 02–13 | **12** | `:::note[Theory deep dive]` callout, a pure pointer |
| `14-synthesis` | 3 links | into `#what-breaks-the-idiom` anchors |
| `appendix-presets` | 2 links | into `#what-breaks-the-idiom` anchors |
| `about-this-guide` | 1 link | to the counterpoint overview |
| `appendix-references` | 1 link | to the counterpoint overview |
| `site/astro.config.mjs` | 1 group | `Theory Deep Dives`, 12 entries |

Links **out of** the deep dives are not this slice's business: 265 point at
shipping pages and 14 at each other, and since shipping pages do not move and
all twelve deep dives move together, every one resolves again on republication.

---

## Task 1 — The navigation stops offering a theory section

**Consumes:** nothing. **Produces:** a sidebar with no theory entry.

1. Remove the `Theory Deep Dives` group and its twelve entries from
   `site/astro.config.mjs`.
2. Run `npm --prefix site test` and `bash scripts/check-doc-conformance.sh`.
   The pages still exist on disk, so nothing that reads them should care —
   if something does, that is a finding worth reporting rather than working
   around, because it means a guard was asserting navigation rather than
   content.
3. Run `format`, `site-unit`, `doc-conformance`, `doc-discipline`. Commit with
   `Slice: M002/S01`, `Rows: FR06`.

## Task 2 — The twelve chapter callouts go

**Consumes:** nothing. **Produces:** chapters that do not advertise a section
the reader cannot reach.

Each callout is a pure pointer — "For the rules of additive counterpoint stated
explicitly … see [Additive Counterpoint: Balkan]" — so there is no claim inside
one to preserve. They are deleted whole, `:::note[Theory deep dive]` through the
closing `:::`.

1. Delete the callout from each of the twelve chapters: `02` through `13`.
   Take the blank line that follows it too, so no chapter is left with a double
   gap between paragraphs.
2. Confirm the count reached zero by reading the tree, not by trusting this
   plan: `grep -rl ':::note\[Theory deep dive\]' site/src/content/docs/*.mdx`
   must return nothing.
3. Run `format`, `site-unit`, `doc-conformance`, `doc-discipline`. Commit with
   `Slice: M002/S01`, `Rows: FR07`.

## Task 3 — The four remaining links into the deep dives go

**Consumes:** nothing. **Produces:** the last of the inbound links, and the
slice closes.

Four documents link in without carrying a callout. Two of them carry meaning
and are reworded rather than cut; two are plain mentions and go.

1. **`14-synthesis.mdx`** — one sentence sends the reader to three
   `#what-breaks-the-idiom` anchors. Reword so it still states that each
   tradition treats different things as idiom-breaking, and drop the three
   links. The claim is the point; the pointer is not.
2. **`appendix-presets.mdx`** — two admonitions end with "See [what breaks the
   gamelan idiom]" and the Balkan equivalent. Same treatment: the sentence
   keeps saying the patch is a deliberate cross-cultural break, and the link
   goes.
3. **`about-this-guide.mdx`** — the sentence says the counterpoint overview
   "explains the convention" of where a caveat sits. Rewrite so the convention
   is stated rather than delegated, or drop the clause if the surrounding text
   already states it; read the passage before choosing, because the right
   answer depends on what the neighbouring sentences already say.
4. **`appendix-references.mdx`** — the opening says "Nothing in this guide —
   including the Theory Deep Dives section — is original research." Remove the
   parenthetical naming of the section; the sentence's claim is about the whole
   guide and survives without it.
5. Confirm no `.mdx` outside `site/src/content/docs/theory-*` still references a
   deep-dive slug, by grepping the tree for all twelve slugs.
6. Run `format`, `site-unit`, `doc-conformance`, `doc-discipline`. Append the
   slice's evidence, tick every definition-of-done box, set FR06, FR07, FR08 and
   FR09 `done`, set the slice `done`, run `jk-standards ledger`, and commit with
   `Slice: M002/S01`, `Rows: FR08, FR09`.
