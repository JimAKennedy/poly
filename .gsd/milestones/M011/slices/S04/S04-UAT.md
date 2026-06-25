# S04: Documentation and recurrence prevention — UAT

**Milestone:** M011
**Written:** 2026-06-23T17:29:57.690Z

## S04: Documentation and Recurrence Prevention — UAT\n\n### Pre-push Quality Gate\n- [ ] Clone fresh, run `pre-commit install -t pre-push`\n- [ ] Attempt `git push` to main — should be blocked with instructions to use PR\n- [ ] Attempt push to feature branch — should run format, RT safety, build+test\n\n### Documentation\n- [ ] KNOWLEDGE.md contains R6 (ownership-transfer), R7 (MSVC includes), R8 (clang-format pin), R9 (NFR rationale)\n- [ ] CLAUDE.md contains MSVC Portability, Ownership Transfer Annotations, Pre-Push Quality Gate sections\n\n### CI\n- [ ] PR #16 shows all CI checks green (code-quality, builds, engine-isolation, NFR review, secrets-scan, coverage)
