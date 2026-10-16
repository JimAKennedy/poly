---
class: gated
---

# M002 — decisions

Append-only. Every question asked at planning, its answer, and every choice made
on the user's behalf during execution.

## 2026-10-15 — planning M002/S01

- **Q:** `PresetConformance.EveryPresetHasExactlyOneLockedReferent` requires
  exactly one fixed lane per factory preset, but four of the formulae (àgà,
  mò.kóndí, mò.nzòlì, mò.mbénzélé) have several fixed parts, and the only ways
  to exempt a drawn lane — probability, mutation, drift, phrase gating — move
  its marks. A: change the test for the Central African category only (at
  least one locked lane, with the reason cited to p. 230), or B: build the
  drawn parts another way? — **A:** A
- **Q:** Approve the M002/S01 design: presets appended at indices 45–55;
  macros at identity; timeline hit counts equal to their marks; the grid test
  rendering through `Engine::renderRange`; the "African" string in the
  metronomic-tradition test replaced by both new names; the taxonomy doc's caps
  raised to 11 categories and 11 presets per category; every stale count site
  brought current; lane names ≤ 15 ASCII characters; WASM, `presets.json` and
  `counts.json` regenerated together; four tasks? — **A:** yes
- **Decision:** append, never insert — **Why:** `kWebPresetLaneNames`, the mock
  host, the taxonomy mapping, two conformance allow-lists and the WebUI's join
  of the WASM list to `presets.json` are all keyed by preset index
- **Decision:** macros complexity 0.5, density 0.5, syncopation 0, swing 0,
  tension 0, humanize 0 — **Why:** at those values `resolveMacros` leaves a
  Euclidean lane's hits, rotation and probability exactly as set
  (`lerp(a, b, 0) = a`), and swing and humanize 0 satisfy the
  metronomic-tradition test
- **Decision:** the grid test renders through `Engine::renderRange` after
  `resolveConstraints(base, resolveMacros(base))`, the production pipeline —
  **Why:** a test that re-derives patterns mirrors the builder it should check;
  rendering checks what the engine emits
- **Decision:** in Kponingbo and Bondo, which have no drawn lane, the pellet
  bells and the rattle carry `Role::AnchorPulse` — **Why:** `lockPresetReferent`
  then fixes that lane, and `lockReferentLane` derives its fixed pattern from
  the lane's own Euclidean recipe, so its marks do not move; the dì.kétɔ̀ lanes
  stay Euclidean so Rotation still moves the mould
- **Decision:** zɔ̀bɔ̀kɔ̀'s mò.kóngò is a drawn lane, not additive cells —
  **Why:** the note allows either; a drawn lane is immune to the macros, and
  nothing established that additive cells are
- **Decision:** the mock host's two missing presets (Rumba Clave, Clapping
  Music) are added with the eleven — **Why:** its tables are positional, so
  indices 45–55 cannot exist without 43 and 44
