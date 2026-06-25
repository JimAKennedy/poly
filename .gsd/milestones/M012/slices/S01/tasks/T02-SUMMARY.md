---
id: T02
parent: S01
milestone: M012
key_files:
  - site/src/styles/custom.css
  - site/package.json
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-25T00:36:39.871Z
blocker_discovered: false
---

# T02: Installed Fontsource variable fonts and created design system CSS with full colour palette and typography scale

**Installed Fontsource variable fonts and created design system CSS with full colour palette and typography scale**

## What Happened

Installed @fontsource-variable/source-serif-4, @fontsource-variable/inter, @fontsource-variable/jetbrains-mono. Created site/src/styles/custom.css with all Poly colour palette CSS variables, Starlight theme overrides (--sl-color-accent etc), typography scale matching the plan spec (serif for prose, sans for UI, mono for code), and a max-width constraint for two-column readability at wider viewports.

## Verification

Build succeeds with custom CSS loaded; browser verification confirms Source Serif 4 Variable is the computed font and --sl-color-accent is #01696f

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cd site && npm run build` | 0 | pass | 568ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `site/src/styles/custom.css`
- `site/package.json`
