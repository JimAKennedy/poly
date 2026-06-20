---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Implement PPQ-derived envelope phase calculation

Add computeEnvelopePhase(double ppqPosition, float periodBars, float phaseOffset) that derives phase from absolute PPQ. periodBars * 4.0 gives periodPpq (4 PPQ per bar in 4/4). Phase = fmod(ppqPosition / periodPpq + phaseOffset, 1.0). Handle negative fmod results. This must never accumulate.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/envelope.h`
- `engine/src/envelope.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
