// front-door.test.mjs — the site's first page sends a visitor to the download
// and the in-browser engine, and no page says the guide is under construction.
//
// open-source-launch M004/S04, OS26 and OS27. Every page carried "Under
// active construction" over a guide that first-release M001–M004 had just
// verified, and the hero offered Start Reading and GitHub with no download
// and no way to hear the engine. The banner is now a pre-release notice
// linking the Releases page, and the hero's actions are the download, the
// Foundations chapter (the first playable previews) and the guide.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readdirSync, readFileSync, statSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const SITE = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const SRC = resolve(SITE, 'src');

function walk(dir, out = []) {
  for (const name of readdirSync(dir)) {
    const p = join(dir, name);
    if (statSync(p).isDirectory()) walk(p, out);
    else out.push(p);
  }
  return out;
}

test('no file under site/src carries the construction banner text', () => {
  const hits = walk(SRC).filter((p) => readFileSync(p, 'utf8').includes('Under active construction'));
  assert.deepEqual(hits.map((p) => p.slice(SITE.length + 1)), [], 'the construction banner text is still in the tree (OS26)');
});

test('the banner is a pre-release notice linking the Releases page', () => {
  const banner = readFileSync(resolve(SRC, 'components/Banner.astro'), 'utf8');
  assert.match(banner, /pre-release/i, 'Banner.astro does not say pre-release (OS26)');
  assert.ok(banner.includes('https://github.com/JimAKennedy/poly/releases'), 'Banner.astro does not link the Releases page (OS26)');
});

// The hero's actions, read from index.mdx's frontmatter: each `- text:` with
// the `link:` that follows it, in order.
function heroActions() {
  const src = readFileSync(resolve(SRC, 'content/docs/index.mdx'), 'utf8');
  const fm = src.slice(0, src.indexOf('\n---', 3));
  const block = fm.slice(fm.indexOf('actions:'));
  return [...block.matchAll(/-\s+text:\s*(.+)\n\s+link:\s*(\S+)/g)].map((m) => ({ text: m[1].trim(), link: m[2] }));
}

test('the hero offers download, try it and the guide', () => {
  assert.deepEqual(heroActions(), [
    { text: 'Download', link: 'https://github.com/JimAKennedy/poly/releases' },
    { text: 'Try it in the browser', link: '/01-foundations/' },
    { text: 'Read the guide', link: '/introduction/' },
  ]);
});
