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
