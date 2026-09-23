---
class: gated
---

# First release — vision

**Input document for `/jk:assess`.** Not a plan: a statement of the outcome, the
current state measured rather than assumed, and the decisions a plan will have
to take. The milestone decomposition below is a proposal to argue with.

Scope note: [`docs/plans/open-source-launch/vision.md`](../open-source-launch/vision.md)
covers how a release is *built and delivered* — signing, notarisation,
installers, the tag-driven pipeline — having absorbed the installers vision on
2026-09-23. This document covers **what is in it**: which surfaces ship
and which are held back. The two are independent and can run in either order.

## The observation

Poly is closer to a first release than its surface area suggests, and the gap is
not missing features. It is that the product currently presents three things a
first release should not have to defend: a second main view that is not
finished, a second tier of documentation that doubles the maintenance surface,
and a bibliography with measured defects in it.

All three are subtractions. That is what makes this a good release-shaped piece
of work — nothing here needs designing, only deciding and removing.

## What is actually there

### The two views are called Desk and Cloth

There is no "fabric" identifier anywhere in the tree. `webui/index.html` carries
`aria-label="Poly plugin window — desk and cloth modes"` and two chips, `mCloth`
and `mDesk`. **This document assumes "fabric view" means the Cloth view**; if it
means something else, everything in §1 changes.

**Cloth is not purely presentational, and this is the finding that matters
most.** Two things are gated to it in `webui/ui.js`:

| Gated to Cloth | Line | What it is |
|---|---|---|
| `capCtl` — capture bars + Arm | `capCtl.classList.toggle('show', m === 'cloth')` | The MIDI capture controls |
| `learnBtn` | `learnBtn.classList.toggle('mode-hidden', m !== 'cloth')` | Reveals the three annotations inside `#cloth` |

So the premise "all functionality can now be accessed from the desk view" **is
not true as the code stands**. Hiding Cloth today would hide MIDI capture with
it.

The good news is that this is a small problem wearing a large coat. Chapter 16
documents capture as VST3 parameters 600 and 601 — Export Trigger and Capture
Length — so the *capability* was never Cloth-bound; only the two chips are.
Ungating `capCtl` to the toolbar, where `exportBtn` already lives, restores the
premise. Learn is different: its annotations describe the woven visualisation
and have nothing to say about the Desk, so Learn leaves with Cloth rather than
moving.

No end-to-end spec references `cloth`, `mCloth` or `mDesk`, so the e2e suite
does not constrain this. `guide-using-poly.mdx` documents the Cloth/Desk toggle
in prose and its screenshot is already Desk.

### The theory deep dives are twelve pages with eleven test files attached

`site/src/content/docs/theory-*.mdx` — twelve files, presented as a
`Theory Deep Dives` nav group in `site/astro.config.mjs`. The rest of the site
points at them from four kinds of place:

- eleven chapters carry a `:::note[Theory deep dive]` callout
- `about-this-guide.mdx` explains what they are
- `appendix-references.mdx` names them in its opening sentence
- `theory-counterpoint-overview.mdx` is the section's own front door

**And eleven test files assert their content**, which is the constraint that
shapes this whole milestone:

| Test file | `theory-` references |
|---|---|
| `theory-patch-conformance.test.mjs` | 71 |
| `scope-framing.test.mjs` | 19 |
| `theory-euclidean-guardrail.test.mjs` | 12 |
| `idiom-break-framing.test.mjs` | 11 |
| `prose-conformance-claims.test.mjs` | 10 |
| `theory-audit-claims.test.mjs` | 7 |
| plus `doc-conformance-wiring`, `literature-enrichment`, `citation-tier`, `chapter-euclidean-guardrail`, `prose-claim-helpers` | 2–5 each |

These are not incidental. They are the guards the theory-audit programme built,
and several of them were the *deliverable* of a milestone. Deleting the pages
retires them.

### The bibliography, measured last night

M002 of the verifiable-references programme recorded a verdict for all 107
entries, so this is known rather than estimated:

