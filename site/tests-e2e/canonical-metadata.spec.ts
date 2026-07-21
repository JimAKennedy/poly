import { test, expect } from '@playwright/test';

// M047 S05 T03 — canonical + OG metadata host regression guard.
//
// D7 from the 2026-07-16 product review flagged that canonical URLs and
// OG/Twitter image URLs pointed at jimakennedy.github.io while branding
// says poly.jk.digital. astro.config.mjs now sets site to
// https://poly.jk.digital; Starlight derives the canonical link from
// site + base, and the OG/Twitter image URLs are set in the head config.
//
// This spec asserts the deployed pages carry the branded host on every
// SEO-visible meta tag and NEVER carry the old GitHub Pages host. Any
// future config regression that reintroduces jimakennedy.github.io in
// a head tag fails this spec.
//
// Runs against Playwright's baseURL — http://localhost:4321 for local
// preview or POLY_SITE_URL for remote verify. Both cases render the
// same absolute URLs into the head because Astro bakes site+base into
// canonical/OG at build time.

const CANONICAL_HOST = 'https://poly.jk.digital';
const OLD_HOST = 'jimakennedy.github.io';

// Two representative pages: the landing card and one chapter. Chapter
// pages exercise the general MDX pipeline; the landing page is a
// splash template with different meta-tag emission paths.
const PAGES = ['/poly/', '/poly/03-afro-cuban/'];

for (const pagePath of PAGES) {
  test(`canonical + OG metadata on ${pagePath} points at poly.jk.digital`, async ({ page }) => {
    await page.goto(pagePath);

    const canonicalHref = await page
      .locator('link[rel="canonical"]')
      .getAttribute('href');
    expect(canonicalHref, `canonical href for ${pagePath}`).toBeTruthy();
    expect(canonicalHref!.startsWith(CANONICAL_HOST)).toBe(true);
    expect(canonicalHref!.includes(OLD_HOST)).toBe(false);

    const ogImage = await page
      .locator('meta[property="og:image"]')
      .getAttribute('content');
    expect(ogImage, `og:image content for ${pagePath}`).toBeTruthy();
    expect(ogImage!.startsWith(CANONICAL_HOST)).toBe(true);

    const twitterImage = await page
      .locator('meta[name="twitter:image"]')
      .getAttribute('content');
    expect(twitterImage, `twitter:image content for ${pagePath}`).toBeTruthy();
    expect(twitterImage!.startsWith(CANONICAL_HOST)).toBe(true);

    // Belt-and-braces: no head tag anywhere may reference the old host.
    // Query the raw head HTML because the meta tags checked above cover
    // canonical/og:image/twitter:image but not og:url, alternate links,
    // or any future tag someone adds — this guard catches all of them.
    const headHtml = await page.locator('head').innerHTML();
    expect(headHtml.includes(OLD_HOST)).toBe(false);
  });
}
