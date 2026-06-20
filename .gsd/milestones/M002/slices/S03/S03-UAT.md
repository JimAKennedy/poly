# S03: Swing and Humanization — UAT

**Milestone:** M002
**Written:** 2026-06-20T21:34:12.344Z

## UAT: Swing and Humanization

### Swing
- [ ] swingAmount=0 produces no timing offset
- [ ] swingAmount=0.5 shifts odd steps forward by stepPpq/6
- [ ] swingAmount=1.0 shifts odd steps by full triplet offset (stepPpq/3)
- [ ] Swing is deterministic across runs

### Humanize
- [ ] humanizeMs > 0 adds jitter to note positions
- [ ] Jitter stays within [-jitterPpq, +jitterPpq] bounds
- [ ] humanizeMs=0 means no jitter
- [ ] Different seeds produce different jitter patterns
- [ ] Humanize is deterministic for same seed

### Note Duration
- [ ] noteDuration=0 defaults to stepPpq * 0.5
- [ ] noteDuration > 0 sets exact duration in PPQ
- [ ] Works for staccato (short) and legato (long) values

### Determinism
- [ ] Swing + humanize output is block-size independent
- [ ] All features maintain golden test determinism