| | |
|---|---|
| Total anchors | 107 — 43 numbered, 64 Further Reading |
| Cited outside the bibliography | 106 |
| **Cited *only* by `theory-*` pages** | **66** |
| Cited by at least one non-theory page | **40** |
| Of those 40: Tier A | **39** (the only Tier B is `fr-linn-attack-2020`) |
| Of those 40: mismatches | **2** — `ref-1`, `ref-34` |

**Removing the deep dives does most of the reference reduction by itself**:
106 citations collapse to 40, and every Tier C entry in the guide — the YouTube
links, the marketing blogs, the study guides — is cited *only* from the deep
dives. The quality problem and the volume problem have the same solution.

Getting from 40 to the requested 10–20 is the part that is genuinely editorial,
and it has a real cost: the 40 survivors are distributed roughly one per
chapter, so a cut to 15 means most chapters keep no citation of their own.

**`ref-1` has to be fixed regardless of where the cut lands.** It is the guide's
most-cited reference — six non-theory chapters — it underpins the Euclidean
claim the whole generator rests on, and M002 established that it names
Toussaint's 2005 BRIDGES paper while linking arXiv:0705.4085, which is "The
Distance Geometry of Music" by eight authors in 2007. Shipping a first release
with that in it is the one thing in this document that is not a matter of taste.

## The tension a plan has to resolve

The two removals pull against each other, and neither the tests nor the
provenance check will let the contradiction pass silently.

- **Delete the `theory-*.mdx` files** → eleven test files lose their subject.
  The bibliography is then free to shrink to 15, because nothing cites the other
  92 entries.
- **Keep the files and merely unpublish them** → every test keeps working, but
  `jk-standards.yaml` points `research_provenance.doc_roots` at
  `site/src/content/docs`, so the deep dives' inline citations still have to
  resolve. The bibliography stays pinned near 107 and the reference reduction
  cannot happen.

**The owner has taken this decision: the deep dives are *deferred*, not
retired — they are intended for publication in a later release.** So they move
rather than being deleted, and the move has to leave them re-publishable.

Move them out of the published content root, so the pages stop rendering, the
provenance check stops scanning them, and the tests keep asserting against them
at a new path. It costs a path update in eleven test files and buys both
removals at once, with the proven guards intact.

### Where they move to, which is not a detail

Every deep dive opens with `import PolyPatch from '../../components/PolyPatch.astro'`.
That relative path is what decides the destination:

| Destination | `../../components` resolves to | Imports survive? |
|---|---|---|
| `site/src/content/docs/` (today) | `site/src/components` | — |
| **`site/src/content/theory/`** | **`site/src/components`** | **yes, byte-identical** |
| `docs/theory/` | `components` (repo root, does not exist) | no |

`docs/theory/` would require rewriting the import in all twelve files, and
rewriting it back to republish — which is precisely the cost that "deferred"
is supposed to avoid. A sibling directory under `site/src/content/` is the same
depth as today, so the imports do not move at all.

It also renders nothing. `site/src/content.config.ts` defines exactly one
collection, `docs`, via Starlight's `docsLoader()`. A sibling `theory/`
directory is not a collection, so Astro will not build it — and republishing
later becomes a config change rather than an edit to twelve files.

One fragility to record rather than rely on: `jk-standards.yaml` scopes both
`doc_roots` and `research_provenance.doc_roots` to `site/src/content/docs` with
`.mdx`, so a sibling directory falls outside both. That is the behaviour this
plan wants, but it is a consequence of a path list rather than an expressed
intent. The move should say so in that file's comments, so a later widening of
the root is a deliberate act rather than a surprise.

## How this reconciles with verifiable-references

Three milestones of that programme are still open, and this vision changes what
they are worth. A plan should reconcile rather than run both:

- **M003 (the archive exists)** — scoped to archiving what can be archived
  across 107 entries. Against 15 entries it is a much smaller milestone, and
  most of the 19 browser-worklist items belong to references that would be cut.
