import { test, expect } from '@playwright/test';

import {
  PresetContractError,
  applyMutation,
  mutationActive,
  comparePresets,
  loadPresetExpectations,
  type PresetExpectation,
} from './preset-contract';

// M004 S02 (DAW02) unit tests. No browser, no Cubase: these prove the
// comparison decides pass and fail correctly, which is the half of the sweep
// that can be proved without a DAW. The runner-gated half is
// preset-recall.spec.ts.

const EXPECTED: PresetExpectation[] = [
  { index: 0, name: 'A', laneCount: 2, noteNumbers: [36, 38] },
  { index: 1, name: 'B', laneCount: 1, noteNumbers: [42] },
];

test.describe('loadPresetExpectations', () => {
  test('reads the shipped presets.json and finds every preset named and laned', () => {
    const all = loadPresetExpectations();
    expect(all.length).toBeGreaterThan(0);
    for (const p of all) {
      expect(p.name.length).toBeGreaterThan(0);
      expect(p.laneCount).toBeGreaterThan(0);
      expect(p.noteNumbers.length).toBe(p.laneCount);
    }
  });

  test('a missing file throws rather than yielding an empty contract', () => {
    expect(() => loadPresetExpectations('/nonexistent/presets.json')).toThrow(
      PresetContractError,
    );
  });
});

test.describe('comparePresets', () => {
  test('identical observation yields no problems', () => {
    expect(comparePresets(EXPECTED, EXPECTED.map((p) => ({ ...p })))).toEqual([]);
  });

  test('a wrong lane count is reported and names the preset', () => {
    const observed = [{ index: 0, laneCount: 3, noteNumbers: [36, 38] }, EXPECTED[1]];
    const problems = comparePresets(EXPECTED, observed);
    expect(problems.length).toBe(1);
    expect(problems[0]).toContain('preset 0 (A)');
    expect(problems[0]).toContain('3 lanes in the host');
  });

  test('wrong note numbers are reported', () => {
    const observed = [{ index: 0, laneCount: 2, noteNumbers: [36, 99] }, EXPECTED[1]];
    expect(comparePresets(EXPECTED, observed)[0]).toContain('[36,99] in the host');
  });

  test('a preset never observed is reported rather than skipped', () => {
    expect(comparePresets(EXPECTED, [EXPECTED[0]])[0]).toContain('was never observed');
  });

  test('every mismatch is reported, not just the first', () => {
    const observed = [
      { index: 0, laneCount: 9, noteNumbers: [36, 38] },
      { index: 1, laneCount: 9, noteNumbers: [42] },
    ];
    expect(comparePresets(EXPECTED, observed).length).toBe(2);
  });
});

test.describe('applyMutation', () => {
  test('without the knob the contract is untouched', () => {
    expect(applyMutation(EXPECTED, undefined)).toEqual(EXPECTED);
    expect(applyMutation(EXPECTED, 's04-skip-reattach')).toEqual(EXPECTED);
  });

  test('the knob makes a correct observation fail, which is the red path', () => {
    const mutated = applyMutation(EXPECTED, 's02-malformed-preset');
    const problems = comparePresets(mutated, EXPECTED.map((p) => ({ ...p })));
    expect(problems.length).toBe(1);
    expect(problems[0]).toContain('lanes in the host');
  });
});

test.describe('mutationActive', () => {
  test('an unset knob activates nothing', () => {
    expect(mutationActive('s02-malformed-preset', undefined)).toBe(false);
    expect(mutationActive('s02-malformed-preset', '')).toBe(false);
  });

  test('a single value activates only itself', () => {
    expect(mutationActive('s02-malformed-preset', 's02-malformed-preset')).toBe(true);
    expect(mutationActive('s04-skip-reattach', 's02-malformed-preset')).toBe(false);
  });

  // The whole point of the list: one red dispatch must be able to turn every
  // M004 spec red at once. A single-valued knob would leave the others green.
  test('a list activates every name in it, comma or space separated', () => {
    const list = 's02-malformed-preset,s03-accumulated-phase s04-skip-reattach';
    expect(mutationActive('s02-malformed-preset', list)).toBe(true);
    expect(mutationActive('s03-accumulated-phase', list)).toBe(true);
    expect(mutationActive('s04-skip-reattach', list)).toBe(true);
    expect(mutationActive('s07-flatten-lane', list)).toBe(false);
  });

  test('a substring is not a match', () => {
    expect(mutationActive('s02', 's02-malformed-preset')).toBe(false);
  });
});
