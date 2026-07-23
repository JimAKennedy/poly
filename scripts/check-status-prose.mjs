#!/usr/bin/env node
// M048 S10 — ban stale status-tracking prose in gated docs.
//
// Progress claims ("not yet implemented", "Phase B", "6 TODO markers remain",
// "Status: proposed" without a date) drift the moment reality moves. This
// linter enforces two rules across gated docs (front-matter class:gated):
//
//   1. `Status:` lines must carry a `(YYYY-MM-DD)` date anchor so readers
//      know when the claim was made. Undated status = permanent lie in
//      waiting.
//   2. Forbidden progress-tracking phrases are rejected outright — they
//      belong in CHANGELOG.md, GitHub issues, or the generated dashboards
//      that track state, not in prose docs that describe contracts and
//      rationale.
//
// Exemptions:
//   - class:archived docs are never checked (they're honest historical
//     snapshots — the archive banner is what signals staleness)
//   - class:generated docs are never checked (their content is machine-
//     authored)
//   - docs/reviews/** dated reviews are never checked (they're immutable
//     dated evidence — progress prose inside them IS the historical fact)

import { readdirSync, readFileSync, statSync } from 'node:fs';
import { dirname, join, relative, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, '..');

const TARGET_ROOTS = [
  join(REPO_ROOT, 'docs'),
  join(REPO_ROOT, 'site/src/content/docs'),
];

// Directory prefixes (repo-relative) that are exempt regardless of front-matter.
const EXEMPT_DIRS = ['docs/reviews/'];

// Forbidden progress-tracking phrases. Each entry: {pattern, hint}.
// Patterns are case-insensitive; hint tells the reader where the info belongs.
const FORBIDDEN = [
  { pattern: /\bnot\s+yet\s+(implemented|applied|evaluated|resolved|wired)\b/i,
    hint: 'progress claim — describe current contract or delete; state belongs in CHANGELOG/issues' },
  { pattern: /\bnot\s+implemented\s+yet\b/i,
    hint: 'progress claim — describe current contract or delete; state belongs in CHANGELOG/issues' },
  { pattern: /\bTODO\s+markers?\s+remain\b/i,
    hint: 'progress claim — counts drift the moment code moves; belongs in generated dashboard' },
  { pattern: /\bTODO\(spike\)\s+markers?\s+remain\b/i,
    hint: 'progress claim — counts drift the moment code moves; belongs in generated dashboard' },
  { pattern: /^\s*Status:\s+Phase\s+[A-Z]\b/im,
    hint: 'phase-tracking prose — belongs in ROADMAP/issues, not in gated docs' },
  { pattern: /\bDesigned\s+For,\s+Not\s+Implemented\b/i,
    hint: 'forbidden section heading — describe current state or remove' },
];

// Status: lines must carry a (YYYY-MM-DD) date anchor.
const STATUS_LINE_RE = /^Status:\s+/i;
const DATE_ANCHOR_RE = /\(20\d{2}-\d{2}-\d{2}\)/;

function readFrontmatterClass(text) {
  if (!text.startsWith('---')) return null;
  const end = text.indexOf('\n---', 3);
  if (end === -1) return null;
  const fm = text.slice(3, end);
  const m = fm.match(/^\s*class:\s*(\S+)/m);
  return m ? m[1].trim() : null;
}

function isExempt(relpath) {
  return EXEMPT_DIRS.some((d) => relpath.startsWith(d));
}

function listDocs() {
  const out = [];
  for (const root of TARGET_ROOTS) {
    if (!statSync(root, { throwIfNoEntry: false })) continue;
    walk(root, out);
  }
  return out;
}

function walk(dir, out) {
  for (const name of readdirSync(dir)) {
    const p = join(dir, name);
    const st = statSync(p);
    if (st.isDirectory()) walk(p, out);
    else if (name.endsWith('.md') || name.endsWith('.mdx')) out.push(p);
  }
}

let errors = 0;
for (const abs of listDocs()) {
  const rel = relative(REPO_ROOT, abs);
  if (isExempt(rel)) continue;
  const text = readFileSync(abs, 'utf8');
  const klass = readFrontmatterClass(text);
  if (klass !== 'gated') continue;

  const lines = text.split('\n');
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const lineno = i + 1;

    // Rule 1: Status: lines need a date anchor.
    if (STATUS_LINE_RE.test(line) && !DATE_ANCHOR_RE.test(line)) {
      console.log(
        `::error file=${rel},line=${lineno}::Status: prose needs (YYYY-MM-DD) date anchor — undated status will drift`,
      );
      errors++;
    }

    // Rule 2: forbidden progress-tracking phrases.
    for (const { pattern, hint } of FORBIDDEN) {
      if (pattern.test(line)) {
        console.log(`::error file=${rel},line=${lineno}::${hint}  [${line.trim().slice(0, 120)}]`);
        errors++;
        break;
      }
    }
  }
}

if (errors > 0) {
  console.error(`\nFound ${errors} status-prose violation(s) in gated docs.`);
  console.error(`See scripts/check-status-prose.mjs header for rules and rationale.`);
  process.exit(1);
}

console.log('No status-prose violations in gated docs.');
