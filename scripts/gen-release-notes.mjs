#!/usr/bin/env node
// gen-release-notes.mjs — extract a CHANGELOG.md section body ready for
// `gh release create --notes-file`. Maintainer aid, no CI wiring.
//
// Usage:
//   node scripts/gen-release-notes.mjs Unreleased
//   node scripts/gen-release-notes.mjs 0.1.0
//   node scripts/gen-release-notes.mjs 0.1.0 > notes.md
//
// Behavior:
// - Reads CHANGELOG.md at repo root.
// - Finds the H2 section whose header contains the given version token
//   (matches `## [Unreleased]`, `## [0.1.0]`, `## [0.1.0] - 2026-06-27`, etc).
// - Prints everything between that header and the next H2 (exclusive of
//   both headers), trimmed. Exits 0.
// - If no matching section, prints an error to stderr and exits 1.
//
// M048 S12: gives releases a predictable body without hand-copying prose.

import { readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = dirname(fileURLToPath(import.meta.url));
const repoRoot = dirname(scriptDir);
const changelogPath = join(repoRoot, "CHANGELOG.md");

const version = process.argv[2];
if (!version) {
    console.error("Usage: gen-release-notes.mjs <version>");
    console.error("  e.g. gen-release-notes.mjs Unreleased");
    console.error("  e.g. gen-release-notes.mjs 0.1.0");
    process.exit(2);
}

let src;
try {
    src = readFileSync(changelogPath, "utf8");
} catch (err) {
    console.error(`Cannot read ${changelogPath}: ${err.message}`);
    process.exit(2);
}

const lines = src.split("\n");
// H2 headers in Keep-a-Changelog format: `## [Version]` or `## [Version] - Date`.
const h2Indexes = [];
for (let i = 0; i < lines.length; i++) {
    if (/^##\s+\[/.test(lines[i])) h2Indexes.push(i);
}

const target = version.toLowerCase();
let startIdx = -1;
for (const idx of h2Indexes) {
    // Match the bracketed token case-insensitively, so `Unreleased`, `unreleased`,
    // and `0.1.0` all work without quoting.
    const m = lines[idx].match(/^##\s+\[([^\]]+)\]/);
    if (m && m[1].toLowerCase() === target) {
        startIdx = idx;
        break;
    }
}

if (startIdx === -1) {
    console.error(`No CHANGELOG section for version "${version}"`);
    console.error(`Available sections:`);
    for (const idx of h2Indexes) console.error(`  ${lines[idx]}`);
    process.exit(1);
}

// Next H2 after startIdx bounds the section.
let endIdx = lines.length;
for (const idx of h2Indexes) {
    if (idx > startIdx) {
        endIdx = idx;
        break;
    }
}

const body = lines.slice(startIdx + 1, endIdx).join("\n").trim();

if (!body) {
    console.error(`Section "${version}" is empty`);
    process.exit(1);
}

process.stdout.write(body + "\n");
