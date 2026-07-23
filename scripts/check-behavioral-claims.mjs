#!/usr/bin/env node
// Enforces the M048 S09 behavioral-claims convention: any doc sentence
// marked `[verified: TestSuite.TestName]` must cite an existing test, and
// `[⚠ unverified]` markers are counted as warnings so the honest-state count
// is visible over time.
//
// Opt-in by design. The linter does NOT try to auto-detect behavioral claims
// via verb-matching — 131 verb hits across chapters includes musical prose
// ("this pattern produces a syncopated feel") that isn't a Poly-implementation
// claim. Authors mark claims explicitly where machine-verified truth matters.
//
// Test index sources:
//   - GTest suites in tests/**/*.cpp — `TEST(Suite, Name)` and `TEST_F(Suite, Name)`
//   - Playwright specs in site/tests-e2e/**/*.spec.ts — file basename plus `test('name')`
//   - Node tests in site/tests/**/*.test.mjs — `test('name')`
//
// Marker syntax examples:
//   [verified: HostTests.Determinism_SameInputSameOutput]
//   [verified: appendix-euclidean-claims/every-appendix-row-pattern-is-a-rotation-of-canonical-Bjorklund]
//   [⚠ unverified]
//
// CI wiring: .github/workflows/ci.yml site-lint job.

import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, resolve, relative, basename } from 'node:path';
import { execSync } from 'node:child_process';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, '..');

const VERIFIED_RE = /\[verified:\s*([^\]]+?)\s*\]/g;
const UNVERIFIED_RE = /\[⚠\s*unverified\]/g;

const GTEST_RE = /\bTEST(?:_F)?\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)/g;
const JS_TEST_RE = /\btest\s*\(\s*['"`]([^'"`]+)['"`]/g;

function listFiles(pattern, cwd) {
  // git ls-files uses fnmatch, not shell globbing — `**` doesn't cross
  // directory boundaries. Enumerate a directory root, then filter by extension.
  const [dir, ext] = pattern.split(':');
  try {
    const out = execSync(`git ls-files -- ${dir}`, { cwd, encoding: 'utf8' });
    return out
      .split('\n')
      .filter(Boolean)
      .filter((f) => f.endsWith(ext));
  } catch {
    return [];
  }
}

function buildTestIndex() {
  const index = new Set();

  // GTest — tests/**/*.cpp
  for (const rel of listFiles('tests/:.cpp', REPO_ROOT)) {
    const src = readFileSync(resolve(REPO_ROOT, rel), 'utf8');
    let m;
    while ((m = GTEST_RE.exec(src)) !== null) {
      index.add(`${m[1]}.${m[2]}`);
    }
    GTEST_RE.lastIndex = 0;
  }

  // Playwright — site/tests-e2e/**/*.spec.ts (fileBasename/testName)
  for (const rel of listFiles('site/tests-e2e/:.spec.ts', REPO_ROOT)) {
    const src = readFileSync(resolve(REPO_ROOT, rel), 'utf8');
    const stem = basename(rel).replace(/\.spec\.ts$/, '');
    let m;
    while ((m = JS_TEST_RE.exec(src)) !== null) {
      const slug = m[1].toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '');
      index.add(`${stem}/${slug}`);
    }
    JS_TEST_RE.lastIndex = 0;
  }

  // Node — site/tests/**/*.test.mjs
  for (const rel of listFiles('site/tests/:.test.mjs', REPO_ROOT)) {
    const src = readFileSync(resolve(REPO_ROOT, rel), 'utf8');
    const stem = basename(rel).replace(/\.test\.mjs$/, '');
    let m;
    while ((m = JS_TEST_RE.exec(src)) !== null) {
      const slug = m[1].toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '');
      index.add(`${stem}/${slug}`);
    }
    JS_TEST_RE.lastIndex = 0;
  }

  return index;
}

function scanDocs(testIndex) {
  const errors = [];
  let unverifiedCount = 0;
  const docPatterns = ['docs/:.md', 'site/src/content/docs/:.mdx'];
  const files = [];
  for (const p of docPatterns) files.push(...listFiles(p, REPO_ROOT));

  for (const relPath of files) {
    const abs = resolve(REPO_ROOT, relPath);
    const src = readFileSync(abs, 'utf8');
    const lines = src.split('\n');
    lines.forEach((line, i) => {
      const lineNo = i + 1;

      let m;
      while ((m = VERIFIED_RE.exec(line)) !== null) {
        const citation = m[1];
        if (!testIndex.has(citation)) {
          errors.push({
            file: relPath,
            line: lineNo,
            message: `unresolved test citation: [verified: ${citation}]. Not found in tests/**/*.cpp GTest names or site/tests/site/tests-e2e slugs.`,
          });
        }
      }
      VERIFIED_RE.lastIndex = 0;

      while ((m = UNVERIFIED_RE.exec(line)) !== null) {
        unverifiedCount++;
        // Emit a warning line but don't fail — the marker is the honest state.
        process.stdout.write(
          `::warning file=${relPath},line=${lineNo}::unverified behavioral claim — S09 tracking.\n`,
        );
      }
      UNVERIFIED_RE.lastIndex = 0;
    });
  }

  return { errors, unverifiedCount };
}

const testIndex = buildTestIndex();
const { errors, unverifiedCount } = scanDocs(testIndex);

if (errors.length > 0) {
  for (const e of errors) {
    process.stderr.write(`::error file=${e.file},line=${e.line}::${e.message}\n`);
  }
  process.stderr.write(
    `\nFound ${errors.length} unresolved test citation(s). ` +
      `Either fix the Suite.Name / spec/slug in the marker, or add the test, or replace with [⚠ unverified].\n`,
  );
  process.exit(1);
}

process.stdout.write(
  `Behavioral claims lint OK — ${testIndex.size} test(s) indexed, ${unverifiedCount} unverified marker(s).\n`,
);
