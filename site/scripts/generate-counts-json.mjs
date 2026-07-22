#!/usr/bin/env node
// Derives site/src/generated/counts.json from the true sources:
//   - presets/categories: site/src/generated/presets.json (produced by generate-presets)
//   - lanesMax: kMaxLanes in engine/include/poly/types.h
//   - chaptersCount: number of NN-*.mdx chapter files under site/src/content/docs/
// The MDX index page imports this JSON to render "{counts.presets} presets" so a
// preset addition upstream flips the site copy on next rebuild instead of drifting.
// Must run AFTER generate-presets in the site build pipeline.

import { existsSync, readFileSync, readdirSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, '..', '..');
const PRESETS_JSON = resolve(REPO_ROOT, 'site', 'src', 'generated', 'presets.json');
const TYPES_H = resolve(REPO_ROOT, 'engine', 'include', 'poly', 'types.h');
const CHAPTERS_DIR = resolve(REPO_ROOT, 'site', 'src', 'content', 'docs');
const OUT_PATH = resolve(REPO_ROOT, 'site', 'src', 'generated', 'counts.json');

function log(msg) {
  process.stdout.write(`[generate-counts] ${msg}\n`);
}

function fail(msg) {
  process.stderr.write(`[generate-counts] ${msg}\n`);
  process.exit(1);
}

function readPresets() {
  if (!existsSync(PRESETS_JSON)) {
    fail(`presets.json not found at ${PRESETS_JSON} — run generate-presets first`);
  }
  let parsed;
  try {
    parsed = JSON.parse(readFileSync(PRESETS_JSON, 'utf8'));
  } catch (e) {
    fail(`presets.json is not valid JSON: ${e.message}`);
  }
  if (typeof parsed.presetCount !== 'number') fail('presets.json missing presetCount');
  if (!Array.isArray(parsed.categories)) fail('presets.json missing categories array');
  return { presets: parsed.presetCount, categories: parsed.categories.length };
}

function readLanesMax() {
  if (!existsSync(TYPES_H)) fail(`types.h not found at ${TYPES_H}`);
  const src = readFileSync(TYPES_H, 'utf8');
  const m = src.match(/constexpr\s+int\s+kMaxLanes\s*=\s*(\d+)\s*;/);
  if (!m) fail('could not locate `constexpr int kMaxLanes = N;` in types.h');
  return parseInt(m[1], 10);
}

function countChapters() {
  if (!existsSync(CHAPTERS_DIR)) fail(`chapters dir not found at ${CHAPTERS_DIR}`);
  return readdirSync(CHAPTERS_DIR).filter((f) => /^\d{2}-.*\.mdx$/.test(f)).length;
}

function main() {
  const { presets, categories } = readPresets();
  const lanesMax = readLanesMax();
  const chaptersCount = countChapters();

  // generatedAt intentionally omitted — a fresh timestamp on every build would
  // create no-op git diff churn for site/src/generated/counts.json. The counts
  // themselves are the whole point; if they change, the file changes.
  const out = {
    schemaVersion: 1,
    presets,
    categories,
    lanesMax,
    chaptersCount,
  };

  mkdirSync(dirname(OUT_PATH), { recursive: true });
  writeFileSync(OUT_PATH, JSON.stringify(out, null, 2) + '\n');
  log(
    `wrote ${OUT_PATH.replace(REPO_ROOT + '/', '')} — presets:${presets} categories:${categories} lanesMax:${lanesMax} chaptersCount:${chaptersCount}`,
  );
}

main();
