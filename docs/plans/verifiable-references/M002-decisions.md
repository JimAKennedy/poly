# M002 — decisions

Append-only. One entry per decision that shaped the milestone, with the reason,
so a reviewer can see what was chosen on the owner's behalf and what the owner
chose themselves.

## 2026-09-20 — planning M002/S01 and M002/S02

Four questions were put to the owner before either slice was planned, with the
measurements that made them answerable: 43 numbered entries and 64 Further
Reading, 107 total; 78 Tier A, 13 B, 16 C; two entries carry a DOI, 44 carry any
URL, and **no** Further Reading entry carries one.

- **Q:** How deep should verification go before recording `unverified`, given
  107 entries of which 64 have no URL at all? — **A:** open every source that
  can be opened.
- **Decision:** the description verdict is reached by reading the document where
  the document can be reached, not by matching metadata — **Why:** the owner's
  call, and the strongest available reading of "against the source, not against
  the guide's entry". It would have caught `ref-9` directly rather than by luck:
  every metadata field there matched a real dissertation, and only the text
  showed the subject was African pianism rather than drumming. **Stated cost:**
  this is the long tail the milestone's own note warns about, and several
  scholarly hosts refuse automation outright, so a large share of entries will
  reach the browser worklist rather than being settled in this pass. Which
  entries went to the queue, and why, is recorded per entry.

- **Q:** How should the manifest point at archived PDFs without committing a
  personal path? — **A:** bare filename plus an environment-supplied root.
- **Decision:** the manifest stores `ref-9-oluranti-2012.pdf` and nothing more;
  `POLY_REFERENCES_ARCHIVE` supplies the root at read time, defaulting to a
  gitignored `.references/` in the project root — **Why:** `check-personal-paths`
  rejected the absolute form in the vision's own first draft, and this makes the
  guard green by construction rather than by review. The owner's Dropbox path
  lives in their shell, not in git.

- **Q:** How should the manifest be structured? — **A:** keyed by anchor, rich
  record.
- **Decision:** an object keyed by anchor id, each record carrying
  obtainability, evidence, check date, description verdict, note, identifier,
  price, and archive filename — **Why:** keying makes duplicate anchors
  structurally impossible rather than something a test has to exclude, and the
  definition of done requires every one of those fields to have somewhere to go:
  VR06 wants a price, VR07 wants what the source actually is.

- **Q:** How should S02's 107 entries land as commits? — **A:** one commit per
  section of the bibliography.
- **Decision:** roughly six to eight commits following the appendix's own
  grouping, each naming what it verified and what it could not — **Why:** a
  107-record diff is one no reviewer can read, and a halt mid-pass leaves a
  coherent partial state rather than losing the work.

### Deferred

- **Q:** The queue guard M001/S01 left dominated — generalise it into the
  worklist mechanism M003/S02 needs, or retire it? — **Deferred to:** M003's
  planning. It is not a blocker for M002: nothing in this milestone reads or
  writes that guard, and the decision wants M003's requirements in view.
