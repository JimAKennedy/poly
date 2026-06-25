---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T04: EuclideanDiagram SVG component

SVG circle diagram rendering Euclidean rhythm onset distribution from steps/hits/rotation props. Filled circles for hits, open circles for rests, arranged on a circle. Implements Bjorklund algorithm in the component to compute onset positions.

## Inputs

- `Toussaint circle notation style, Bjorklund distribution algorithm`

## Expected Output

- `site/src/components/EuclideanDiagram.astro with props: steps, hits, rotation, radius`

## Verification

E(7,12) renders 12 positions with 7 filled and 5 open; E(3,8) renders correctly
