# M012: Poly Guide Publication Website

**Vision:** A Crafting Interpreters-style static site for the Poly rhythmic guide, integrating ethnomusicological prose, live-extracted C++ engine code, and Poly patch reference panels in a single publication-quality web document. Built with Astro + Starlight, deployed to GitHub Pages, with code snippets pulled from the repo at build time so documentation never goes stale.

## Success Criteria

- All 17 chapters from poly_rhythmic_guide.md render as MDX pages with PolyPatch, ListenFor, and EuclideanDiagram components
- Live code extraction pulls C++ snippets from engine source via region tags at build time
- Design system implements the three-zone visual language: serif prose, sans-serif patch boxes, dark code panels
- Site deploys automatically to GitHub Pages on push to main
- EuclideanDiagram SVG component renders circle diagrams from steps/hits/rotation props
- PolyScreenshot component handles missing images gracefully with placeholder fallback

## Slices

- [x] **S01: S01** `risk:low` `depends:[]`
  > After this: astro dev serves a working site at localhost:4321 with one sample chapter, correct fonts, colour palette, and two-column layout

- [x] **S02: S02** `risk:medium` `depends:[]`
  > After this: A test page renders all 5 components: PolyPatch with lane table, ListenFor amber callout, CodeSnippet with static placeholder, EuclideanDiagram SVG circle, and PolyScreenshot with missing-image placeholder

- [x] **S03: S03** `risk:medium` `depends:[]`
  > After this: All 17 sections plus 2 appendices render as navigable chapters with PolyPatch callouts replacing parameter tables, ListenFor boxes for listening guidance, and EuclideanDiagram calls for every E(k,n) reference

- [x] **S04: S04** `risk:high` `depends:[]`
  > After this: CodeSnippet components in at least 3 chapters render syntax-highlighted C++ pulled from engine source files via region tags, with filename headers and GitHub line links

- [x] **S05: S05** `risk:low` `depends:[]`
  > After this: Push to main triggers GitHub Actions build and deploys the site to GitHub Pages

- [x] **S06: S06** `risk:low` `depends:[]`
  > After this: At least 5 representative chapters have screenshots showing the Poly UI with patch values loaded

## Boundary Map

Not provided.
