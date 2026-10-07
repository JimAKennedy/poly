import { readFileSync, statSync, writeFileSync } from 'node:fs';
import path from 'node:path';

import { test, expect } from '@playwright/test';

import { connect, polyEditor, remote, requestState, sendAction, sendEdit, sessionDir } from './lib/cdp';
import {
  EDIT_LANE,
  EDIT_STEP,
  EXPECTED_FILE,
  MT_MS,
  MT_STEP,
  PRESET_INDEX,
  applyMutation,
  compareFingerprints,
  fingerprint,
  requireEdited,
  type SessionFingerprint,
} from './lib/session-contract';

// M004 S01 (DAW01) — a saved Cubase project reopens with Poly's patch intact.
//
// The state round trip is the one place kStateVersion and its version-branched
// setState() meet a real host, and CLAUDE.md calls an unversioned state "a
// preset compatibility time bomb". Before this spec nothing saved a project
// and reopened it.
//
// Two passes of the same spec, selected by POLY_SESSION_MODE, so the contract
// cannot drift between them:
//
//   save    -- in a session on a SCRATCH COPY of the fixture
//              (start-session.ps1 -Name s01 -CopyFixture): apply the edits in
//              lib/session-contract.ts, read the state back from the native
//              side, write it to the session dir, and have Cubase save
//              (remote.py save). The save is confirmed by the project file's
//              modification time, not assumed from the CC having been sent.
//   reopen  -- after a quit and a fresh launch on that saved copy
//              (start-session.ps1 -Name s01 -FixtureCpr <the copy>): read the
//              state again and compare it with what was saved.
//
// The committed fixture is never written: the copy lives under the session
// dir, which tests/cubase/fixtures/README.md requires of any run that saves.
//
// POLY_E2E_MUTATE=s01-perturb-state changes the expected micro-timing before
// the reopen comparison (lib/session-contract.ts applyMutation), so the
// comparison must fail on that field.

const SAVE_CONFIRM_TIMEOUT_MS = 20_000;
const STATE_SETTLE_MS = 1_000;

const sleep = (ms: number) => new Promise((r) => setTimeout(r, ms));

function projectFile(): string {
  const cpr = process.env.POLY_SESSION_CPR;
  if (!cpr) throw new Error('POLY_SESSION_CPR is not set -- start this session with start-session.ps1');
  return cpr;
}

test.describe('L4: session recall (S01)', () => {
  test.skip(
    !['save', 'reopen'].includes(process.env.POLY_SESSION_MODE ?? ''),
    'POLY_SESSION_MODE must be save or reopen',
  );

  test('save: a non-default patch is applied, recorded and saved', async () => {
    test.skip(process.env.POLY_SESSION_MODE !== 'save');
    const expectedPath = path.join(sessionDir(), EXPECTED_FILE);
    const cpr = projectFile();

    const browser = await connect();
    try {
      const page = await polyEditor(browser);

      await sendAction(page, 'applyPreset', { index: PRESET_INDEX });
      await sleep(STATE_SETTLE_MS);
      const presetName = (await requestState(page)).preset;

      await sendEdit(page, `lane.${EDIT_LANE}.timeline`, 1.0);
      await sleep(STATE_SETTLE_MS);
      await sendAction(page, 'toggleStep', { lane: EDIT_LANE, step: EDIT_STEP });
      await sendAction(page, 'setMicroTiming', { lane: EDIT_LANE, step: MT_STEP, ms: MT_MS });
      await sleep(STATE_SETTLE_MS);

      const saved = fingerprint(await requestState(page));
      requireEdited(saved, presetName);
      writeFileSync(expectedPath, JSON.stringify(saved, null, 2));
    } finally {
      await browser.close().catch(() => {});
    }

    const before = statSync(cpr).mtimeMs;
    remote('save');
    const deadline = Date.now() + SAVE_CONFIRM_TIMEOUT_MS;
    while (statSync(cpr).mtimeMs === before && Date.now() < deadline) await sleep(500);
    expect(
      statSync(cpr).mtimeMs,
      `${cpr} was not rewritten within ${SAVE_CONFIRM_TIMEOUT_MS}ms of File > Save`,
    ).not.toBe(before);
  });

  test('reopen: the reloaded patch is the one that was saved', async () => {
    test.skip(process.env.POLY_SESSION_MODE !== 'reopen');
    const expectedPath = path.join(sessionDir(), EXPECTED_FILE);
    const expected = applyMutation(
      JSON.parse(readFileSync(expectedPath, 'utf-8')) as SessionFingerprint,
      process.env.POLY_E2E_MUTATE,
    );

    const browser = await connect();
    try {
      const page = await polyEditor(browser);
      const actual = fingerprint(await requestState(page));
      const diffs = compareFingerprints(expected, actual);
      expect(
        diffs,
        `the reopened project's patch differs from the one saved:\n  ${diffs.join('\n  ')}`,
      ).toEqual([]);
    } finally {
      await browser.close().catch(() => {});
    }
  });
});
