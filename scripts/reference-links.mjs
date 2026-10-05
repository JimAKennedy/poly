#!/usr/bin/env node
// reference-links.mjs — check every URL the bibliographies print, and report
// which are dead (verifiable-references M006/S01, VR16).
//
// Before this, nothing fetched anything: research-provenance checks that a
// citation resolves to an anchor and citation-tier checks the declared tier,
// so a Tier-A source could rot to a 404 with every gate green.
//
// Classes (M006-decisions.md):
//   ok       2xx after redirects
//   blocked  401, 403, 406, 429, or a 200 that is a bot challenge — reachable
//            by a person, refused to a script; never reported as dead
//   dead     404, 410, DNS failure, refused or reset connection, or no
//            response within the timeout (retried once first)
//   error    anything else, 5xx included: check by hand, usually transient
//
// Advisory, never a gate: the checker exits 0 whatever it finds, and 2 only
// when it cannot run. A link checker that fails builds gets disabled.

import { readFile, writeFile } from 'node:fs/promises';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { dirname, join } from 'node:path';

import { isChallengePage } from './fetch-references.mjs';

const USER_AGENT = 'poly-reference-links (+https://github.com/JimAKennedy/poly)';
const BODY_SNIFF_BYTES = 64 * 1024;
const BLOCKED = new Set([401, 403, 406, 429]);
const DEAD = new Set([404, 410]);
const DEAD_CODES = new Set(['ENOTFOUND', 'ECONNREFUSED', 'ECONNRESET', 'EAI_AGAIN']);

export function classifyResponse({ status, body }) {
  if (status >= 200 && status < 300) return isChallengePage(body ?? '') ? 'blocked' : 'ok';
  if (BLOCKED.has(status)) return 'blocked';
  if (DEAD.has(status)) return 'dead';
  return 'error';
}

function errorCode(err) {
  return err?.cause?.code ?? err?.code ?? null;
}

// undici reports its own connect/headers/body timeouts as a TypeError whose
// cause carries the code, not as an abort; a host that never answers is a
// no-response either way.
const NO_RESPONSE_CODES = new Set(['UND_ERR_CONNECT_TIMEOUT', 'UND_ERR_HEADERS_TIMEOUT', 'UND_ERR_BODY_TIMEOUT']);

function isNoResponse(err) {
  return err?.name === 'TimeoutError' || err?.name === 'AbortError' || NO_RESPONSE_CODES.has(errorCode(err));
}

// What the report prints for a failed fetch: the cause's code, else its
// message ("redirect count exceeded"), else the error's own name.
export function describeError(err) {
  return errorCode(err) ?? err?.cause?.message ?? err?.name ?? 'unknown error';
}

export function classifyError(err) {
  if (isNoResponse(err)) return 'dead';
  return DEAD_CODES.has(errorCode(err)) ? 'dead' : 'error';
}

async function sniff(res) {
  if (!res.body) return '';
  const reader = res.body.getReader();
  const chunks = [];
  let size = 0;
  try {
    while (size < BODY_SNIFF_BYTES) {
      const { done, value } = await reader.read();
      if (done) break;
      chunks.push(value);
      size += value.length;
    }
  } finally {
    reader.cancel().catch(() => {});
  }
  return Buffer.concat(chunks).toString('utf8');
}

async function attempt(url, timeoutMs, fetchImpl) {
  const res = await fetchImpl(url, {
    headers: { 'User-Agent': USER_AGENT, Accept: 'text/html,application/pdf;q=0.9,*/*;q=0.8' },
    redirect: 'follow',
    signal: AbortSignal.timeout(timeoutMs),
  });
  const body = res.status >= 200 && res.status < 300 ? await sniff(res) : (res.body?.cancel().catch(() => {}), '');
  return { status: res.status, finalUrl: res.url || url, body };
}

export async function checkUrl(url, { timeoutMs = 20_000, fetchImpl = fetch } = {}) {
  let lastErr;
  for (let attempts = 1; attempts <= 2; attempts += 1) {
    try {
      const { status, finalUrl, body } = await attempt(url, timeoutMs, fetchImpl);
      return { url, class: classifyResponse({ status, body }), status, finalUrl, attempts };
    } catch (err) {
      lastErr = err;
      // Only a no-response earns a retry; a DNS failure or a refusal is an
      // answer, and asking twice would not change it.
      if (!isNoResponse(err)) {
        return { url, class: classifyError(err), error: describeError(err), attempts };
      }
    }
  }
  return { url, class: classifyError(lastErr), error: `no response (${describeError(lastErr)})`, attempts: 2 };
}

