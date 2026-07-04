import { defineConfig } from '@playwright/test';

const isCI = !!process.env.CI;

export default defineConfig({
  testDir: './tests',
  timeout: 30000,
  retries: isCI ? 2 : 0,
  reporter: isCI
    ? [['html', { open: 'never' }], ['list']]
    : [['list']],
  use: {
    viewport: { width: 1280, height: 840 },
    trace: isCI ? 'on-first-retry' : 'off',
    screenshot: isCI ? 'only-on-failure' : 'off',
    launchOptions: process.env.PLAYWRIGHT_EXECUTABLE
      ? { executablePath: process.env.PLAYWRIGHT_EXECUTABLE }
      : {},
  },
});
