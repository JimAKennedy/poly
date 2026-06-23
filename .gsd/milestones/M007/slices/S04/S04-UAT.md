# S04: Lane Timing Offset — UAT

**Milestone:** M007
**Written:** 2026-06-22T21:05:41.794Z

## S04: Lane Timing Offset UAT

### Setup
1. Load Poly in Cubase, ensure transport is playing
2. Open the generic parameter editor to access Timing Offset per lane

### Test Cases

**TC1: Zero offset = no change**
- Set all Timing Offset knobs to center (0ms)
- Verify output sounds identical to before the update

**TC2: Positive offset = behind the beat**
- Set kick (L1) Timing Offset to +5ms
- Play and listen — kick should feel slightly lazy/behind

**TC3: Negative offset = ahead of the beat**
- Set hi-hat (L3) Timing Offset to -3ms
- Play and listen — hi-hat should feel slightly rushed/ahead

**TC4: Combined pocket feel**
- Kick +5ms, snare +2ms, hi-hat -3ms
- Play and listen — should create a human groove pocket feel that swing alone can't achieve

**TC5: Extreme values**
- Set one lane to +20ms, another to -20ms
- Verify no audio glitches, clicks, or dropped notes
