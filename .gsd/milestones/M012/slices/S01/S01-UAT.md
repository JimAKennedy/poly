# S01: Astro Starlight Scaffold and Design System — UAT

**Milestone:** M012
**Written:** 2026-06-25T00:37:13.050Z

## UAT: S01 Astro Starlight Scaffold and Design System

### Prerequisites
- Node.js 20+ installed
- Run `cd site && npm install`

### Test Steps

1. **Build succeeds**
   - Run `npm run build` in `site/`
   - Expected: 20 pages built, exit code 0

2. **Dev server serves**
   - Run `npm run dev` in `site/`
   - Navigate to `http://localhost:4321/poly/`
   - Expected: Landing page with hero section and card grid

3. **Fonts load correctly**
   - Open browser dev tools, inspect body text
   - Expected: Computed font is `Source Serif 4 Variable`, not system serif

4. **Colour palette applied**
   - Inspect CSS variable `--sl-color-accent`
   - Expected: `#01696f` (teal)

5. **Sidebar navigation complete**
   - Check left sidebar on any chapter page
   - Expected: Introduction, 15 numbered chapters, 2 appendices

6. **Chapter page renders**
   - Navigate to `/poly/01-foundations/`
   - Expected: H1 chapter title, H2 section headings, prose paragraphs with serif font
