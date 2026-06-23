# S01: Phrase System — UAT

**Milestone:** M007
**Written:** 2026-06-22T19:40:29.027Z

## Phrase System UAT

### Setup
1. Build and deploy Poly VST3
2. Load in Cubase, create instrument track

### Test Cases

- [ ] **Continuous mode**: With Len=0, pattern plays continuously (default behavior unchanged)
- [ ] **Phrase gating**: Set Len=8bt, Gap=4bt — pattern plays for 8 beats, silences for 4, repeats
- [ ] **Offset**: With Len+Gap active, adjust Ofs — play/gap windows shift in time
- [ ] **Multi-lane phrases**: Set different Len values per lane — lanes create polyrhythmic phrase offsets
- [ ] **Transport jump**: Jump playhead into a gap region — lane is correctly silent
- [ ] **Loop restart**: Loop a section — phrase pattern is deterministic across restarts
- [ ] **Phase alignment view**: Play/gap arcs visible and accurate during playback
