import { execFileSync } from 'node:child_process';
import { writeFileSync } from 'node:fs';

import { test, expect, chromium } from '@playwright/test';
import type { Browser, Page } from '@playwright/test';

import { stepToPpq } from './lib/probe-assert';
import {
  EXPECTED_HIT_FILE,
  KICK_PITCH,
  STEPS_PER_BAR,
  TOGGLE_STEP_INDEX,
  KICK_LANE_INDEX,
} from './lib/toggle-contract';

// M042 S09 — Playwright-over-CDP L4-web flagship.
//
// Attaches over CDP to the WebView2 that hosts Poly's editor INSIDE Cubase on
// the Windows runner, toggles a kick step OFF in the shipping WebUI, and plays
// the transport via the S08 mido driver. The fixture's kick lane starts fully
// lit, so the observable change is a REMOVED onset (asserted absent post-quit).
//
// It deliberately does NOT read the probe JSONL here: poly_midi_probe flushes
// its JSONL only on Cubase DEACTIVATE (setActive(false) in
// tools/midi_probe/source/probe_processor.cpp) — i.e. on quit, which happens in
// a later workflow step. So this spec records the expected hit to a file and the
// post-quit `assert-toggle` CLI does the probe assertion once the JSONL exists.
//
// This is runner-gated (R10) and Windows-only: WebView2 honors the CDP
// remote-debugging port, WKWebView (macOS) does not. On the dev machine this
// file only needs to typecheck/parse — the actual attach requires Cubase with
// -EnableCdp (scripts/cubase/launch-cubase.ps1) exposing CDP on POLY_CDP_PORT.

// --- Runner-provided environment ---------------------------------------------
// Use 127.0.0.1, NOT localhost: WebView2's --remote-debugging-port listener
// binds to IPv4 127.0.0.1 only, but Node resolves `localhost` to IPv6 ::1
// first, so `http://localhost:9222` gets `connect ECONNREFUSED ::1:9222` even
// when CDP is up on v4 (observed on the runner, M042 S09).
const CDP_ENDPOINT =
  process.env.POLY_CDP_ENDPOINT ||
  `http://127.0.0.1:${process.env.POLY_CDP_PORT || '9222'}`;
// The S08 mido driver drives the transport; path relative to repo root.
const DRIVER =
  process.env.POLY_DRIVER || 'tests/cubase/driver/play_scenario.py';

const ATTACH_TIMEOUT_MS = 30_000;
// WebView2's CDP endpoint appears asynchronously: the --remote-debugging-port
// listener only comes up after the plugin editor view renders, which can lag
// Cubase's "main window stable" signal. A one-shot connectOverCDP fails with
// ECONNREFUSED if it fires first. Retry the initial connect over this window.
const CONNECT_TIMEOUT_MS = 30_000;
const CONNECT_RETRY_MS = 1_000;

/**
 * Connect over CDP with retry. WebView2 opens its remote-debugging port only
 * after the editor renders (after Cubase reports its window stable), so the
 * first connect can be refused. Retry until the endpoint accepts or we time
 * out, then fail loud naming the likely cause.
 */
async function connectWithRetry(): Promise<Browser> {
  const deadline = Date.now() + CONNECT_TIMEOUT_MS;
  let lastErr: unknown;
  while (Date.now() < deadline) {
    try {
      return await chromium.connectOverCDP(CDP_ENDPOINT);
    } catch (err) {
      lastErr = err;
      await new Promise((r) => setTimeout(r, CONNECT_RETRY_MS));
    }
  }
  throw new Error(
    `Could not connect over CDP at ${CDP_ENDPOINT} within ${CONNECT_TIMEOUT_MS}ms. ` +
      `Likely cause: Cubase was not launched with -EnableCdp, or the Poly editor ` +
      `window is not open in the fixture, so WebView2 never exposed a CDP ` +
      `endpoint. Last error: ${lastErr instanceof Error ? lastErr.message : lastErr}`,
  );
}

/**
 * Attach over CDP and locate the page whose DOM is Poly's editor. WebView2 may
 * expose several targets (Cubase's own webviews, etc.); we pick the one that has
 * the `.strip` lane containers the WebUI renders. Fails loud, naming the likely
 * cause (remote-debugging-port not set), if none appears within the timeout.
 */
