---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T04: Final end-to-end verification push

Push all changes to main, verify CI passes all checks (code-quality, all platform builds, engine-isolation, secrets-scan, coverage). If any check fails, fix and re-push. This is the acceptance test for the entire milestone.

## Inputs

- `all modified files`

## Expected Output

- `green CI run on main`

## Verification

gh run list --limit 1 shows CI passing on main
