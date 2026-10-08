import { test, expect } from '@playwright/test';

import {
  EDIT_LANE,
  MT_MS,
  MT_STEP,
  SessionContractError,
  applyMutation,
  compareFingerprints,
  fingerprint,
  requireEdited,
  type SessionFingerprint,
} from './session-contract';

// M004 S01 (DAW01) unit tests. No Cubase: the comparison and its guards are
// the logic that decides pass or fail. session-recall.spec.ts is the
// runner-gated half.

function saved(): SessionFingerprint {
  const mt = [0, 0, 0, 0];
  mt[MT_STEP] = MT_MS;
  return fingerprint({
    preset: 'Bembe 12/8',
    seed: 7,
    lanes: [
      { name: 'Kick', timeline: true, fixed: [1, 1, 0, 1], pattern: [1, 1, 0, 1], mt },
      { name: 'Snare', timeline: false, fixed: null, pattern: [0, 1, 0, 1], mt: [0, 0, 0, 0] },
    ],
  });
}

test.describe('compareFingerprints', () => {
  test('a session that comes back as saved has no differences', () => {
    expect(compareFingerprints(saved(), saved())).toEqual([]);
  });

  test('each edited field is caught when it does not survive', () => {
    const lost = saved();
    lost.preset = 'Init';
    lost.lanes[EDIT_LANE].timeline = false;
    lost.lanes[EDIT_LANE].fixed = null;
    lost.lanes[EDIT_LANE].mt[MT_STEP] = 0;
    const diffs = compareFingerprints(saved(), lost);
    expect(diffs.map((d) => d.split(':')[0])).toEqual([
      'preset',
      `lanes[${EDIT_LANE}].timeline`,
      `lanes[${EDIT_LANE}].fixed`,
      `lanes[${EDIT_LANE}].mt`,
    ]);
  });

  test('a lost lane is caught', () => {
    const fewer = saved();
    fewer.lanes.pop();
    expect(compareFingerprints(saved(), fewer)).toEqual(['lanes.length: 2 -> 1']);
  });
});

test.describe('requireEdited', () => {
  test('accepts a state carrying all three edits', () => {
    expect(() => requireEdited(saved(), 'Bembe 12/8')).not.toThrow();
  });

  test('rejects the default patch, whose round trip proves nothing', () => {
    const plain = saved();
    plain.preset = 'Init';
    plain.lanes[EDIT_LANE].timeline = false;
    plain.lanes[EDIT_LANE].mt[MT_STEP] = 0;
    expect(() => requireEdited(plain, 'Bembe 12/8')).toThrow(SessionContractError);
  });
});

test.describe('applyMutation (s01-perturb-state)', () => {
  test('is inert unless named', () => {
    expect(applyMutation(saved(), undefined)).toEqual(saved());
    expect(applyMutation(saved(), 's04-skip-reattach')).toEqual(saved());
  });

  test('turns an honest round trip red on exactly the edited step', () => {
    const perturbed = applyMutation(saved(), 's01-perturb-state');
    expect(compareFingerprints(perturbed, saved())).toEqual([
      `lanes[${EDIT_LANE}].mt: [0,0,${MT_MS + 1},0] -> [0,0,${MT_MS},0]`,
    ]);
  });

  test('does not mutate its input', () => {
    const input = saved();
    applyMutation(input, 's01-perturb-state');
    expect(input).toEqual(saved());
  });
});
