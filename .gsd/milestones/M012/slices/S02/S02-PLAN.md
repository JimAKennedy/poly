# S02: Custom MDX Components

**Goal:** Build 5 custom Astro components (PolyPatch, ListenFor, CodeSnippet, EuclideanDiagram, PolyScreenshot) and verify them on a test page
**Demo:** A test page renders all 5 components: PolyPatch with lane table, ListenFor amber callout, CodeSnippet with static placeholder, EuclideanDiagram SVG circle, and PolyScreenshot with missing-image placeholder

## Must-Haves

- All 5 components render correctly on a test page; PolyPatch shows lane tables with teal styling; ListenFor shows amber callout; CodeSnippet renders static placeholder; EuclideanDiagram draws SVG circle from steps/hits; PolyScreenshot shows placeholder for missing images; dev server builds without errors

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: PolyPatch component** `est:20min`
  Teal-bordered callout box displaying patch title, optional preset name, and a lane parameter table rendered from child markdown table content. Uses Inter font, poly-patch-bg colour.
  - Files: `site/src/components/PolyPatch.astro`, `site/src/styles/custom.css`
  - Verify: Component renders with teal border, title, and table on dev server

- [x] **T02: ListenFor component** `est:10min`
  Amber-bordered callout box for listening guidance. Left border accent in gold/amber, warm background, child content as prose. Ear emoji prefix.
  - Files: `site/src/components/ListenFor.astro`, `site/src/styles/custom.css`
  - Verify: Component renders with amber border and warm background on dev server

- [x] **T03: CodeSnippet shell component** `est:15min`
  Static placeholder component for S02. Shows file path header and a placeholder message. Will be wired to the Vite region-tag extractor in S04.
  - Files: `site/src/components/CodeSnippet.astro`, `site/src/styles/custom.css`
  - Verify: Component renders dark panel with file header and placeholder text

- [x] **T04: EuclideanDiagram SVG component** `est:30min`
  SVG circle diagram rendering Euclidean rhythm onset distribution from steps/hits/rotation props. Filled circles for hits, open circles for rests, arranged on a circle. Implements Bjorklund algorithm in the component to compute onset positions.
  - Files: `site/src/components/EuclideanDiagram.astro`, `site/src/styles/custom.css`
  - Verify: E(7,12) renders 12 positions with 7 filled and 5 open; E(3,8) renders correctly

- [x] **T05: PolyScreenshot component** `est:15min`
  Figure element with image and caption. When src image is missing or not yet provided, renders a styled placeholder with camera icon instead of a broken image. Uses figure/figcaption HTML.
  - Files: `site/src/components/PolyScreenshot.astro`, `site/src/styles/custom.css`
  - Verify: Component shows placeholder when src is missing; shows image when present

- [x] **T06: Component test page and integration verification** `est:15min`
  Add a test/demo page (component-demo.mdx) that imports and exercises all 5 components with representative data. Verify the dev server builds and renders correctly.
  - Files: `site/src/content/docs/component-demo.mdx`
  - Verify: astro dev serves the page; all 5 components render; no build errors

## Files Likely Touched

- site/src/components/PolyPatch.astro
- site/src/styles/custom.css
- site/src/components/ListenFor.astro
- site/src/components/CodeSnippet.astro
- site/src/components/EuclideanDiagram.astro
- site/src/components/PolyScreenshot.astro
- site/src/content/docs/component-demo.mdx
