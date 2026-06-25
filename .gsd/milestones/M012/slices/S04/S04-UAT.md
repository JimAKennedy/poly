# S04: Live Code Extraction Pipeline — UAT

**Milestone:** M012
**Written:** 2026-06-25T01:30:46.217Z

## UAT: Live Code Extraction Pipeline

### Prerequisites
- Node.js installed, `npm ci` in `site/`

### Test Steps

1. **Build the site**
   ```bash
   cd site && npx astro build
   ```
   - [ ] Build completes with zero errors

2. **Check Chapter 1 (Foundations)**
   - Open `dist/01-foundations/index.html`
   - [ ] Code block shows the `euclidean()` function with Bresenham-style distribution
   - [ ] Header links to GitHub source file

3. **Check Chapter 5 (Gamelan)**
   - Open `dist/05-gamelan/index.html`
   - [ ] Code block shows kotekan complement generation (sangsih from polos)

4. **Check Chapter 8 (Minimalism)**
   - Open `dist/08-minimalism/index.html`
   - [ ] Code block shows drift-accumulator logic from engine.cpp (not lane_state.cpp)

5. **Check Chapter 14 (Synthesis)**
   - Open `dist/14-synthesis/index.html`
   - [ ] Code block shows envelope shape evaluation functions

6. **Check Chapter 15 (Compositional Grammar)**
   - Open `dist/15-compositional-grammar/index.html`
   - [ ] Code block shows macro resolution with density/complexity scaling

7. **Modify a region and rebuild**
   - Add a comment inside a region in engine source
   - Rebuild the site
   - [ ] The change appears in the rendered page
