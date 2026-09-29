# M004 — decisions

Every question `/jk:auto` asked before running, every answer, and every choice
taken on the owner's behalf. Append-only.

## 2026-09-29 — planning M004/S01, S02, S03 and S04

Measured before asking, on `main` at `0b4736a`. The About panel is empty:
no description, no homepage, no topics. `ROADMAP.md` lists four issue numbers,
all closed, and its Priority 3 table is empty; the repository has **no GitHub
milestones at all**, so a "milestone query" would link to an empty page. The
README puts Building before Installing, gives signing-secret provisioning its
own section, has no screenshot, never mentions the in-browser engine, and
carries six internal IDs; `CONTRIBUTING.md:16` carries two. `RELEASING.md`
does not exist. `.bg-shell/manifest.json` is tracked and ignored at once.
`docs/README.md` does not exist, and a new document under `docs/` must be
named in the drift map or `doc-completeness` fails. `CLAUDE.md:18` lists
"VSTGUI 4". The site's construction banner is mounted from
`astro.config.mjs`; its hero offers *Start Reading* and *GitHub*; no
playground page exists, and the in-browser engine plays inside chapters
through `PolyPreviewCard`, first in the Foundations chapter
(`/01-foundations/`). `guide-using-poly.mdx` says Poly ships in two formats
and names the bundles `Poly.vst3` and `Poly.component`; the release zips are
`poly-<tag>-<platform>.zip` and each extracts to `poly_plugin.vst3`. No site
test covers the hero, the banner or the config's description.

- **Q:** OS21: which one sentence says what Poly is? — **A:** "Poly is a
  free, open-source polymetric drum sequencer for your DAW: grooves grounded
  in real drumming traditions, a guide that cites where every preset comes
  from, deterministic output, and an engine that runs in your browser."
- **Decision:** that sentence, verbatim, in the README opening, the site's
  meta description, the About description and `CLAUDE.md`, held by a site
  test — **Why:** it names the four things a free Euclidean sequencer does
  not have, which is the difference the ledger says Poly loses a checklist
  without. Alternatives offered and not taken: "An open-source VST3
  instrument that turns the world's drumming traditions into evolving
  polymetric MIDI grooves." (shorter, tradition-led) and "Poly generates
  polyrhythmic drum grooves rooted in the world's drumming traditions: a VST3
  MIDI instrument with 45 documented presets, deterministic output, and a
  browser demo." (product-first).
- **Q:** OS19: may the run set the About panel through the GitHub API, with
  the sentence, `poly.jk.digital`, and the topics vst3, midi,
  euclidean-rhythm, polyrhythm, drum-machine, audio-plugin? — **A:** yes.
- **Decision:** `gh api` sets description, homepage and topics; the evidence
  records the read-back — **Why:** reversible from the settings page, and the
  read-back is the proof the row asks for.
- **Q:** OS27: where does the hero's *Try it* point? — **A:** the Foundations
  chapter.
- **Decision:** `/01-foundations/`, the first chapter with playable preview
  cards — **Why:** no new page, and the reader hears the engine after one
  paragraph of context.
- **Q:** OS26: remove the construction banner or replace it? — **A:** replace
  it with a pre-release notice.
- **Decision:** the banner says the first release is in preparation and
  links the Releases page; a site test forbids the construction text; M006
  retires the notice when it tags — **Why:** the owner's choice; the site is
  verified but the download does not exist yet, and a visitor should be told
  which.

### Taken on the owner's behalf

- **The roadmap links label queries and the ledgers, not milestone
  queries.** No GitHub milestone exists, and inventing one to link would be
  the enumeration problem in a new form. Each theme links an open-issues
  query by label; planned work points at `docs/plans/`. If milestones are
  created later, the roadmap can link them then.
- **A `check-front-door.mjs` guard, in `guards`,** asserts no `#NNN` issue
  reference in `ROADMAP.md` and no `D0NN`/`M0NN` identifier in `README.md`
  or `CONTRIBUTING.md`. One file for the two rows' guards, each assertion
  seen red before its fix.
- **`RELEASING.md` is created, not "kept unchanged".** OS22 assumed it
  existed; it does not. The README's signing section moves there whole, and
  the README's Building section links it for maintainers.
- **The README's install section keeps the `xattr` instruction** for
  unsigned builds until M009 (OS40) deletes it; OS22 reorders, it does not
  re-promise signing.
- **`docs/README.md` gets a guard too**, `check-docs-index.mjs` in `guards`:
  every top-level document and subdirectory under `docs/` must appear in the
  index, the same rule `check-scripts-readme.sh` applies to `scripts/`,
  because an index that silently omits new documents is worse than none. The
  index is declared `cannot_drift` in the drift map with that reason.
- **The hero's three actions replace both existing ones.** *GitHub* leaves
  the hero; the header's GitHub icon remains. The tagline stays.
- **The guide's install section loses the Audio Unit entirely** — the "two
  formats" sentence, the `Poly.component` copy, the AU build-from-source and
  `auval` prose — since OS17 declined it and no release contains it. The VST3
  build-from-source stays. A test forbids `Poly.vst3` and `Poly.component`
  returning.
- **`CLAUDE.md`'s ownership-transfer convention says "VST3"**, not
  "VST3/VSTGUI", in the same edit as the tech-stack line; no VSTGUI target
  exists.
- **S01 runs before S02** as the ledger orders, so the About description is
  set in S01 with the sentence S02 lands in the tree; the sentence is decided
  above, and the read-back in S01's evidence is the same text S02's test
  holds.
