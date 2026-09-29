#!/usr/bin/env node
// gen-release-notes.mjs — emit the Release body for a version: the changelog
// section's "### For musicians" block, and one link to the full section.
//
// Usage:
//   node scripts/gen-release-notes.mjs 0.2.0
//   node scripts/gen-release-notes.mjs 0.2.0 > release-notes.md
//
// Behavior:
// - Reads CHANGELOG.md at repo root.
// - Finds the H2 section whose bracketed token matches the version
//   (`## [0.2.0]`, `## [0.2.0] - 2026-10-01`, case-insensitive).
// - Within it, finds `### For musicians` and prints the lines after it up to
//   the next H3 or H2, trimmed, then a blank line and
//   `Full changelog: https://github.com/JimAKennedy/poly/blob/v<version>/CHANGELOG.md`.
// - No section, or a section with no For musicians block: error to stderr,
//   exit 1, so a tag of that version halts instead of publishing.
//
// open-source-launch M005/S01 (OS28): release.yml publishes this output as
// the Release body. The engineering narrative stays in CHANGELOG.md, below
// the block, one link away; scripts/check-release-workflow.mjs holds the
// block to four headings and 400 words.

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

// The musician block: from the line after `### For musicians` to the line
// before the next H3 (the engineering categories) or the section's end.
let blockStart = -1;
for (let i = startIdx + 1; i < endIdx; i++) {
    if (/^###\s+For musicians\s*$/i.test(lines[i])) {
        blockStart = i + 1;
        break;
    }
}
if (blockStart === -1) {
    console.error(`No "### For musicians" block in the CHANGELOG section for "${version}"`);
    console.error(`The Release body is that block; add it above the section's engineering entries.`);
    process.exit(1);
}
let blockEnd = endIdx;
for (let i = blockStart; i < endIdx; i++) {
    if (/^###\s/.test(lines[i])) {
        blockEnd = i;
        break;
    }
}

const body = lines.slice(blockStart, blockEnd).join("\n").trim();
if (!body) {
    console.error(`The "### For musicians" block for "${version}" is empty`);
    process.exit(1);
}

const link = `Full changelog: https://github.com/JimAKennedy/poly/blob/v${version}/CHANGELOG.md`;
process.stdout.write(body + "\n\n" + link + "\n");
