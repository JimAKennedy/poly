# S06: Screenshots and Visual Polish

**Goal:** Add screenshot references to representative chapters and polish site visual design
**Demo:** At least 5 representative chapters have screenshots showing the Poly UI with patch values loaded

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Add PolyScreenshot calls to 5+ representative chapters** `est:30m`
  Add PolyScreenshot component calls to at least 5 chapters that best benefit from UI visuals — foundations, sub-saharan africa, afro-cuban, gamelan, and electronic. Place them at contextually appropriate points where seeing the Poly UI with that chapter's patch loaded adds value. Screenshots will show placeholder until actual images are added.
  - Files: `site/src/content/docs/01-foundations.mdx`, `site/src/content/docs/02-sub-saharan-africa.mdx`, `site/src/content/docs/03-afro-cuban.mdx`, `site/src/content/docs/05-gamelan.mdx`, `site/src/content/docs/09-electronic.mdx`
  - Verify: astro build succeeds; at least 5 chapters contain PolyScreenshot calls

- [x] **T02: Visual polish pass on site CSS** `est:45m`
  Review and improve custom.css for consistent spacing, typography, and component styling. Ensure PolyPatch tables, ListenFor callouts, CodeSnippet panels, and EuclideanDiagram circles all have consistent visual treatment. Check mobile responsiveness.
  - Files: `site/src/styles/custom.css`, `site/src/components/PolyPatch.astro`, `site/src/components/ListenFor.astro`, `site/src/components/EuclideanDiagram.astro`
  - Verify: astro build succeeds; dev server shows polished styling

- [x] **T03: Create screenshots directory and document screenshot workflow** `est:15m`
  Create site/public/screenshots/ directory structure. Add a brief README explaining how to capture and name screenshots for each chapter (from Cubase with patch loaded). This makes it easy to add real screenshots later.
  - Files: `site/public/screenshots/`
  - Verify: Directory exists; README explains naming convention

## Files Likely Touched

- site/src/content/docs/01-foundations.mdx
- site/src/content/docs/02-sub-saharan-africa.mdx
- site/src/content/docs/03-afro-cuban.mdx
- site/src/content/docs/05-gamelan.mdx
- site/src/content/docs/09-electronic.mdx
- site/src/styles/custom.css
- site/src/components/PolyPatch.astro
- site/src/components/ListenFor.astro
- site/src/components/EuclideanDiagram.astro
- site/public/screenshots/
