import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

// guide-parity M004/S01 (GP09). Rendering Mermaid at build time makes a real
// browser a dependency of building the docs site. That dependency is invisible
// in the one place it matters: `site-lint` never builds the site and `site-e2e`
// installs chromium already, so every pull-request check stays green while the
// Pages deploy -- which runs `npx astro build` with no browser -- breaks after
// merge. Nothing would have caught that before it shipped.
//
// This test ties the two facts together. If the Astro config renders Mermaid,
// then every workflow job that builds the site *or runs the site test suite*
// must install Playwright browsers. It is deliberately derived from the config
// rather than hardcoding "deploy-site.yml needs chromium": back the plugin out
// and the requirement lifts on its own, instead of leaving a stale rule behind.
//
// The "or runs the site test suite" half was missing when this test was first
// written, and its absence cost a red CI board on PR #314. The guard checked
// only jobs that BUILD, so when mermaid-syntax.test.mjs was later added to
// site-unit -- a test that launches a browser -- site-lint ran it with no
// browser installed and this guard was structurally unable to see it. A guard
// mutation-proved only on the cases it already covers proves nothing about the
// case it does not.

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = join(HERE, '..', '..');
const WORKFLOWS = join(REPO, '.github', 'workflows');

// Parsed with targeted matches rather than a YAML library. check-release-workflow.mjs
// states the convention: this repo carries no YAML dependency, and declaring one
// here added a `dep-freshness` red finding for a package pinned to 4.x on
// purpose. Workflow files are hand-written and regular; the arms below assert
// this parse is not silently matching nothing.
function jobsWithSteps(src) {
  const jobs = [];
  const jobsAt = src.indexOf('\njobs:');
  if (jobsAt === -1) return jobs;
  const body = src.slice(jobsAt);
  const jobRe = /^  ([A-Za-z0-9_-]+):$/gm;
  const marks = [...body.matchAll(jobRe)].map((m) => ({ name: m[1], at: m.index }));
  for (const [i, mark] of marks.entries()) {
    const end = i + 1 < marks.length ? marks[i + 1].at : body.length;
    jobs.push({ name: mark.name, body: body.slice(mark.at, end) });
  }
  return jobs;
}

const buildsTheSite = (body) => /astro build|npm run build\b/.test(body);
// `npm --prefix site test` and `npm test` inside site/ both reach the suite
// that renders mermaid; site-verify-local.sh builds, which the first matcher
// already covers.
const runsSiteTests = (body) => /npm\s+--prefix\s+site\s+test|npm\s+(run\s+)?test\b/.test(body);
const needsABrowser = (body) => buildsTheSite(body) || runsSiteTests(body);
const installsBrowsers = (body) => /playwright install/.test(body);

test('every workflow job that builds the site or runs its tests installs a browser', async () => {
  const config = await readFile(join(HERE, '..', 'astro.config.mjs'), 'utf8');
  assert.match(
    config,
    /rehype-mermaid/,
    'this test is scoped to the mermaid pipeline; if the plugin is gone, delete the test rather than weakening it',
  );

  const files = (await readdir(WORKFLOWS)).filter((f) => f.endsWith('.yml') || f.endsWith('.yaml'));
  const offenders = [];
  let jobsChecked = 0;

  for (const file of files) {
    const src = await readFile(join(WORKFLOWS, file), 'utf8');
    for (const job of jobsWithSteps(src)) {
      if (!needsABrowser(job.body)) continue;
      jobsChecked += 1;
      if (!installsBrowsers(job.body)) offenders.push(`${file}:${job.name}`);
    }
  }

  // Guard against the check silently covering nothing -- a workflow rename
  // would otherwise turn this into a green test over an empty set.
  assert.ok(
    jobsChecked > 0,
    'found no workflow job that builds the site or runs its tests; this test has stopped checking anything',
  );
  assert.deepEqual(
    offenders,
    [],
    `these jobs build the site or run its tests with no Playwright browser installed, so rehype-mermaid will fail there: ${offenders.join(', ')}`,
  );
});

// The browser CI installs must be the browser rehype-mermaid launches, and that
// is not automatic: both `playwright` and `@playwright/test` ship a `playwright`
// binary, so node_modules/.bin/playwright resolves to one of them while
// mermaid-isomorphic imports the other. On PR #314 they were 1.62.1 and 1.63.0,
// the installer fetched one build and the renderer demanded the other, and
// site-e2e failed with "Executable doesn't exist at chromium_headless_shell-1243"
// despite the job running `npx playwright install --with-deps chromium`.
//
// Locally this was invisible: a spike had already cached three chromium builds,
// so whichever version was asked for, it was there.
test('the playwright packages are the same version', async () => {
  const pkg = JSON.parse(await readFile(join(HERE, '..', 'package.json'), 'utf8'));
  const deps = { ...pkg.dependencies, ...pkg.devDependencies };
  const runner = deps['@playwright/test'];
  const lib = deps.playwright;

  assert.ok(runner, '@playwright/test is not declared');
  assert.ok(lib, 'playwright is not declared — rehype-mermaid needs it as a peer');
  assert.equal(
    runner,
    lib,
    `@playwright/test (${runner}) and playwright (${lib}) must match: the CI browser install uses whichever binary resolves, and rehype-mermaid launches the other`,
  );
});
