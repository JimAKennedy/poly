import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { existsSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join, relative, extname } from 'node:path';
import { unified } from 'unified';
import rehypeParse from 'rehype-parse';
import rehypeStringify from 'rehype-stringify';
import rehypeMermaid from 'rehype-mermaid';
import { mermaidRehypeOptions } from '../src/lib/mermaid-config.mjs';

// guide-parity M004/S02, row GP11. Four of the five mermaid fences in this repo
// live in `.md` files -- ARCHITECTURE.md among them -- which Astro never builds
// and GitHub renders at view time. Nothing checked those.
//
// The site-side `.mdx` fence needs no help: `astro build` exits 1 on a bad
// fence (measured, by breaking one), and both site-e2e and the Pages deploy
// build the site, so a syntax error turns the pull request red before it can
// reach production. This test exists for the four that no build touches.
//
// HONEST LIMIT: this renders against the mermaid version pinned in
// site/package.json. GitHub renders with its own, and the two can diverge. A
// fence passing here is strong evidence, not a guarantee, and no local check
// can close that gap.

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = join(HERE, '..', '..');

// Same scoping as scripts/check-ascii-diagrams.mjs: the governed doc roots plus
// ARCHITECTURE.md, which sits outside all of them.
const ROOTS = [
  { path: 'docs', extensions: ['.md'] },
  { path: 'site/src/content/docs', extensions: ['.mdx'] },
];
const EXEMPT = ['docs/reviews/', 'docs/plans/'];

async function* walk(dir) {
  for (const entry of await readdir(dir, { withFileTypes: true })) {
    if (entry.name === 'node_modules' || entry.name.startsWith('.')) continue;
    const full = join(dir, entry.name);
    if (entry.isDirectory()) yield* walk(full);
    else yield full;
  }
}

async function collectFences() {
  const found = [];
  const files = [];
  for (const root of ROOTS) {
    const abs = join(REPO, root.path);
    if (!existsSync(abs)) continue;
    for await (const f of walk(abs)) {
      if (!root.extensions.includes(extname(f))) continue;
      const rel = relative(REPO, f);
      if (EXEMPT.some((d) => rel.startsWith(d))) continue;
      files.push(rel);
    }
  }
  if (existsSync(join(REPO, 'ARCHITECTURE.md'))) files.push('ARCHITECTURE.md');

  for (const rel of files.sort()) {
    const src = await readFile(join(REPO, rel), 'utf8');
    for (const [i, m] of [...src.matchAll(/```mermaid\n([\s\S]*?)```/g)].entries()) {
      found.push({ rel, index: i + 1, code: m[1] });
    }
  }
  return found;
}

async function render(code) {
  const escaped = code.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  const html = `<pre><code class="language-mermaid">${escaped}</code></pre>`;
  return String(
    await unified()
      .use(rehypeParse, { fragment: true })
      .use(rehypeMermaid, mermaidRehypeOptions)
      .use(rehypeStringify)
      .process(html),
  );
}

test('every mermaid fence in a governed doc renders', async () => {
  const fences = await collectFences();

  // Without this, a moved directory or a renamed extension turns the whole
  // test into a pass over an empty set -- green, and checking nothing.
  assert.ok(
    fences.length > 0,
    'found no mermaid fences in any governed doc; this test has stopped checking anything',
  );

  const failures = [];
  for (const fence of fences) {
    try {
      const out = await render(fence.code);
      if (!/<svg/.test(out)) failures.push(`${fence.rel} fence ${fence.index}: produced no <svg>`);
    } catch (e) {
      failures.push(`${fence.rel} fence ${fence.index}: ${e.message.split('\n')[0]}`);
    }
  }

  assert.deepEqual(failures, [], `mermaid fences failed to render:\n  ${failures.join('\n  ')}`);
});
