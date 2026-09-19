import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { load } from 'js-yaml';

// guide-parity M004/S01 (GP09). Rendering Mermaid at build time makes a real
// browser a dependency of building the docs site. That dependency is invisible
// in the one place it matters: `site-lint` never builds the site and `site-e2e`
// installs chromium already, so every pull-request check stays green while the
// Pages deploy -- which runs `npx astro build` with no browser -- breaks after
// merge. Nothing would have caught that before it shipped.
//
// This test ties the two facts together. If the Astro config renders Mermaid,
// then every workflow job that builds the site must install Playwright
// browsers. It is deliberately derived from the config rather than hardcoding
// "deploy-site.yml needs chromium": back the plugin out and the requirement
// lifts on its own, instead of leaving a stale rule behind.

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = join(HERE, '..', '..');
const WORKFLOWS = join(REPO, '.github', 'workflows');

const buildsTheSite = (step) => /astro build|npm run build\b/.test(String(step?.run ?? ''));
const installsBrowsers = (step) => /playwright install/.test(String(step?.run ?? ''));

test('every workflow job that builds the site installs a browser', async () => {
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
    const wf = load(await readFile(join(WORKFLOWS, file), 'utf8'));
    for (const [jobName, job] of Object.entries(wf?.jobs ?? {})) {
      const steps = job?.steps ?? [];
      if (!steps.some(buildsTheSite)) continue;
      jobsChecked += 1;
      if (!steps.some(installsBrowsers)) offenders.push(`${file}:${jobName}`);
    }
  }

  // Guard against the check silently covering nothing -- a workflow rename
  // would otherwise turn this into a green test over an empty set.
  assert.ok(jobsChecked > 0, 'found no workflow job that builds the site; this test has stopped checking anything');
  assert.deepEqual(
    offenders,
    [],
    `these jobs build the site with no Playwright browser installed, so rehype-mermaid will fail there: ${offenders.join(', ')}`,
  );
});
