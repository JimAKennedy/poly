#!/usr/bin/env node
// check-ascii-diagrams.mjs — guide-parity M004/S02 (GP10).
//
// Architecture diagrams drawn as ASCII art are hand-maintained: they render as
// monospace blocks unlike the rest of a typography-first site, they cannot be
// re-laid-out when a box grows, and they drift from the code silently. M004
// converts them to Mermaid, which the build renders. This guard is what stops
// them coming back.
//
// Scope is read from jk-standards.yaml rather than hardcoded, so the roots and
// the exemptions cannot drift from the rest of the doc tooling. ARCHITECTURE.md
// is added explicitly: it lives at the repo root, outside every declared root,
// while being exactly the kind of file this rule exists for.
//
// Escape hatch, per the repo's escape-hatch discipline — in-band, greppable,
// reasoned:
//
//     <!-- boxdraw-ok: <reason> -->
//
// exempts the file it appears in. The reason must be non-empty: a hatch used
// without a stated reason is a finding in itself, so an empty one fails rather
// than passes. Exempted files are counted in the summary, because an invisible
// suppression count is how a rule rots.

import { readFile, readdir } from 'node:fs/promises';
import { existsSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join, relative, extname } from 'node:path';

const REPO = dirname(dirname(fileURLToPath(import.meta.url)));

// U+2500–U+257F is the Box Drawing block. The four arrowheads are the glyphs
// this repo's diagrams actually use for direction; they appear nowhere in the
// governed docs except inside those diagrams, checked before being included.
const BOX_DRAWING = /[─-╿▲▼◄►]/;
// `[\s\S]` rather than `.` so a multi-line reason matches: a one-line reason
// slot would push authors toward terse, uninformative hatches, which is the
// opposite of what the discipline asks for.
const HATCH = /<!--\s*boxdraw-ok:\s*([\s\S]*?)\s*-->/;

// Node has no built-in YAML parser and this repo carries no root package.json,
// so every scripts/*.mjs uses node: built-ins only — the convention
// check-release-workflow.mjs states outright. The two lists needed here are
// flat enough for a targeted parse, and `assertParsed` below refuses to run on
// an empty read rather than silently scanning nothing.
const configSource = await readFile(join(REPO, 'jk-standards.yaml'), 'utf8');

function parseDocRoots(src) {
  const block = src.match(/^doc_roots:\n((?:[^\S\n]+.*\n|\n)*)/m);
  if (!block) return [];
  return [...block[1].matchAll(/-\s*path:\s*(\S+)\s*\n\s*extensions:\s*\[([^\]]*)\]/g)].map((m) => ({
    path: m[1],
    extensions: m[2].split(',').map((e) => e.trim().replace(/["']/g, '')).filter(Boolean),
  }));
}

function parseExemptDirs(src) {
  const block = src.match(/^exempt_dirs:\n((?:[^\S\n]+.*\n|\n)*)/m);
  if (!block) return [];
  return [...block[1].matchAll(/^\s+-\s+(\S+)/gm)].map((m) => m[1]);
}

const docRoots = parseDocRoots(configSource);
const exemptDirs = parseExemptDirs(configSource);

// A config read that silently returns nothing would turn this guard into a
// green check over an empty set — the exact failure CLAUDE.md records for the
// jk-standards rules that passed while matching nothing.
if (docRoots.length === 0) {
  console.error('[check-ascii-diagrams] parsed no doc_roots from jk-standards.yaml — refusing to report a pass');
  process.exit(1);
}
if (exemptDirs.length === 0) {
  console.error('[check-ascii-diagrams] parsed no exempt_dirs from jk-standards.yaml — refusing to scan frozen records');
  process.exit(1);
}

async function* walk(dir) {
  for (const entry of await readdir(dir, { withFileTypes: true })) {
    const full = join(dir, entry.name);
    if (entry.name === 'node_modules' || entry.name.startsWith('.')) continue;
    if (entry.isDirectory()) yield* walk(full);
    else yield full;
  }
}

const targets = [];
for (const root of docRoots) {
  const abs = join(REPO, root.path);
  if (!existsSync(abs)) continue;
  for await (const file of walk(abs)) {
    if (!root.extensions.includes(extname(file))) continue;
    const rel = relative(REPO, file);
    if (exemptDirs.some((d) => rel.startsWith(d))) continue;
    targets.push(rel);
  }
}
// Governed but outside every declared root.
if (existsSync(join(REPO, 'ARCHITECTURE.md'))) targets.push('ARCHITECTURE.md');

const findings = [];
const exempted = [];
let emptyReason = false;

for (const rel of targets.sort()) {
  const body = await readFile(join(REPO, rel), 'utf8');
  const hatch = body.match(HATCH);
  if (hatch) {
    if (!hatch[1]) {
      console.error(`${rel}: boxdraw-ok marker carries no reason — state why this file keeps its ASCII art`);
      emptyReason = true;
    } else {
      exempted.push(`${rel} — ${hatch[1].replace(/\s+/g, ' ').trim()}`);
    }
    continue;
  }
  body.split('\n').forEach((line, i) => {
    if (BOX_DRAWING.test(line)) findings.push({ rel, line: i + 1, text: line.trim().slice(0, 60) });
  });
}

console.log(`[check-ascii-diagrams] scanned ${targets.length} governed doc(s)`);
if (exempted.length) {
  console.log(`[check-ascii-diagrams] ${exempted.length} file(s) exempted by boxdraw-ok:`);
  for (const e of exempted) console.log(`  - ${e}`);
}

if (emptyReason) process.exit(1);

if (findings.length === 0) {
  console.log('[check-ascii-diagrams] no ASCII diagrams found');
  process.exit(0);
}

const byFile = new Map();
for (const f of findings) byFile.set(f.rel, (byFile.get(f.rel) ?? 0) + 1);

console.error(`\n[check-ascii-diagrams] ${findings.length} line(s) of ASCII diagram in ${byFile.size} file(s):\n`);
for (const [rel, count] of byFile) console.error(`  ${rel} — ${count} line(s)`);
for (const f of findings.slice(0, 10)) console.error(`\n  ${f.rel}:${f.line}  ${f.text}`);
if (findings.length > 10) console.error(`\n  … and ${findings.length - 10} more`);
console.error(
  '\nEither convert the diagram to a ```mermaid fence — the site renders it at\n' +
    'build time and GitHub renders it in .md — or, if the block is genuinely not\n' +
    'a diagram, add a marker naming the reason:\n\n' +
    '    <!-- boxdraw-ok: why this file keeps its ASCII art -->\n',
);
process.exit(1);
