#!/usr/bin/env node
// check-release-verify-workflow.mjs — contract test for
// .github/workflows/release-verify.yml (open-source-launch M006/S01, OS33).
//
// The verifier is what a clean machine measures of a published Release: it
// downloads the tag's assets onto fresh hosted runners, verifies the checksums
// and the provenance attestation, installs the bundle where a DAW looks, records
// the verdicts that produce the platform's dialogs — the quarantine attribute
// and spctl on macOS, the Mark-of-the-Web and Authenticode status on Windows —
// runs pluginval against the installed bundle, and uploads a report. A verifier
// nothing checks is the gap this programme keeps finding, so this file locks
// its shape the way check-release-workflow.mjs locks release.yml's. No YAML
// parser: targeted structural matches. Run:
// `node --test scripts/check-release-verify-workflow.mjs` (wired into check-guards.sh).
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { existsSync, readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const REPO = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const WF_PATH = resolve(REPO, '.github', 'workflows', 'release-verify.yml');

test('release-verify.yml exists', () => {
  assert.ok(existsSync(WF_PATH), 'no .github/workflows/release-verify.yml');
});

const wf = existsSync(WF_PATH) ? readFileSync(WF_PATH, 'utf8') : '';
const job = (runner) => {
  const start = wf.indexOf(`runs-on: ${runner}`);
  assert.notEqual(start, -1, `no job runs on ${runner}`);
  const next = wf.indexOf('\n  ' , wf.indexOf('\n    steps:', start) + 1);
  const rest = wf.slice(start);
  const end = rest.search(/\n  [a-z][a-z0-9-]*:\n/);
  return end === -1 ? rest : rest.slice(0, end);
};

test('runs on dispatch with a required tag input, and on every published release', () => {
  assert.match(wf, /workflow_dispatch:\s*\n\s*inputs:\s*\n\s*tag:/, 'no workflow_dispatch tag input');
  assert.match(wf, /required:\s*true/, 'the tag input must be required');
  assert.match(wf, /release:\s*\n\s*types:\s*\[published\]/, 'must also run on release: published');
});

test('holds least privilege: top-level contents: read and nothing more', () => {
  assert.match(wf, /^permissions:\n  contents: read\n/m, 'top-level permissions must be exactly contents: read');
  assert.doesNotMatch(wf, /^\s+(id-token|attestations|issues|pull-requests|packages):\s*write/m, 'the verifier writes nothing');
});

test('one job per shipping platform, on fresh hosted runners', () => {
  assert.match(wf, /runs-on: macos-14/, 'no macos-14 job');
  assert.match(wf, /runs-on: windows-2022/, 'no windows-2022 job');
  assert.doesNotMatch(wf, /self-hosted/, 'the verifier must run on hosted runners, which are fresh VMs');
});

for (const [runner, zip, sum] of [
  ['macos-14', 'macos-universal', 'shasum -a 256'],
  ['windows-2022', 'windows-x64', 'sha256sum'],
]) {
  test(`${runner}: downloads the tag's ${zip} zip and SHA256SUMS, verifies the digest and the provenance attestation`, () => {
    const j = job(runner);
    assert.match(j, /gh release download/, 'assets must come from the Release, as a stranger gets them');
    assert.ok(j.includes(zip), `must download the ${zip} asset`);
    assert.ok(j.includes('SHA256SUMS'), 'must download SHA256SUMS');
    assert.ok(j.includes(sum), `must verify the digest with ${sum}`);
    assert.match(j, /gh attestation verify .*--owner JimAKennedy/, 'must verify the provenance attestation');
  });
}

test('macOS: installs into the user VST3 folder, applies the quarantine attribute a browser would, and records xattr, codesign and spctl verdicts', () => {
  const j = job('macos-14');
  assert.ok(j.includes('Library/Audio/Plug-Ins/VST3'), 'must install into ~/Library/Audio/Plug-Ins/VST3');
  assert.match(j, /xattr -w com\.apple\.quarantine/, 'must apply the quarantine attribute as a browser download would');
  assert.match(j, /xattr -l/, 'must record the attribute');
  assert.match(j, /codesign -dv/, 'must record the signature verdict');
  assert.match(j, /spctl --assess/, "must record Gatekeeper's verdict");
});

test('Windows: installs into the user VST3 folder and records the Mark-of-the-Web and Authenticode verdicts', () => {
  const j = job('windows-2022');
  assert.match(j, /Common\\VST3|Common Files\\VST3/, 'must install into a VST3 folder');
  assert.match(j, /Zone\.Identifier/, 'must record the Mark-of-the-Web stream');
  assert.match(j, /Get-AuthenticodeSignature/, 'must record the Authenticode status');
});

for (const runner of ['macos-14', 'windows-2022']) {
  test(`${runner}: runs pluginval 1.0.4, digest-verified, at strictness 8 against the installed bundle, and uploads a report`, () => {
    const j = job(runner);
    assert.match(j, /pluginval\/releases\/download\/v1\.0\.4\//, 'pluginval must be pinned to v1.0.4');
    assert.match(j, /[0-9a-f]{64}\s+pluginval\.zip/, "pluginval's digest must be verified before it runs");
    assert.match(j, /--strictness-level 8/, 'pluginval must run at strictness 8');
    assert.match(j, /report-[a-z0-9-]+\.md/, 'must write a per-platform report');
    assert.match(j, /actions\/upload-artifact@[0-9a-f]{40}/, 'must upload the report with the pinned action');
  });
}

test('measurements never fail the job; only download, digest, attestation and pluginval may', () => {
  // Every line that invokes a verdict command (not a heading that merely
  // names it inside an echo) must swallow its exit status.
  for (const cmd of ['spctl --assess', 'codesign -dv', 'Get-AuthenticodeSignature']) {
    const lines = wf.split('\n').filter((l) => l.includes(cmd) && !/echo\s+"/.test(l) && !l.trim().startsWith('#'));
    assert.ok(lines.length > 0, `${cmd} is never invoked`);
    for (const line of lines) {
      assert.match(line, /\|\| true|-ErrorAction SilentlyContinue|2>&1/, `${cmd} must be a measurement that cannot fail the job: ${line.trim()}`);
    }
  }
});
