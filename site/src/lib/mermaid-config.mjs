// guide-parity M004/S01 (GP09). The single source of the Mermaid rendering
// options, imported by both `astro.config.mjs` and the tests, so the tests
// exercise the options the build actually uses rather than a copy that drifts.
//
// `strategy: 'inline-svg'` emits the SVG into the page HTML, which is what lets
// site CSS reach it and the diagram inherit the guide's typography. Nothing
// mermaid-related ships to the browser: rendering happens during `astro build`.
//
// There is deliberately no `deterministicIds` here. The obvious version of this
// file sets it and names it the reason two builds agree. It is inert:
// rehype-mermaid renders each diagram in a fresh page context, so the id
// counter starts at zero either way -- measured by rendering with and without
// it and getting identical bytes and an identical `id="mermaid-0"`. Determinism
// is a property of the library's per-diagram isolation, and
// `tests/mermaid-render.test.mjs` asserts that property directly rather than
// trusting a flag that does nothing.

/** @type {import('rehype-mermaid').Options} */
export const mermaidRehypeOptions = {
  strategy: 'inline-svg',
};
