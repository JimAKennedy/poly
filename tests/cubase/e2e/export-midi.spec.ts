import { execFileSync } from 'node:child_process';
import { existsSync, rmSync, statSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

import { test, expect, chromium } from '@playwright/test';
import type { Browser, Page } from '@playwright/test';

import {
  EXPORT_SINK,
  EXPORT_ALL_LANE_NAMES,
  EXPORT_LANE_INDEX,
  EXPORT_LANE_NAME,
} from './lib/export-contract';

// M032 export UAT automation (Level A) — in-DAW MIDI export over CDP.
//
// This is the automated replacement for M032's deferred manual UAT: "export a
// .mid from the plugin, open it in Cubase, and confirm N named tracks / no
// dropped notes." It attaches over CDP to the WebView2 hosting Poly's editor
// INSIDE Cubase on the Windows runner, clicks the shipping Export chip (all
// lanes) and a per-lane export handle, and validates each resulting .mid with an
// independent SMF parser (tests/cubase/validate_smf_export.py, mido).
//
// The plugin runs in POLY_EXPORT_SINK mode (set by the workflow at Cubase
// launch): exportSaveAs writes the exact SMF bytes to the sink path and SKIPS
// the modal native Save-As panel — otherwise the dialog would block the UI
// thread forever under the unattended runner (web_ui_view.cpp
// openMidiExportDialog). So this spec drives the REAL in-plugin export code path
// (renderCurrentPatternSmf -> writeMultiTrackSMF) and asserts its output, which
// is exactly what the manual "open in Cubase" step confirmed — minus Cubase's
// own importer, which remains the one residual manual check.
//
// Runner-gated (R10) and Windows-only: WebView2 honors the CDP port, WKWebView
// (macOS) does not. On the dev machine this file only needs to typecheck/parse.

// Use 127.0.0.1, NOT localhost: WebView2's CDP listener binds IPv4 only, but
// Node resolves `localhost` to IPv6 ::1 first (ECONNREFUSED ::1:9222 on the
// runner, MEM115 / M042 S09).
const CDP_ENDPOINT =
  process.env.POLY_CDP_ENDPOINT ||
  `http://127.0.0.1:${process.env.POLY_CDP_PORT || '9222'}`;

const HERE = path.dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = path.resolve(HERE, '..', '..', '..');
const VALIDATOR = path.join(REPO_ROOT, 'tests', 'cubase', 'validate_smf_export.py');

const ATTACH_TIMEOUT_MS = 30_000;
const CONNECT_TIMEOUT_MS = 30_000;
const CONNECT_RETRY_MS = 1_000;
// The sink file appears synchronously on the export handler's UI-thread call,
// but the CDP action -> UI-thread dispatch is async, so poll briefly for it.
const SINK_TIMEOUT_MS = 10_000;
const SINK_POLL_MS = 250;

/** Connect over CDP with retry — WebView2 opens its port only after the editor
 * renders, so the first connect can be refused. Mirrors toggle-step.spec.ts. */
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
      `window is not open. Last error: ${lastErr instanceof Error ? lastErr.message : lastErr}`,
  );
}

/** Attach and locate the page whose DOM is Poly's editor. */
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
      `${ATTACH_TIMEOUT_MS}ms. Likely cause: Cubase not launched with -EnableCdp.`,
  );
}

/** Wait until the export sink file exists and is non-empty. */
async function waitForSink(): Promise<void> {
  const deadline = Date.now() + SINK_TIMEOUT_MS;
  while (Date.now() < deadline) {
    if (existsSync(EXPORT_SINK) && statSync(EXPORT_SINK).size > 0) return;
    await new Promise((r) => setTimeout(r, SINK_POLL_MS));
  }
  throw new Error(
    `Export sink ${EXPORT_SINK} was not written within ${SINK_TIMEOUT_MS}ms after ` +
      `clicking Export. Likely cause: POLY_EXPORT_SINK not inherited by Cubase ` +
      `(it must be exported BEFORE Cubase launches), or the plugin build predates ` +
      `the sink hook.`,
  );
}

/** Run the independent-parser validator on the sink file. Throws (with the
 * validator's stdout) if the M032 contract is not satisfied. */
function validateSink(extraArgs: string[]): void {
  const args = [VALIDATOR, EXPORT_SINK, '--expected-bpm', '120', ...extraArgs];
  // stdio inherit so the validator's [smf-validate] report lands in the job log.
  execFileSync('python', args, { stdio: 'inherit', cwd: REPO_ROOT });
}

test.describe('L4-web: in-DAW MIDI export produces a valid Format-1 SMF', () => {
  test.describe.configure({ mode: 'serial' });

  test('export chip and per-lane export write validatable SMF', async () => {
    const browser = await connectWithRetry();
    try {
      const page = await findPolyEditor(browser);

      // --- All-lanes export via the global Export chip -----------------------
      // Clear any stale sink so waitForSink can't pass on a previous file.
      if (existsSync(EXPORT_SINK)) rmSync(EXPORT_SINK);

      const exportBtn = page.locator('#exportBtn');
      await expect(exportBtn).toBeVisible();
      await exportBtn.click();

      await waitForSink();
      // All active lanes -> multiple named GM tracks; the default 4-bar preset
      // renders well over 16 note-ons, so 16 is a safe no-loss floor.
      //
      // Name the expected tracks explicitly rather than letting the validator
      // fall back to its preset-0 default: this fixture is NOT preset 0, and
      // the export names tracks from the engine GM vocabulary, not the WebUI
      // display names. See EXPORT_ALL_LANE_NAMES for the derivation.
      validateSink([
        '--min-note-ons',
        '16',
        ...EXPORT_ALL_LANE_NAMES.flatMap((n) => ['--lane-name', n]),
      ]);

      // --- Single-lane export via the per-lane handle ------------------------
      rmSync(EXPORT_SINK);
      const laneStrip = page.locator('.strip').nth(EXPORT_LANE_INDEX);
      const lex = laneStrip.locator('[data-lex]');
      await expect(lex).toBeVisible();
      await lex.click();

      await waitForSink();
      // Exactly one named track for the chosen lane, no loss.
      validateSink([
        '--expected-named-tracks',
        '1',
        '--min-note-ons',
        '4',
        '--lane-name',
        EXPORT_LANE_NAME,
      ]);
    } finally {
      await browser.close();
    }
  });
});
