# M001 — decisions

Append-only. One entry per decision that shaped the milestone, with the reason,
so a reviewer can see what was chosen on the owner's behalf and what the owner
chose themselves.

## 2026-09-21 — planning M001/S01 and M001/S02

Four questions were put to the owner before either slice was planned, each with
the measurement that made it answerable.

- **Q:** "Removed from the build, source retained" — retained how? —
  **A:** delete it; git history is the archive.
- **Decision:** the markup, the ~285-line cloth block in `ui.js` and the
  `#cloth` CSS are deleted outright — **Why:** the deep dives move because they
  have a scheduled return; Cloth does not. A build-time flag would ship two UI
  paths to maintain, and moving a JS view out of the build would rot it against
  the host interface it talks to.

- **Q:** Nine webui specs assert Cloth behaviour, `capture-timeline.spec.mjs`
  19 times. What happens to them? — **A:** port what tests capture, delete what
  tests the cloth.
- **Decision:** capture assertions retarget the toolbar chips; assertions about
  the loom canvas, the annotations and mode switching are deleted — **Why:**
  capture survives this milestone and its arm → capturing → complete coverage is
  worth keeping; the cloth-specific assertions describe something that will not
  exist.

- **Q:** The capline is the only per-bar capture progress readout. What replaces
  it? — **A:** the bars chip shows progress while capturing.
- **Decision:** `capBarsBtn` renders `3/8 bars` while `capState >= 2` —
  **Why:** one line in `updateCaptureChips()`, no new element, no layout
  pressure, and the information stays where the user is already looking. The
  chips already narrate state: Arm flips to Reset, the bars chip locks, Export
  marks ready.

- **Q:** Where should the guard against a returning mode chip live? —
  **A:** a source assertion in the normal gate.
- **Decision:** a node test asserting `index.html` carries no mode chip and
  `ui.js` no `setMode` — **Why:** `webui-e2e` is declared pre-push-exempt, so a
  Playwright-only guard would let a regression reach CI before anyone saw it.

### Taken on the owner's behalf

- **Decision:** `site-unit` is added to M001/S02's validation tokens, and the
  guard lands in `site/tests/` — **Why:** the slice owed `format`, `webui-e2e`
  and `doc-conformance`, none of which would run a node source assertion inside
  the pre-push gate. `site-unit` is already declared in `.jk/validations.yml`
  and already runs there. `site/tests/doc-conformance-wiring.test.mjs` is the
  precedent for asserting a repo-wide invariant from that directory.

- **Decision:** the specs are retargeted **before** Cloth is removed, not after
  — **Why:** capture assertions pointed at the toolbar pass with Cloth present
  and with Cloth gone, so the suite is never red between commits. The reverse
  order leaves a commit where nine specs fail.

### Findings recorded during planning

- Nine of the thirty webui Playwright specs reference Cloth. The ledger's
  M001/S02 does not mention them; they fall inside the slice because it owes
  `webui-e2e`, which cannot pass while they assert a view that is gone.
- The capture state display (`capline`) lives inside `#cloth`, so the
  entanglement is deeper than "two chips are gated". The state itself is
  already mirrored onto the toolbar by `updateCaptureChips()`; only the per-bar
  progress readout is unique to the capline.

## 2026-09-21 — judgment calls during M001/S01

- **Decision:** the existing spec `the capture control cluster is Cloth-only`
  was inverted rather than left in place beside a new case — **Why:** it asserts
  the behaviour FR01 names as the defect, so keeping it would have turned the
  suite red the moment the gating was removed, and two cases asserting opposite
  things is not a choice a later reader should have to arbitrate. The plan's
  step said "add a spec case"; inverting the contradicting one is what that step
  meant in a file that already had it.