- **M004 (nothing unreviewable is cited)** — largely *solved* by removing the
  deep dives, because every Wikipedia, YouTube and study-guide citation lives
  there. Its VR11 Wikipedia policy question may evaporate entirely.
- **M005 (one bibliography, one standard)** — a 15-entry list is trivially one
  list. The milestone becomes a formatting pass rather than a merge.

The M002 manifest at `site/src/data/references.json` is the input that makes the
editorial cut cheap: every candidate already carries its obtainability, its
description verdict, and its identifier. Because the deep dives are deferred
rather than retired, the manifest keeps earning against all 107 entries and not
just the 15 that ship — which is the argument for the deep dives taking a
bibliography with them rather than losing their citations.

## Decisions a plan must take

1. **Does "fabric view" mean Cloth?** Everything in §1 assumes yes.
2. **Hidden or removed?** A feature flag that hides the chip, versus deleting
   the Cloth markup, canvas and draw loop. Hiding is reversible and leaves dead
   code in the shipped bundle; removing is honest and is work to undo.
3. **Where do the capture controls go?** Recommended: ungate `capCtl` so it sits
   in the toolbar beside Export, which is where a user would look for it anyway.
4. ~~**Retire or defer the deep dives?**~~ **Taken: deferred.** They move to
   `site/src/content/theory/` and the eleven test files are repointed. What
   remains open is the one thing deferral creates — see decision 8.
5. **What is the target reference count, and what is the rule?** "10–20" needs a
   principle a later editor can apply — for example *one anchor per chapter,
   Tier A only, obtainable without an institution* — rather than a list someone
   chose once.
6. **Do the chapters keep inline citations at all?** If the bibliography drops
   to 15, most chapters' `See also refs` lines lose their targets and become
   edits in their own right.
7. **Is `ref-1` corrected, or cut?** Correcting it means pointing at the real
   2005 BRIDGES paper; cutting it means the Euclidean claim rests on
   `fr-toussaint-2013` and `ref-44` instead.
8. **What bibliography do the deferred deep dives keep?** This is the question
   deferral creates and it has no default. 66 anchors are cited *only* from the
   deep dives. If the published bibliography drops to 15 and the deep dives keep
   pointing at it, those 66 citations are dangling the day someone republishes —
   and nothing will warn about it in the meantime, because the provenance check
   will no longer scan them.

   The recommendation is that the move takes a bibliography with it: a
   `theory-references.mdx` alongside the pages, carrying the entries only they
   cite. M002's manifest already holds an obtainability verdict, a description
   verdict and an identifier for all 107, so this preserves that work instead of
   discarding two thirds of it. The alternative — let them dangle and repair on
   republication — throws away a milestone's output and defers a known breakage
   into a release nobody has scheduled.

## Proposed milestone decomposition

To argue with, not to accept.

| Milestone | Outcome | Depends |
|---|---|---|
| **R1 — Desk is the only view** | Cloth is not reachable in a shipped build, capture controls are in the toolbar, and a test fails if a mode chip returns | — |
| **R2 — The deep dives stop shipping** | No `theory-*` page renders, no link or callout points at one, the guards that assert their content still run against the new path, and the moved bundle carries the references only it cites | — |
| **R3 — A bibliography a reader can finish** | 10–20 entries, every one Tier A and obtainable, every inline citation resolving, `ref-1` correct | R2 |
| **R4 — The release is defensible** | Changelog, version, and a build of exactly what the docs describe | R1, R2, R3 |

R1 and R2 are independent. R3 depends on R2 because the deep dives are what pin
the bibliography's size.

## Out of scope

- The installer, signing and notarisation pipeline — that is the installers
  vision, and it is the other half of "first release"
- Any change to the engine, the preset set, or the parameter surface
- Rewriting chapter prose beyond what removing a callout or a citation requires
- Deciding Poly's long-term documentation strategy; this is a release cut, and
  the deep dives being unpublished is not a judgement on their quality — they
  are deferred to a later release by an explicit decision, and the move is
  designed so that republishing them is a configuration change
- Actually republishing them. R2 leaves them buildable, not built.
