import { test } from 'node:test';
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

// guide-parity M004/S01 (GP09). The guide's architecture diagrams become
// Mermaid source rendered to vector output during `astro build` -- not in the
// reader's browser. These tests pin the three properties that makes a claim
// worth having: the fence is consumed at build time, the output is byte-stable
// across builds, and no mermaid runtime ships to the reader.
//
// The determinism test spawns two child processes rather than rendering twice
// in this one. "Two builds" means two processes, and a same-process comparison
// is structurally unable to see state that persists across a process boundary
// -- which is exactly the state that would break a real second build.
//
// It does NOT compare against a committed fixture. The emitted SVG carries
// browser-computed geometry derived from font metrics (viewBox numbers change
// with the available fonts), so byte-identity holds across builds on one
// machine and need not hold across machines. A fixture would assert something
// false; rendering twice asserts the property the definition of done names.

const HERE = dirname(fileURLToPath(import.meta.url));
const RENDER_ONCE = join(HERE, 'helpers', 'render-mermaid-once.mjs');

const SOURCE = `graph LR
  A[ProcessContext] --> B[TransportContext]
  B --> C[Engine]
`;

/** Render SOURCE in a fresh process, returning the rendered HTML. */
function renderInChildProcess() {
  return execFileSync(process.execPath, [RENDER_ONCE], {
    input: SOURCE,
    encoding: 'utf8',
    maxBuffer: 32 * 1024 * 1024,
  });
}

test('a mermaid block renders to inline SVG', async () => {
  const out = renderInChildProcess();
  assert.match(out, /<svg/, 'expected an inline <svg> in the rendered output');
  assert.doesNotMatch(
    out,
    /language-mermaid/,
    'the mermaid fence was passed through rather than consumed at build time',
  );
});

test('two renders of one source produce identical bytes', async () => {
  const first = renderInChildProcess();
  const second = renderInChildProcess();
  assert.equal(
    second.length,
    first.length,
    `rendered length differed between builds: ${first.length} vs ${second.length}`,
  );
  assert.equal(second, first, 'two builds over unchanged source produced different bytes');
});

test('no mermaid runtime reaches the browser', async () => {
  const out = renderInChildProcess();
  assert.doesNotMatch(
    out,
    /<script/,
    'a <script> tag in the output means rendering was deferred to page load',
  );
});

// The plugin above only works because `@astrojs/markdown-remark` is installed:
// Astro 7 defaults to the Sätteri processor, and `markdown.rehypePlugins` runs
// on the unified processor, which that package provides. Reinstalling it swaps
// the processor for every page, not only the one carrying a diagram.
//
// That swap was measured rather than assumed. Of 49 built pages, 43 differed in
// bytes — entity style (`&amp;` vs `&#x26;`), `<path/>` vs `<path></path>`, and
// inter-tag whitespace, all equivalent — but only 4 differed in visible text,
// and 3 of those were `--` widening from an en dash to an em dash. `oldschool`
// is the SmartyPants convention where `--` stays an en dash, which is the
// correct glyph in a range like `0.0--1.0`.
//
// This test exists because that setting is one word in a config file and its
// absence is silent: the guide would simply re-widen every dash, and nothing
// else would notice.
test("the markdown processor keeps `--` an en dash", async () => {
  const config = await readFile(join(HERE, '..', 'astro.config.mjs'), 'utf8');
  assert.match(
    config,
    /smartypants:\s*\{[^}]*dashes:\s*'oldschool'/,
    "markdown.smartypants.dashes must stay 'oldschool' — the default widens every `--` in the guide to an em dash, including numeric ranges",
  );
});

// M004/S01 task 2. The milestone's demo line requires diagrams that match the
// site's typography. Mermaid's default is `arial,sans-serif`, which is none of
// the guide's three faces.
//
// The stack is declared once in `src/lib/mermaid-config.mjs` and checked here
// against `--poly-font-sans` in `custom.css`, so the two cannot drift apart
// silently. Reading the CSS at build time instead would make the Astro config
// depend on parsing a stylesheet, which is a worse trade than one asserted
// literal.
test("rendered diagrams use the site's sans face", async () => {
  const out = renderInChildProcess();
  assert.match(out, /Inter/, 'the rendered SVG does not name the site sans stack');
  assert.doesNotMatch(out, /arial,\s*sans-serif/i, "mermaid's default Arial survived into the output");
});

test("the mermaid font stack matches --poly-font-sans", async () => {
  const css = await readFile(join(HERE, '..', 'src', 'styles', 'custom.css'), 'utf8');
  const declared = css.match(/--poly-font-sans:\s*([^;]+);/);
  assert.ok(declared, 'could not find --poly-font-sans in custom.css');
  const { mermaidRehypeOptions } = await import('../src/lib/mermaid-config.mjs');
  const normalise = (s) => s.replace(/\s+/g, ' ').trim();
  assert.equal(
    normalise(mermaidRehypeOptions.mermaidConfig.fontFamily),
    normalise(declared[1]),
    'the mermaid font stack has drifted from the stylesheet',
  );
});

// M004/S01 task 4. The pipeline is only proved once a real page uses it. This
// slice converts one diagram -- the appendix's System Overview -- and leaves
// the rest of the file to M004/S02, so the pipeline is demonstrated without
// this slice absorbing that slice's content work.
test('the plugin-architecture appendix has a mermaid System Overview and no ascii art in it', async () => {
  const page = await readFile(
    join(HERE, '..', 'src', 'content', 'docs', 'appendix-plugin-architecture.mdx'),
    'utf8',
  );
  assert.match(page, /```mermaid/, 'the appendix carries no mermaid source');

  const start = page.indexOf('## System Overview');
  assert.ok(start >= 0, 'the System Overview heading has moved or been renamed');
  const end = page.indexOf('\n## ', start + 1);
  const section = page.slice(start, end === -1 ? undefined : end);

  assert.match(section, /```mermaid/, 'the System Overview is not a mermaid diagram');
  const boxDrawing = section.match(/[─-╿▲▼◄►]/g) ?? [];
  assert.deepEqual(
    boxDrawing,
    [],
    `the System Overview still contains ${boxDrawing.length} box-drawing character(s)`,
  );
});
