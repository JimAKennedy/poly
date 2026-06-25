# S01: Astro Starlight Scaffold and Design System

**Goal:** Scaffold an Astro Starlight site in site/ with the Poly design system: fonts (Source Serif 4, Inter, JetBrains Mono), colour palette, two-column layout, and one sample chapter rendering correctly at localhost:4321.
**Demo:** astro dev serves a working site at localhost:4321 with one sample chapter, correct fonts, colour palette, and two-column layout

## Must-Haves

- astro dev serves at localhost:4321 with correct fonts, colour palette, sidebar navigation stub, and a sample chapter page demonstrating the two-column layout

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Initialize Astro Starlight project** `est:10min`
  Create the Astro Starlight project in site/ using npm create astro with the starlight template. Install dependencies. Verify the default site builds and serves.
  - Files: `site/package.json`, `site/astro.config.mjs`, `site/tsconfig.json`
  - Verify: cd site && npm run build succeeds with exit code 0

- [x] **T02: Install fonts and create design system CSS** `est:15min`
  Install @fontsource/source-serif-4, @fontsource/inter, @fontsource/jetbrains-mono. Create site/src/styles/custom.css with CSS custom properties for the full colour palette and typography scale from the plan spec.
  - Files: `site/package.json`, `site/src/styles/custom.css`
  - Verify: Build succeeds and custom.css contains all colour variables and font-face declarations

- [x] **T03: Configure Astro Starlight with sidebar and layout** `est:15min`
  Update astro.config.mjs with site title, sidebar navigation entries for all 17 chapters + 2 appendices, custom CSS import, and Starlight theme overrides for the Poly colour palette.
  - Files: `site/astro.config.mjs`
  - Verify: astro build succeeds and sidebar entries appear in the built HTML

- [x] **T04: Create sample chapter page and verify dev server** `est:10min`
  Create a sample chapter MDX page (01-foundations.mdx) with placeholder prose, heading hierarchy, and a two-column layout test. Verify it renders correctly via astro dev at localhost:4321.
  - Files: `site/src/content/docs/01-foundations.mdx`
  - Verify: astro dev serves at localhost:4321 and the sample chapter renders with correct fonts and layout

## Files Likely Touched

- site/package.json
- site/astro.config.mjs
- site/tsconfig.json
- site/src/styles/custom.css
- site/src/content/docs/01-foundations.mdx
