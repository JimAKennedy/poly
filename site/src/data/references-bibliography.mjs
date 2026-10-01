// The two bibliographies, read as data.
//
// verifiable-references M003/S01. The archive is named from the bibliography
// and fetched from it, so both need each entry's text and link. The shipping
// appendix and the deferred theory bundle (first-release M002/S02) carry
// overlapping anchors; where both carry one, the appendix's text and link win,
// because that is the version a reader of the shipping guide sees — and for
// ref-1 the only one whose link is the work the entry names.

import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const CONTENT = join(HERE, '..', 'content');

export const BIBLIOGRAPHY_PATHS = [
  join(CONTENT, 'docs', 'appendix-references.mdx'),
  join(CONTENT, 'theory', 'theory-references.mdx'),
];

const ANCHOR = /<span id="((?:ref|fr)-[A-Za-z0-9-]+)"[^>]*>/;
const LINK = /\[([^\]]*)\]\(([^)\s]+)\)/g;

function parseLine(line) {
  const m = ANCHOR.exec(line);
  if (!m) return null;
  const urls = [...line.matchAll(LINK)].map((l) => l[2]);
  const text = line
    .replace(/^\s*-\s*/, '')
    .replace(/<\/?span[^>]*>/g, '')
    .replace(/\*\*\[\d+\]\*\*/g, '')
    .replace(LINK, '')
    .replace(/\s*·\s*/g, ' ')
    .replace(/\s+/g, ' ')
    .trim();
  return [m[1], { text, url: urls[0] ?? null }];
}

export async function readBibliography() {
  const out = new Map();
  for (const path of BIBLIOGRAPHY_PATHS) {
    const mdx = await readFile(path, 'utf8');
    let found = 0;
    for (const line of mdx.split('\n')) {
      const parsed = parseLine(line);
      if (!parsed) continue;
      found += 1;
      if (!out.has(parsed[0])) out.set(parsed[0], parsed[1]);
    }
    if (found === 0) {
      throw new Error(`no ref-/fr- entries parsed from ${path}; has the anchor convention changed?`);
    }
  }
  return out;
}