// ---- Collecting the URLs ---------------------------------------------------

const HERE = dirname(fileURLToPath(import.meta.url));
const ENTRY = /id="((?:ref|fr)-[a-z0-9-]+)"/;
// One level of balanced parentheses inside a URL (Wikipedia-style), matching
// site/src/data/references-bibliography.mjs.
const LINK = /\]\((https?:\/\/[^\s()]*(?:\([^\s()]*\)[^\s()]*)*)\)/g;

function bibliographyPaths() {
  const root = process.env.POLY_BIBLIOGRAPHY_ROOT || join(HERE, '..', 'site', 'src', 'content');
  return [join(root, 'docs', 'appendix-references.mdx'), join(root, 'theory', 'theory-references.mdx')];
}

export async function readBibliographyTexts() {
  return Promise.all(
    bibliographyPaths().map(async (p) => ({ name: p.split('/').pop(), text: await readFile(p, 'utf8') })),
  );
}

// Every http(s) link on every entry line, deduplicated across files, each
// with the anchors that print it. Prose links are not entries and are skipped.
export function collectUrls(texts) {
  const urls = new Map();
  for (const { text } of texts) {
    for (const line of text.split('\n')) {
      const anchor = ENTRY.exec(line)?.[1];
      if (!anchor) continue;
      for (const m of line.matchAll(LINK)) {
        if (!urls.has(m[1])) urls.set(m[1], new Set());
        urls.get(m[1]).add(anchor);
      }
    }
  }
  return urls;
}

// ---- The command line ------------------------------------------------------

const CONCURRENCY = 4;
const ORDER = ['dead', 'error', 'blocked'];
const HEADINGS = {
  dead: 'Dead — 404, 410, or no response',
  error: 'Check by hand — a server error or an unexpected status',
  blocked: 'Blocked — refused to a script, reachable by a person (not dead)',
};

function parseArgs(argv) {
  const args = { dryRun: false, extra: [], json: null, summary: null };
  for (let i = 0; i < argv.length; i += 1) {
    const a = argv[i];
    if (a === '--dry-run') args.dryRun = true;
    else if (a === '--extra-url') args.extra.push(argv[++i]);
    else if (a === '--json') args.json = argv[++i];
    else if (a === '--summary') args.summary = argv[++i];
    else throw new Error(`unknown argument ${a}`);
  }
  return args;
}

function summaryMarkdown(results) {
  const counts = Object.fromEntries(['ok', ...ORDER].map((c) => [c, results.filter((r) => r.class === c).length]));
  const lines = [
    '## Reference links',
    '',
    `${results.length} URLs checked: ${counts.ok} ok, ${counts.dead} dead, ${counts.error} to check by hand, ${counts.blocked} blocked.`,
  ];
  for (const c of ORDER) {
    const rows = results.filter((r) => r.class === c);
    if (rows.length === 0) continue;
    lines.push('', `### ${HEADINGS[c]}`, '', '| URL | Result | Cited as |', '|---|---|---|');
    for (const r of rows) lines.push(`| ${r.url} | ${r.status ?? r.error} | ${r.anchors.join(', ')} |`);
  }
  return { counts, markdown: lines.join('\n') + '\n' };
}

async function main(argv) {
  let args;
  try {
    args = parseArgs(argv);
  } catch (err) {
    process.stderr.write(`reference-links: ${err.message}\n`);
    return 2;
  }
  let urls;
  try {
    urls = collectUrls(await readBibliographyTexts());
  } catch (err) {
    process.stderr.write(`reference-links: cannot read a bibliography: ${err.message}\n`);
    return 2;
  }
  for (const u of args.extra) urls.set(u, new Set(['(injected)']));
  process.stdout.write(`${urls.size} URLs to check\n`);
  if (args.dryRun) return 0;

  const queue = [...urls.entries()];
  const results = [];
  await Promise.all(
    Array.from({ length: CONCURRENCY }, async () => {
      while (queue.length > 0) {
        const [url, anchors] = queue.shift();
        const r = await checkUrl(url);
        results.push({ ...r, anchors: [...anchors] });
      }
    }),
  );
  results.sort((a, b) => a.url.localeCompare(b.url));
  const { counts, markdown } = summaryMarkdown(results);
  process.stdout.write(markdown);
  if (args.json) await writeFile(args.json, JSON.stringify({ checked: results.length, counts, findings: results }, null, 2) + '\n');
  if (args.summary) await writeFile(args.summary, markdown);
  return 0;
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  main(process.argv.slice(2)).then((code) => process.exit(code));
}
