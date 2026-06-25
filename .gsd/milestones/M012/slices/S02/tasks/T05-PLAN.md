---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T05: PolyScreenshot component

Figure element with image and caption. When src image is missing or not yet provided, renders a styled placeholder with camera icon instead of a broken image. Uses figure/figcaption HTML.

## Inputs

- `Design spec: figure with caption, graceful fallback`

## Expected Output

- `site/src/components/PolyScreenshot.astro with props: src, caption`

## Verification

Component shows placeholder when src is missing; shows image when present
