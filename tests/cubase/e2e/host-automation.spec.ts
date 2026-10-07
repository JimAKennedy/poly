import path from 'node:path';

import { test, expect } from '@playwright/test';

import {
  AUTOMATED_PITCH,
  CHANGE_PPQ,
  OFF_AT_BAR,
  automationFindings,
  requireComparable,
} from './lib/automation-contract';
import { readSettledProbe, remote, sessionDir } from './lib/cdp';
import { mutationActive } from './lib/preset-contract';
import { noteOns, parseProbeJsonl } from './lib/probe-assert';
import { splitPasses } from './lib/transport-contract';

// M004 S07 (DAW07) — host automation moves Poly's output when and where the
// lane says.
//
// Runs in a session of its own (scripts/cubase/start-session.ps1 -Name s07):
// it writes an automation lane into the open project, and every later spec in
// the same session would inherit it. remote.py `automate` plays three passes --
// baseline, write, read -- and the probe, which rewrites its JSONL on every
// transport stop, holds all three by the time the command returns.
//
// The lane is written through Cubase's own automation path: Poly's plugin W
// switch on, and lane 0's "Active" driven through MIDI Remote direct access
// while the transport rolls. The read pass is that lane played back by the
// host into the plugin's process() as parameter changes, which is exactly what
// the row says is untested. Why "Active", and the exact comparison, are in
// lib/automation-contract.ts.
//
// POLY_E2E_MUTATE=s07-flatten-lane: the spec asks the driver for a lane that
// never changes, so "the output changed at the position" must fail. The knob
// changes what the HOST is told to record, never what Poly does.

test.describe('L4: host automation lane (S07)', () => {
  test('lane 0 Active automated off at bar 3 silences the kick there and not before', async () => {
    const flat = mutationActive('s07-flatten-lane', process.env.POLY_E2E_MUTATE);
    remote('automate', '--off-at-bar', String(OFF_AT_BAR), ...(flat ? ['--flat'] : []));

    const probe = path.join(sessionDir(), 'probe.jsonl');
    const notes = noteOns(parseProbeJsonl(await readSettledProbe(probe), probe));
    const passes = splitPasses(notes);
    requireComparable(passes);

    const findings = automationFindings(passes[0], passes[2]);
    const detail = findings
      .map((f) => `  ppq ${f.ppq}: baseline [${f.baseline}] read-back [${f.automated}] -- ${f.why}`)
      .join('\n');
    expect(
      findings,
      `the read-back pass departs from a lane switching pitch ${AUTOMATED_PITCH} off at ` +
        `ppq ${CHANGE_PPQ}:\n${detail}`,
    ).toEqual([]);
  });
});
