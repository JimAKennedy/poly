---
estimated_steps: 1
estimated_files: 5
skills_used: []
---

# T01: Add region tags to C++ engine source files

Add // region:name and // endregion:name comment pairs to 4 engine source files marking the 5 code regions referenced by CodeSnippet components. Fix 08-minimalism.mdx reference from nonexistent lane_state.cpp to engine.cpp.

## Inputs

- `site/src/content/docs/01-foundations.mdx`
- `site/src/content/docs/05-gamelan.mdx`
- `site/src/content/docs/08-minimalism.mdx`
- `site/src/content/docs/14-synthesis.mdx`
- `site/src/content/docs/15-compositional-grammar.mdx`

## Expected Output

- `Region-tagged C++ source files`

## Verification

grep -r 'region:' engine/src/ shows all 5 region pairs
