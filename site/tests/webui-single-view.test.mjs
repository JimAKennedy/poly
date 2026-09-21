// M001/S02, FR05. Poly ships one main view. Cloth was a second, unfinished one,
// and it was removed in this slice rather than hidden — so the thing that can
// regress is not a flag flipping but a chip, a node or a mode switch coming
// back on someone's branch.
//
// This is a source assertion rather than a Playwright one on purpose. The
// `webui-e2e` token is declared pre-push-exempt in .jk/validations.yml (it needs
// a browser stack a clean checkout does not have), so a rendered-UI guard would
// only speak up in CI, one push too late. `site-unit` runs in the pre-push gate,
// and `site/tests/doc-conformance-wiring.test.mjs` is the precedent for
// asserting a repo-wide invariant from this directory.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const WEBUI = join(HERE, '..', '..', 'webui');

// A file that has moved must fail loudly rather than let an assertion pass over
// an empty string — the vacuity this repo has shipped once before.
async function webuiSource(name) {
  const text = await readFile(join(WEBUI, name), 'utf8');
  if (text.trim() === '') {
    throw new Error(`${name} is empty — has the WebUI moved? An empty source ` +
      'would let every assertion below pass while checking nothing.');
  }
  return text;
}

test('the shipped markup carries no mode chip and no cloth node', async () => {
  const html = await webuiSource('index.html');
  const offenders = [
    ['id="modes"', 'the mode-chip container'],
    ['mCloth', 'the Cloth chip'],
    ['mDesk', 'the Desk chip'],
    ['id="cloth"', 'the Cloth view'],
    ['id="loom"', 'the loom canvas'],
    ['learnBtn', 'the Learn chip'],
  ].filter(([needle]) => html.includes(needle));
  assert.deepEqual(
    offenders.map(([n, what]) => `${n} (${what})`),
    [],
    'Poly ships one view. A second one reaching index.html means a user can ' +
      'find an unfinished surface',
  );
});

test('the shipped script carries no mode switching', async () => {
  const js = await webuiSource('ui.js');
  const offenders = [
    ['setMode', 'the mode switcher'],
    ['drawLoom', 'the loom renderer'],
    ['sizeLoom', 'the loom sizer'],
    ['toggleLearn', 'the Learn toggle'],
  ].filter(([needle]) => js.includes(needle));
  assert.deepEqual(
    offenders.map(([n, what]) => `${n} (${what})`),
    [],
    'a view the markup cannot reach is still code that ships and still code ' +
      'that has to be maintained',
  );
});
