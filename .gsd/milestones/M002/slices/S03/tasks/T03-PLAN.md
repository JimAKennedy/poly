---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Implement humanize timing jitter

Apply humanizeMs as bounded PPQ jitter to each note. Convert humanizeMs to PPQ using tempo: jitterPpq = humanizeMs * tempo / 60000.0. Use deterministicRand(seed, laneId, absStep, channel=3) to generate a random offset in [-jitterPpq, +jitterPpq]. Apply after swing. Clamp to keep note within the current block boundaries.

## Inputs

- `engine/include/poly/types.h`
- `engine/include/poly/rng.h`

## Expected Output

- `Modified engine.cpp with humanize jitter using deterministic RNG`

## Verification

cd build && cmake --build . && ctest --output-on-failure
