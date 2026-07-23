#!/usr/bin/env node
/**
 * check-file-line-refs.mjs (M048 S11)
 *
 * Rule: enduring docs and source comments cite symbols/regions, not `foo.cpp:429`.
 * Line numbers rot on every edit — the reference points at the wrong code by
 * the next commit.
 *
 * What's scanned:
 *   1. Gated MDX docs under site/src/content/docs/**\/*.mdx
 *   2. Top-level docs under docs/**\/*.md
 *   3. Source comments under engine/{src,include}/**\/*.{cpp,h,hpp}
 *   4. Source comments under plugin/source/**\/*.{cpp,h,hpp}
 *
 * What's flagged:
 *   `word.EXT:LINE` where EXT ∈ {cpp,h,hpp,mjs,ts,astro,mdx,md,sh,js} and
 *   LINE is one or more digits (optionally followed by `-DIGITS` or `,DIGITS`).
 *
 * What's allowed:
 *   - Dated review docs under docs/reviews/** (per S11 spec: dated docs are
 *     frozen at their write date, so line refs are pinned by the review's date
 *     anchor and don't rot in the same way).
 *   - The linter script itself (this file talks about the pattern).
 *   - Occurrences on lines carrying `[file-line-ok]` marker for legitimate
 *     cases the author has consciously accepted (e.g. commit SHAs, GitHub
 *     permalinks that pin a line).
 *
 * For source files, only comment lines are scanned (lines starting with `//`
 * after leading whitespace, or lines inside a `/* ... *\/` block). String
 * literals are not scanned.
 *
 * Exits 1 if any violations found; prints filename:line and the offending
 * substring using GitHub-Actions `::error` format.
 */

import { readFileSync, readdirSync, statSync } from "node:fs";
import { join, relative, resolve, dirname } from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(__dirname, "..");

const REF_RE =
  /\b[a-zA-Z_][a-zA-Z0-9_]*\.(cpp|h|hpp|mjs|ts|astro|mdx|md|sh|js)\b:\d+(?:[-,]\d+)*/g;

const ALLOWLIST_MARKER = "[file-line-ok]";
const SELF_PATH = relative(REPO_ROOT, fileURLToPath(import.meta.url));

const DOC_ROOTS = [
  { dir: "site/src/content/docs", ext: [".mdx"] },
  { dir: "docs", ext: [".md"], excludePrefixes: ["docs/reviews/"] },
];

const SOURCE_ROOTS = [
  { dir: "engine/src", ext: [".cpp", ".h", ".hpp"] },
  { dir: "engine/include", ext: [".h", ".hpp"] },
  { dir: "plugin/source", ext: [".cpp", ".h", ".hpp"] },
];

function walk(dir, exts, out = []) {
  let entries;
  try {
    entries = readdirSync(dir);
  } catch {
    return out;
  }
  for (const name of entries) {
    const full = join(dir, name);
    let s;
    try {
      s = statSync(full);
    } catch {
      continue;
    }
    if (s.isDirectory()) {
      walk(full, exts, out);
    } else if (exts.some((e) => name.endsWith(e))) {
      out.push(full);
    }
  }
  return out;
}

const findings = [];

function record(path, line, snippet) {
  const rel = relative(REPO_ROOT, path);
  findings.push({ path: rel, line, snippet });
}

// --- Doc scan: every line, no comment-context check --------------------------
for (const { dir, ext, excludePrefixes = [] } of DOC_ROOTS) {
  const abs = join(REPO_ROOT, dir);
  const files = walk(abs, ext);
  for (const path of files) {
    const rel = relative(REPO_ROOT, path);
    if (excludePrefixes.some((p) => rel.startsWith(p))) continue;
    const text = readFileSync(path, "utf8");
    const lines = text.split("\n");
    for (let i = 0; i < lines.length; i++) {
      const line = lines[i];
      if (line.includes(ALLOWLIST_MARKER)) continue;
      // Reset regex state between lines (RE has /g).
      REF_RE.lastIndex = 0;
      const match = line.match(REF_RE);
      if (match) record(path, i + 1, match[0]);
    }
  }
}

// --- Source scan: only comment lines (// and /* ... */) ----------------------
function scanSource(path) {
  if (relative(REPO_ROOT, path) === SELF_PATH) return;
  const text = readFileSync(path, "utf8");
  const lines = text.split("\n");
  let inBlockComment = false;
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    if (line.includes(ALLOWLIST_MARKER)) continue;

    // Track block-comment context (naive but sufficient: this codebase uses
    // block comments only for multi-line commentary, not inside string
    // literals containing `/*` or `*/`).
    let scanRange = "";
    let idx = 0;
    let inCommentThisLine = inBlockComment;
    while (idx < line.length) {
      if (!inCommentThisLine) {
        const startBlock = line.indexOf("/*", idx);
        const startLine = line.indexOf("//", idx);
        const start =
          startBlock === -1
            ? startLine
            : startLine === -1
              ? startBlock
              : Math.min(startBlock, startLine);
        if (start === -1) break;
        if (start === startLine) {
          scanRange += " " + line.slice(start);
          idx = line.length;
        } else {
          inCommentThisLine = true;
          idx = start + 2;
        }
      } else {
        const end = line.indexOf("*/", idx);
        if (end === -1) {
          scanRange += " " + line.slice(idx);
          idx = line.length;
        } else {
          scanRange += " " + line.slice(idx, end);
          idx = end + 2;
          inCommentThisLine = false;
        }
      }
    }
    inBlockComment = inCommentThisLine;

    if (!scanRange) continue;
    REF_RE.lastIndex = 0;
    const match = scanRange.match(REF_RE);
    if (match) record(path, i + 1, match[0]);
  }
}

for (const { dir, ext } of SOURCE_ROOTS) {
  const abs = join(REPO_ROOT, dir);
  for (const path of walk(abs, ext)) scanSource(path);
}

if (findings.length > 0) {
  for (const f of findings) {
    console.error(
      `::error file=${f.path},line=${f.line}::file:line reference — cite a symbol/region name instead (or add [file-line-ok] if legitimately pinned)  [${f.snippet}]`,
    );
  }
  console.error(
    `\nFound ${findings.length} file:line reference(s). Dated docs under docs/reviews/** are exempt.`,
  );
  console.error("See scripts/check-file-line-refs.mjs header for rules.");
  process.exit(1);
}

console.log("No file:line references in enduring docs or source comments.");
