---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add GitHub Actions deploy workflow

Create deploy-site.yml workflow triggered on push to main, building the Astro site and deploying to GitHub Pages

## Inputs

- `site/astro.config.mjs`

## Expected Output

- `.github/workflows/deploy-site.yml`

## Verification

Workflow file has correct triggers, build steps, and deploy configuration
