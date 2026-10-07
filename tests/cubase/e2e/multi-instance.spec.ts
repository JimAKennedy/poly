import path from 'node:path';

import { test, expect } from '@playwright/test';

import {
  connect,
  playScenario,
  polyEditors,
  readSettledProbe,
  requestState,
  sendAction,
  sessionDir,
} from './lib/cdp';
import { SECOND_PRESET_INDEX, SECOND_PRESET_NAME, asymmetry } from './lib/instance-contract';
import { mutationActive } from './lib/preset-contract';
import { noteOns, parseProbeJsonl } from './lib/probe-assert';
import { splitPasses } from './lib/transport-contract';

// M004 S05 (DAW05) — two Poly instances in one project, each with its own
// state, bridge and output.
//
// Runs in its own session on a scratch copy of poly-2instance.cpr
// (start-session.ps1 -Name s05 -CopyFixture). Both editors are saved open in
// the fixture, and both WebView2s share one CDP endpoint -- one browser
// process, two pages, observed on the runner (2026-10-07) -- so the spec can
// reach each instance's bridge in turn. Page order is NOT instance order: the
// instances are told apart by the state their own bridge reports.
//
// Three isolations are checked, one per thing the row says is assumed:
//   state   -- each editor's bridge round trip reports a different patch, the
//              default on one and the fixture's second preset on the other;
//   output  -- each probe captures notes the other does not (asymmetry, see
//              lib/instance-contract.ts);
//   probe   -- the two probes write two files: probe.jsonl and probe-2.jsonl.
//
// POLY_E2E_MUTATE=s05-shared-state applies the second instance's preset to the
// first through the first one's bridge before playing, so both instances run
// one patch and the asymmetry must fail. That is the row's "both pointed at one
// state blob" in testable form, done through the editor's own action path.

test.describe('L4: two Poly instances (S05)', () => {
  test('each instance keeps its own patch, bridge and output', async () => {
    const shared = mutationActive('s05-shared-state', process.env.POLY_E2E_MUTATE);

    const browser = await connect();
    try {
      const pages = await polyEditors(browser, 2);
      expect(pages.length, 'expected exactly two Poly editors over CDP').toBe(2);
      const states = await Promise.all(pages.map((p) => requestState(p)));
      const second = states.findIndex((s) => s.preset === SECOND_PRESET_NAME);
      const first = states.findIndex((s) => s.preset !== SECOND_PRESET_NAME);
      expect(
        second >= 0 && first >= 0 && first !== second,
        `the two bridges should report different patches; they report ` +
          `[${states.map((s) => s.preset).join(', ')}]`,
      ).toBe(true);

      if (shared) {
        await sendAction(pages[first], 'applyPreset', { index: SECOND_PRESET_INDEX });
        await new Promise((r) => setTimeout(r, 1_000));
      }
    } finally {
      await browser.close().catch(() => {});
    }

    playScenario('--bars', '4', '--tempo', '120');

    const captures = await Promise.all(
      ['probe.jsonl', 'probe-2.jsonl'].map(async (name) => {
        const file = path.join(sessionDir(), name);
        const notes = noteOns(parseProbeJsonl(await readSettledProbe(file), file));
        expect(notes.length, `${name} captured no notes`).toBeGreaterThan(0);
        return splitPasses(notes)[0];
      }),
    );

    const a = asymmetry(captures[0], captures[1]);
    expect(
      a.onlyFirst > 0 && a.onlySecond > 0,
      `each instance's capture should hold notes the other's does not; ` +
        `probe.jsonl has ${a.onlyFirst} of its own, probe-2.jsonl has ${a.onlySecond}`,
    ).toBe(true);
  });
});
