import { defineConfig, devices } from '@playwright/test';

// POLY_SITE_URL overrides baseURL so the same spec runs against local preview
// (default http://localhost:4321) and any deployed URL passed by the
// site-verify-remote.sh script (e.g. https://user.github.io/poly/).
const baseURL = process.env.POLY_SITE_URL || 'http://localhost:4321';

export default defineConfig({
  testDir: './tests-e2e',
  timeout: 60_000,
  fullyParallel: false,
  reporter: 'list',
  use: {
    baseURL,
    trace: 'off',
    // Chromium normally requires a user gesture before AudioContext can start.
    // Playwright's page.click() counts as one, but the flag below removes the
    // policy entirely so a headless run cannot deadlock on it.
    launchOptions: {
      args: ['--autoplay-policy=no-user-gesture-required'],
    },
  },
  projects: [
    {
      name: 'chromium',
      use: { ...devices['Desktop Chrome'] },
    },
  ],
  // Astro dev is started in background mode (see site/CLAUDE.md) and detaches
  // immediately, which is incompatible with Playwright's webServer "wait until
  // command exits or url comes up" semantics. The test runner assumes the dev
  // server is already up (start it with `npx astro dev` before invoking
  // `npm run test:e2e`, or leave it running via the site's usual workflow).
});
