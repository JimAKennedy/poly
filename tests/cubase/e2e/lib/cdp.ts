import { execFileSync } from 'node:child_process';
import { readFileSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

import { chromium } from '@playwright/test';
import type { Browser, Page } from '@playwright/test';

// M004 — what the session specs share: attaching to Poly's editor over CDP,
// asking the native side for its state, and driving the host through the
// MIDI Remote surface.
//
// The attach is the same as toggle-step.spec.ts's, including its two
// runner-confirmed facts: 127.0.0.1 rather than localhost, because WebView2's
// listener binds IPv4 only and Node resolves localhost to ::1 first; and a
// retried connect, because the port opens only after the editor renders.
//
// A third fact belongs here, found building S04 on the runner (2026-10-07):
// closing Poly's editor removes the CDP endpoint entirely -- WebView2's browser
// process goes with the window -- and reopening it brings up a NEW endpoint
// with a new page. A Browser or Page handle never survives an editor cycle.

const HERE = path.dirname(fileURLToPath(import.meta.url));
export const REPO_ROOT = path.resolve(HERE, '..', '..', '..', '..');
const DRIVER_DIR = path.join(REPO_ROOT, 'tests', 'cubase', 'driver');

export const CDP_ENDPOINT =
  process.env.POLY_CDP_ENDPOINT ||
  `http://127.0.0.1:${process.env.POLY_CDP_PORT || '9222'}`;

const CONNECT_TIMEOUT_MS = 30_000;
const ATTACH_TIMEOUT_MS = 30_000;
const STATE_TIMEOUT_MS = 10_000;
const RETRY_MS = 500;

const sleep = (ms: number) => new Promise((r) => setTimeout(r, ms));

/** Connect over CDP, retrying while the editor's endpoint comes up. */
export async function connect(timeoutMs = CONNECT_TIMEOUT_MS): Promise<Browser> {
  const deadline = Date.now() + timeoutMs;
  let lastErr: unknown;
  while (Date.now() < deadline) {
    try {
      return await chromium.connectOverCDP(CDP_ENDPOINT);
    } catch (err) {
      lastErr = err;
      await sleep(RETRY_MS);
    }
  }
  throw new Error(
    `Could not connect over CDP at ${CDP_ENDPOINT} within ${timeoutMs}ms -- ` +
      `is Poly's editor open in a Cubase launched with -EnableCdp? ` +
      `Last error: ${lastErr instanceof Error ? lastErr.message : lastErr}`,
  );
}

/** Whether anything is listening on the CDP endpoint right now. */
export async function endpointUp(): Promise<boolean> {
  try {
    const res = await fetch(`${CDP_ENDPOINT}/json/version`);
    return res.ok;
  } catch {
    return false;
  }
}

/** Wait until the endpoint is up (`want` true) or gone (`want` false). */
export async function waitForEndpoint(want: boolean, timeoutMs = 20_000): Promise<void> {
  const deadline = Date.now() + timeoutMs;
  while (Date.now() < deadline) {
    if ((await endpointUp()) === want) return;
    await sleep(RETRY_MS);
  }
  throw new Error(`CDP endpoint did not go ${want ? 'up' : 'down'} within ${timeoutMs}ms`);
}

/**
 * Every page that is a Poly editor, in target order. One per open editor: a
 * project with two Poly instances and both editors open lists two.
 */
export async function polyEditors(browser: Browser, expected = 1): Promise<Page[]> {
  const deadline = Date.now() + ATTACH_TIMEOUT_MS;
  let found: Page[] = [];
  while (Date.now() < deadline) {
    found = [];
    for (const context of browser.contexts()) {
      for (const page of context.pages()) {
        const isPoly = await page
          .locator('.strip')
          .first()
          .isVisible()
          .catch(() => false);
        if (isPoly) found.push(page);
      }
    }
    if (found.length >= expected) return found;
    await sleep(RETRY_MS);
  }
  throw new Error(
    `Found ${found.length} Poly editor page(s) over CDP, expected ${expected}, ` +
      `within ${ATTACH_TIMEOUT_MS}ms`,
  );
}

/** The single Poly editor. */
export async function polyEditor(browser: Browser): Promise<Page> {
  return (await polyEditors(browser, 1))[0];
}

/** The parts of the native `state` push the M004 specs read (host-iface.js). */
export interface PolyLaneState {
  name: string;
  timeline: boolean;
  fixed: number[] | null;
  pattern: number[];
  mt: number[];
}
export interface PolyState {
  preset: string;
  seed: number;
  lanes: PolyLaneState[];
}

/**
 * One bridge round trip: send `ready` and resolve with the `state` the native
 * side pushes back (bridge-schema.md: `ready` -> push initial `state`).
 *
 * This is the bridge's own request/response, not a DOM read, so a page whose
 * native side has gone -- a stale handle after an editor cycle -- times out
 * here rather than returning whatever the DOM last showed. It changes nothing:
 * `ready` only asks for a snapshot.
 */
export async function requestState(page: Page, timeoutMs = STATE_TIMEOUT_MS): Promise<PolyState> {
  const state = await page.evaluate(
    ({ timeoutMs }) =>
      new Promise<unknown>((resolve, reject) => {
        const w = window as unknown as {
          polyHostPush: (json: string) => void;
          polyHostCall: (json: string) => void;
          POLY_SCHEMA_VERSION: number;
        };
        const original = w.polyHostPush;
        const timer = setTimeout(() => {
          w.polyHostPush = original;
          reject(new Error(`no state push within ${timeoutMs}ms of a ready request`));
        }, timeoutMs);
        w.polyHostPush = (json: string) => {
          original(json);
          const msg = typeof json === 'string' ? JSON.parse(json) : json;
          if (msg && msg.type === 'state') {
            clearTimeout(timer);
            w.polyHostPush = original;
            resolve(msg.state);
          }
        };
        w.polyHostCall(JSON.stringify({ type: 'ready', v: w.POLY_SCHEMA_VERSION }));
      }),
    { timeoutMs },
  );
  return state as PolyState;
}

/** Send a bridge `action` (bridge-schema.md's action reference). */
export async function sendAction(page: Page, name: string, payload: object): Promise<void> {
  await page.evaluate(
    ({ name, payload }) => {
      const w = window as unknown as {
        polyHostCall: (json: string) => void;
        POLY_SCHEMA_VERSION: number;
      };
      w.polyHostCall(JSON.stringify({ type: 'action', v: w.POLY_SCHEMA_VERSION, name, payload }));
    },
    { name, payload },
  );
}

/**
 * Send a bridge `edit` as one complete gesture -- begin, perform, end -- which
 * is what the editor's own switches send (ui.js, the timeline toggle).
 */
export async function sendEdit(page: Page, paramId: string, value: number): Promise<void> {
  await page.evaluate(
    ({ paramId, value }) => {
      const w = window as unknown as {
        polyHostCall: (json: string) => void;
        POLY_SCHEMA_VERSION: number;
      };
      for (const gesture of ['begin', 'perform', 'end']) {
        w.polyHostCall(
          JSON.stringify({ type: 'edit', v: w.POLY_SCHEMA_VERSION, paramId, value, gesture }),
        );
      }
    },
    { paramId, value },
  );
}

/** Run tests/cubase/driver/remote.py with `args`; throws on a non-zero exit. */
export function remote(...args: string[]): void {
  execFileSync('python', [path.join(DRIVER_DIR, 'remote.py'), ...args], {
    stdio: 'inherit',
    cwd: REPO_ROOT,
  });
}

/** Run tests/cubase/driver/play_scenario.py with `args`. */
export function playScenario(...args: string[]): void {
  execFileSync('python', [path.join(DRIVER_DIR, 'play_scenario.py'), ...args], {
    stdio: 'inherit',
    cwd: REPO_ROOT,
  });
}

/**
 * Read a probe capture once it has stopped changing.
 *
 * poly_midi_probe rewrites its whole JSONL from inside process() whenever it
 * has new events, truncating first. A read that lands mid-rewrite sees an
 * empty or partial file -- observed on the runner (2026-10-07) right after an
 * offline export, where Cubase closes the export dialog while the last blocks
 * are still being written. So the file is read until two reads `quietMs`
 * apart return the same non-empty text.
 */
export async function readSettledProbe(file: string, quietMs = 1_500, timeoutMs = 30_000): Promise<string> {
  const deadline = Date.now() + timeoutMs;
  let last = '';
  while (Date.now() < deadline) {
    let text = '';
    try {
      text = readFileSync(file, 'utf-8');
    } catch {
      // not written yet
    }
    if (text.length > 0 && text === last) return text;
    last = text;
    await sleep(quietMs);
  }
  throw new Error(`probe capture ${file} did not settle within ${timeoutMs}ms`);
}

/** This session's directory, published by scripts/cubase/start-session.ps1. */
export function sessionDir(): string {
  const dir = process.env.POLY_SESSION_DIR;
  if (!dir) {
    throw new Error(
      'POLY_SESSION_DIR is not set -- run this spec in a session started by ' +
        'scripts/cubase/start-session.ps1',
    );
  }
  return dir;
}
