# S02: Custom MDX Components — UAT

**Milestone:** M012
**Written:** 2026-06-25T00:45:55.179Z

## S02 UAT: Custom MDX Components

### Prerequisites
- `cd site && npm run dev` serves at localhost:4321

### Tests

1. **PolyPatch rendering**
   - Navigate to `/poly/component-demo/`
   - Verify teal header bar with "Ewe Bell Ensemble (Agbekor Framework)" title and italic preset name
   - Verify 4-row lane parameter table renders with teal column headers and proper alignment

2. **ListenFor rendering**
   - On same page, verify amber/gold left-bordered callout with "WHAT TO LISTEN FOR" header
   - Verify prose content renders in serif font inside the box

3. **CodeSnippet shell**
   - Verify dark panel with `engine/src/euclidean.cpp` file header and `bjorklund` region label
   - Verify placeholder text mentions the file and region
   - Verify caption appears below the panel

4. **EuclideanDiagram SVG**
   - Verify E(7,12) renders 12 positions on a circle with 7 filled (teal) and 5 open (white)
   - Verify E(3,8) renders correctly with 3 filled, 5 open
   - Verify E(5,12) r3 shows rotation label
   - Verify connecting lines appear between adjacent filled positions

5. **PolyScreenshot fallback**
   - Verify placeholder appears (camera icon + "Screenshot pending") for the nonexistent image
   - Verify caption text appears below the placeholder frame
