import { execFileSync } from 'node:child_process';
import path from 'node:path';

import { test, expect } from '@playwright/test';
import type { Browser, Page } from '@playwright/test';

import {
  REPO_ROOT,
  connect,
  playScenario,
  polyEditor,
  readSettledProbe,
  remote,
  requestState,
  sessionDir,
  waitForEndpoint,
} from './lib/cdp';
import { mutationActive } from './lib/preset-contract';

// M004 S04 (DAW04) — Poly survives its editor being closed and reopened.
//
// CLAUDE.md records two conventions this exercises: some hosts call
// setActive() from the audio thread, and allocateMessage()/sendMessage() in
// process() is not guaranteed lock-free. An editor cycle is where both bite --
// the view and its WebView2 are torn down and rebuilt while the processor keeps
// running -- and before this spec nothing in a host ever did it.
//
// The editor is closed and opened through Cubase itself (remote.py `editor`,
// MIDI Remote CC 24 on the instrument slot's edit switch), not by killing a
// window, so the plugin sees the host's own close path.
//
// After the last cycle two things must hold:
//   1. the bridge answers a round trip -- a `ready` request gets a `state`
//      push back from the native side (lib/cdp.ts requestState), and
//   2. the processor still plays the fixture's patch: four bars captured by the
//      probe match the in-process golden, through the same comparison the
//      nightly's first session uses.
//
// Re-attaching is the part that has to be right, and lib/cdp.ts records why:
// closing the editor takes the CDP endpoint away entirely and reopening brings
// up a new one. POLY_E2E_MUTATE=s04-skip-reattach keeps the stale handle
// instead of reconnecting, so assertion 1 must fail -- the failure the row
// names.
//
// Runs in its own session (start-session.ps1 -Name s04) on an unedited copy of
// the fixture, so the probe holds exactly this spec's passage.

const CYCLES = 3;
const GOLDEN =
  process.env.POLY_GOLDEN ||
  path.join(REPO_ROOT, 'tests', 'golden', 'processor_default_4bars.txt');

test.describe('L4: editor lifecycle (S04)', () => {
  test(`the bridge and the processor survive ${CYCLES} editor close/open cycles`, async () => {
    const skipReattach = mutationActive('s04-skip-reattach', process.env.POLY_E2E_MUTATE);

    let browser: Browser = await connect();
    let page: Page = await polyEditor(browser);
    const before = await requestState(page);
    expect(before.lanes.length, 'the editor reported no lanes before cycling').toBeGreaterThan(0);

    for (let cycle = 1; cycle <= CYCLES; cycle++) {
      remote('editor', 'close');
      await waitForEndpoint(false);
      remote('editor', 'open');
      await waitForEndpoint(true);
      if (!skipReattach) {
        await browser.close().catch(() => {});
        browser = await connect();
        page = await polyEditor(browser);
      }
    }

    try {
      const after = await requestState(page);
      expect(after.lanes.length, 'lane count changed across the editor cycles').toBe(
        before.lanes.length,
      );
      expect(after.preset).toBe(before.preset);
    } finally {
      await browser.close().catch(() => {});
    }

    playScenario('--bars', '4', '--tempo', '120');
    const probe = path.join(sessionDir(), 'probe.jsonl');
    await readSettledProbe(probe);
    // Same arguments as the nightly's "Compare probe output to golden" step,
    // minus --expected-hit: nothing in this session toggled a step.
    execFileSync(
      'python',
      [
        path.join(REPO_ROOT, 'tests', 'cubase', 'compare_probe_golden.py'),
        '--probe',
        probe,
        '--first-pass-only',
        '--golden',
        GOLDEN,
      ],
      { stdio: 'inherit', cwd: REPO_ROOT },
    );
  });
});
