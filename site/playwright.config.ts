import { defineConfig, devices } from '@playwright/test';

export default defineConfig({
  testDir: './tests-e2e',
  timeout: 60_000,
  fullyParallel: false,
  reporter: 'list',
  use: {
    baseURL: 'http://localhost:4321',
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
