#!/usr/bin/env node
// fetch-references.mjs — archive every open-access reference a script can
// fetch, and record the ones it cannot (verifiable-references M003/S01, VR09).
//
// Usage (the archive path is set for the one command, never in a file):
//   POLY_REFERENCES_ARCHIVE=<dir> node scripts/fetch-references.mjs [--dry-run] [--retry]
//   POLY_REFERENCES_ARCHIVE=<dir> node scripts/fetch-references.mjs --verify
//
// The destination comes only from POLY_REFERENCES_ARCHIVE. The read-side
// helper falls back to a gitignored .references/ so tests run anywhere; a
// writer that did the same would put copyrighted PDFs wherever the repo was
// checked out, so this one refuses instead.
//
// A candidate is an open-access manifest record whose retrieval has not been
// attempted and that has a route — the entry's first link, or its DOI. A PDF
// is saved as served; an HTML page is printed to PDF with headless Chrome. A
// host that refuses (any status but 200, a bot challenge, a render with no
// document) marks the record `script-refused`, which later runs skip unless
// --retry is given, and every refusal is listed at the end of the run. A file
// already at the target name is never overwritten.
//
// --verify fetches nothing: it checks that every archived record's file
// exists in the archive under the name the convention gives it.

import { readFile, writeFile, stat, rename, rm, mkdtemp } from 'node:fs/promises';
import { existsSync, statSync, readFileSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { tmpdir } from 'node:os';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { dirname, join } from 'node:path';

import {
  archiveFileName,
  resolveArchiveFile,
} from '../site/src/data/references-archive.mjs';
import { readBibliography } from '../site/src/data/references-bibliography.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const MANIFEST_PATH = join(HERE, '..', 'site', 'src', 'data', 'references.json');
const CHROME = '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome';
const USER_AGENT =
  'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 ' +
  '(KHTML, like Gecko) Chrome/129.0 Safari/537.36';
const FETCH_TIMEOUT_MS = 30_000;
const RENDER_TIMEOUT_MS = 90_000;
const MIN_RENDER_BYTES = 10 * 1024;

// A challenge is recognised by what only a challenge carries: Anubis's
// script, Cloudflare's challenge token, or a title saying the page is checking
// for a bot. The bare word "captcha" is not enough — ordinary articles load
// reCAPTCHA for their comment forms, and MediaWiki names it in its config — so
// it counts only on a page too small to be an article. The first real pass
// refused six articles on that word alone.
const CHALLENGE_MARKERS = [/anubis/i, /cf[-_]chl/i];
const CHALLENGE_TITLE = /just a moment|not a bot|attention required|checking your browser/i;
const SMALL_PAGE_CHARS = 20_000;

export function isChallengePage(html) {
  if (CHALLENGE_MARKERS.some((re) => re.test(html))) return true;
  const title = /<title[^>]*>([^<]*)/i.exec(html)?.[1] ?? '';
  if (CHALLENGE_TITLE.test(title)) return true;
  return html.length < SMALL_PAGE_CHARS && /captcha/i.test(html);
}

export function selectCandidates(manifest, bibliography, { retry }) {
  const out = [];
  for (const [anchor, rec] of Object.entries(manifest.entries)) {
    if (rec.obtainability !== 'open-access') continue;
    const status = rec.retrieval?.status;
    if (!(status === 'not-attempted' || (retry && status === 'script-refused'))) continue;
    const entry = bibliography.get(anchor);
    const doi = rec.identifier?.doi;
    const url = entry?.url ?? (doi ? `https://doi.org/${doi}` : null);
    if (!url) continue;
    out.push({ anchor, url, text: entry?.text ?? '' });
  }
  return out;
}

function today() {
  return new Date().toISOString().slice(0, 10);
}

function fail(message) {
  process.stderr.write(`fetch-references: ${message}\n`);
  process.exit(2);
}

function requireArchive() {
  const dir = process.env.POLY_REFERENCES_ARCHIVE;
  if (typeof dir !== 'string' || dir.trim() === '') {
    fail(
      'POLY_REFERENCES_ARCHIVE is not set. This script writes copyrighted PDFs ' +
        'and has no default destination: it will not fall back to .references/ ' +
        'or anywhere else. Set it to the archive directory for this command.',
    );
  }
  let ok = false;
  try {
    ok = statSync(dir).isDirectory();
  } catch {
    ok = false;
  }
  if (!ok) fail(`POLY_REFERENCES_ARCHIVE names ${dir}, which is not an existing directory.`);
  return dir;
}

function isPdf(buf) {
  return buf.length >= 4 && buf.subarray(0, 4).toString('latin1') === '%PDF';
}

async function renderToPdf(url) {
  const work = await mkdtemp(join(tmpdir(), 'poly-refs-render-'));
  const out = join(work, 'page.pdf');
  try {
    spawnSync(
      CHROME,
      [
        '--headless=new',
        '--disable-gpu',
        '--no-first-run',
        `--user-data-dir=${join(work, 'profile')}`,
        '--no-pdf-header-footer',
        `--print-to-pdf=${out}`,
        url,
      ],
      { timeout: RENDER_TIMEOUT_MS, stdio: 'ignore' },
    );
    if (!existsSync(out)) return null;
    const buf = readFileSync(out);
    if (!isPdf(buf) || buf.length < MIN_RENDER_BYTES) return null;
    return buf;
  } finally {
    await rm(work, { recursive: true, force: true });
  }
}

async function retrieve(candidate) {
  const host = new URL(candidate.url).host;
  let res;
  try {
    res = await fetch(candidate.url, {
      headers: { 'User-Agent': USER_AGENT, Accept: 'text/html,application/pdf;q=0.9,*/*;q=0.8' },
      redirect: 'follow',
      signal: AbortSignal.timeout(FETCH_TIMEOUT_MS),
    });
  } catch (err) {
    return { refused: `no response from ${host} (${err.name})` };
  }
  const finalHost = new URL(res.url || candidate.url).host;
  if (res.status !== 200) return { refused: `HTTP ${res.status} from ${finalHost}` };
  const type = (res.headers.get('content-type') ?? '').split(';')[0].trim();
  const buf = Buffer.from(await res.arrayBuffer());
  if (isPdf(buf)) return { bytes: buf, how: 'pdf', host: finalHost };
  if (type === 'text/html' || type === 'application/xhtml+xml') {
    if (isChallengePage(buf.toString('utf8'))) return { refused: `bot challenge at ${finalHost}` };
    const rendered = await renderToPdf(res.url || candidate.url);
    if (!rendered) return { refused: `render produced no document from ${finalHost}` };
    return { bytes: rendered, how: 'rendered html', host: finalHost };
  }
  return { refused: `content-type ${type || 'none'} from ${finalHost}` };
}

async function verify(manifest, bibliography) {
  const problems = [];
  let archived = 0;
  for (const [anchor, rec] of Object.entries(manifest.entries)) {
    if (rec.retrieval?.status !== 'archived') continue;
    archived += 1;
    const expected = archiveFileName(anchor, bibliography.get(anchor)?.text ?? '');
    if (rec.archiveFile !== expected) {
      problems.push(`${anchor}: archiveFile ${JSON.stringify(rec.archiveFile)} is not the convention's ${JSON.stringify(expected)}`);
    }
    if (!existsSync(resolveArchiveFile(rec.archiveFile))) {
      problems.push(`${anchor}: ${rec.archiveFile} is not in the archive`);
    }
  }
  for (const p of problems) process.stdout.write(`FAIL ${p}\n`);
  process.stdout.write(`verify: ${archived} archived records, ${problems.length} problem(s)\n`);
  return problems.length === 0 ? 0 : 1;
}

async function main(argv) {
  const flags = new Set(argv);
  const known = new Set(['--dry-run', '--retry', '--verify']);
  for (const f of flags) if (!known.has(f)) fail(`unknown argument ${f}`);

  requireArchive();
  const raw = await readFile(MANIFEST_PATH, 'utf8');
  const manifest = JSON.parse(raw);
  const bibliography = await readBibliography();

  if (flags.has('--verify')) return verify(manifest, bibliography);

  const candidates = selectCandidates(manifest, bibliography, { retry: flags.has('--retry') });
  process.stdout.write(`${candidates.length} candidate(s)\n`);
  for (const c of candidates) process.stdout.write(`candidate ${c.anchor} ${c.url}\n`);
  if (flags.has('--dry-run')) return 0;

  const refused = [];
  let archived = 0;
  for (const c of candidates) {
    const rec = manifest.entries[c.anchor];
    const name = archiveFileName(c.anchor, c.text);
    const target = resolveArchiveFile(name);
    if (existsSync(target)) {
      process.stdout.write(`exists, not adopted: ${name} (${c.anchor})\n`);
      continue;
    }
    const got = await retrieve(c);
    if (got.refused) {
      rec.retrieval = { status: 'script-refused', checked: today(), detail: got.refused };
      refused.push(`${c.anchor}: ${got.refused}`);
      process.stdout.write(`refused ${c.anchor}: ${got.refused}\n`);
      continue;
    }
    const partial = `${target}.partial`;
    await writeFile(partial, got.bytes, { flag: 'wx' });
    if (existsSync(target)) {
      await rm(partial);
      process.stdout.write(`exists, not adopted: ${name} (${c.anchor})\n`);
      continue;
    }
    await rename(partial, target);
    const size = (await stat(target)).size;
    rec.archiveFile = name;
    rec.retrieval = {
      status: 'archived',
      checked: today(),
      detail: `fetched from ${got.host} (${got.how}), ${size} bytes`,
    };
    archived += 1;
    process.stdout.write(`archived ${c.anchor} -> ${name}\n`);
  }

  await writeFile(MANIFEST_PATH, JSON.stringify(manifest, null, 2) + (raw.endsWith('\n') ? '\n' : ''));
  process.stdout.write(`\n${archived} archived, ${refused.length} refused\n`);
  if (refused.length > 0) {
    process.stdout.write('Refused (recorded as script-refused; skipped next run unless --retry):\n');
    for (const r of refused) process.stdout.write(`  ${r}\n`);
  }
  return 0;
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  main(process.argv.slice(2)).then((code) => process.exit(code));
}
