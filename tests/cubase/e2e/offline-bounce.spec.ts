import { execFileSync } from 'node:child_process';
import path from 'node:path';

import { test, expect } from '@playwright/test';

import { applyMutation, bounceFindings, requireBothPasses } from './lib/bounce-contract';
import { REPO_ROOT, playScenario, readSettledProbe, remote, sessionDir } from './lib/cdp';
import { noteOns, parseProbeJsonl } from './lib/probe-assert';
import { splitPasses } from './lib/transport-contract';

// M004 S06 (DAW06) — Cubase's offline render and its realtime playback get the
// same notes from Poly.
//
// Runs in a session of its own (start-session.ps1 -Name s06) on a scratch
// copy: the export writes a .wav into the project's Mixdown/ folder, which must
// not be the committed fixture's. The passage is played in real time first
// (play_scenario.py), then the locator range is exported offline through
// Cubase's own Export Audio Mixdown -- opened by remote.py `export` and pressed
// by scripts/cubase/export-audio-mixdown.ps1, which returns once the render
// has finished. The probe sits in the render graph like any instrument track,
// so it captures the offline pass too, and the two passes are compared in
// lib/bounce-contract.ts, where the tolerance is written down.
//
// The plan for this slice named export-midi.spec.ts's route as the one to
// reuse. That route is Poly's own MIDI export, which renders SMF from the
// engine outside the host, so it never drives process() offline; the audio
// mixdown is what does, and is what this spec uses.
//
// POLY_E2E_MUTATE=s06-perturb-capture shifts one offline note by a 16th, so
// the comparison must fail naming its position.

test.describe('L4: offline bounce equivalence (S06)', () => {
  test('the offline export emits what realtime playback emitted', async () => {
    playScenario('--bars', '4', '--tempo', '120');

    remote('export');
    execFileSync(
      'pwsh',
      ['-NoProfile', '-File', path.join(REPO_ROOT, 'scripts', 'cubase', 'export-audio-mixdown.ps1')],
      { stdio: 'inherit', cwd: REPO_ROOT },
    );

    const probe = path.join(sessionDir(), 'probe.jsonl');
    const passes = splitPasses(noteOns(parseProbeJsonl(await readSettledProbe(probe), probe)));
    requireBothPasses(passes);

    const offline = applyMutation(passes[1], process.env.POLY_E2E_MUTATE);
    const findings = bounceFindings(passes[0], offline);
    const detail = findings
      .map((f) => `  ppq ${f.ppq}: realtime [${f.realtime}] offline [${f.offline}]`)
      .join('\n');
    expect(findings, `the offline render differs from realtime playback:\n${detail}`).toEqual([]);
  });
});
