# M013: Open-Source Readiness

**Vision:** Remediate NFR review findings so the repo can be safely made public: add community health files (LICENSE, README, SECURITY, etc.), scrub tracked operational files with PII risk, harden CI supply chain by SHA-pinning actions, and tune the NFR scanner config to exclude build artifacts and inapplicable rules.

## Success Criteria

- LICENSE, README.md, SECURITY.md, CODE_OF_CONDUCT.md, CONTRIBUTING.md present at repo root
- No operational files with PII risk (event-log, audit logs) tracked in git
- All GitHub Actions pinned to full SHA commits
- NFR review re-run shows zero high-severity findings on first-party code
- reports/ and site/dist/ excluded from git tracking and NFR scan

## Slices

- [ ] **S01: Community Health Files** `risk:low` `depends:[]`
  > After this: README renders on GitHub with build badges, license link, and project overview

- [ ] **S02: Privacy and Git Hygiene** `risk:medium` `depends:[]`
  > After this: git ls-files shows no event-log, audit, or reports files tracked; .gitignore covers all operational paths

- [ ] **S03: CI Supply-Chain Hardening** `risk:low` `depends:[]`
  > After this: CI run passes with all actions SHA-pinned; pre-commit lint step visible in CI output

- [ ] **S04: NFR Config Tuning** `risk:low` `depends:[S01,S02,S03]`
  > After this: NFR re-scan shows zero high-severity findings and reduced false-positive count

## Boundary Map

Not provided.