async function findPolyEditor(browser: Browser): Promise<Page> {
  const deadline = Date.now() + ATTACH_TIMEOUT_MS;
  let seenTargets = 0;
  while (Date.now() < deadline) {
    for (const context of browser.contexts()) {
      for (const page of context.pages()) {
        seenTargets++;
        const isPoly = await page
          .locator('.strip')
          .first()
          .isVisible()
          .catch(() => false);
        if (isPoly) return page;
      }
    }
    await new Promise((r) => setTimeout(r, 500));
  }
  throw new Error(
    `No Poly editor page found among ${seenTargets} WebView2 target(s) within ` +
      `${ATTACH_TIMEOUT_MS}ms. Likely cause: Cubase was not launched with ` +
      `-EnableCdp (WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS=--remote-debugging-port), ` +
      `so no CDP endpoint is exposed at ${CDP_ENDPOINT}.`,
  );
}

test.describe('L4-web: Playwright over CDP toggles a step inside Cubase', () => {
  // Serial: a single Cubase/CDP session, single runner.
  test.describe.configure({ mode: 'serial' });

  test('toggle kick step and play transport (probe asserted post-quit)', async () => {
    const browser = await connectWithRetry();
    try {
      const page = await findPolyEditor(browser);

      // Expand the kick lane and enter timeline (fixed-pattern) mode so we can
      // toggle an individual step. Selectors mirror the shipping WebUI proven by
      // site/tests-e2e/webui-timeline-toggle.spec.ts.
      const lane = page.locator('.strip').nth(KICK_LANE_INDEX);
      await lane.locator('.ex').click();
      await expect(lane).toHaveClass(/expanded/);

      const patternPane = lane.locator('[data-pane="pattern"]');
      const tlToggle = patternPane.locator('[data-tl]');
      // If not already in timeline mode, flip it on. In timeline mode the ladder
      // step buttons become click-to-toggle (ui.js buildLadder: the click handler
      // is only wired when l.timeline is set); in Euclidean mode they are
      // display-only.
      if (!(await tlToggle.evaluate((el) => el.classList.contains('on')))) {
        await tlToggle.click();
      }

      // The per-step toggle buttons live in the lane's `.ladder` group — a SIBLING
      // of the pattern pane, not inside it (ui.js renders `.ladder` in `.core`,
      // the panes in `.deep`; the pane prose even says "Edit steps on the ladder
      // above"). There is no `[data-fixed]` grid — that was a removed design
      // (webui/tests/interaction.spec.mjs asserts `[data-fixed]` count is 0). The
      // shipping toggle path is proven by interaction.spec.mjs: click
      // `.strip .ladder button` and assert the `hit` class.
      const ladder = lane.locator('.ladder');
      await expect(ladder.locator('button').first()).toBeVisible();

      // Toggle the target step. The fixture's kick lane starts fully lit (all 4
      // steps ON — see toggle-contract.ts), so this click turns step 2 OFF, which
      // must REMOVE its note-on from the probe. We do NOT read the `hit` CSS class
      // as an oracle: the WebUI sets `hit` on every ladder button for a lane with
      // additive cells (ui.js buildLadder: `l.cells || l.pattern[i]`), so it never
      // reflects the engine's real fixedPattern. The probe is the true oracle —
      // asserted post-quit by assert-probe.spec.ts.
      const step = ladder.locator('button').nth(TOGGLE_STEP_INDEX);
      await step.click();

      // Record the expected change for the post-quit assertion. The probe file
      // doesn't exist yet — it's written on Cubase deactivate (quit), a later
      // workflow step — so we hand the expectation to assert-probe.spec.ts.
      // `absent: true` because we toggled the step OFF: its note-on must be gone.
      const expectedPpq = stepToPpq(TOGGLE_STEP_INDEX, STEPS_PER_BAR);
      writeFileSync(
        EXPECTED_HIT_FILE,
        JSON.stringify(
          { pitch: KICK_PITCH, ppq: expectedPpq, absent: true },
          null,
          2,
        ),
      );

      // Drive the transport via the S08 mido driver (it waits for the MIDI
      // Remote ready ping, starts, plays, stops). Poly's output is captured by
      // poly_midi_probe and flushed on the subsequent Cubase quit.
      execFileSync('python', [DRIVER, '--bars', '4', '--tempo', '120'], {
        stdio: 'inherit',
      });
    } finally {
      await browser.close();
    }
  });
});
