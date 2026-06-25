# S05: GitHub Pages Deployment

**Goal:** Set up automatic GitHub Pages deployment via GitHub Actions
**Demo:** Push to main triggers GitHub Actions build and deploys the site to GitHub Pages

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Add GitHub Actions deploy workflow** `est:30m`
  Create deploy-site.yml workflow triggered on push to main, building the Astro site and deploying to GitHub Pages
  - Files: `.github/workflows/deploy-site.yml`
  - Verify: Workflow file has correct triggers, build steps, and deploy configuration

## Files Likely Touched

- .github/workflows/deploy-site.yml
