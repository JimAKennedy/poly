import { defineConfig } from '@playwright/test';

// M042 S09 — Playwright config for the CDP-attach L4-web e2e.
//
// Unlike site/playwright.config.ts (which launches its own Chromium against the
// Astro dev server), this project ATTACHES over CDP to the WebView2 running
// inside Cubase (chromium.connectOverCDP in toggle-step.spec.ts). There is no
// webServer and no browser to launch here — the endpoint is a live Cubase.
//
// The pure-helper unit tests (lib/*.test.ts) run under the same runner; they
// need no browser at all.

export default defineConfig({
  testDir: '.',
  // Match toggle-step.spec.ts and the helper unit tests.
  testMatch: ['**/*.spec.ts', '**/lib/*.test.ts'],
  timeout: 120_000,
  // Single self-hosted runner, single Cubase/CDP session — never parallelize.
  fullyParallel: false,
  workers: 1,
  reporter: 'list',
  use: {
    // Playwright traces + screenshots on failure so a runner-side CDP or
    // selector failure is diagnosable from the uploaded artifacts.
    trace: 'retain-on-failure',
    screenshot: 'only-on-failure',
  },
});
