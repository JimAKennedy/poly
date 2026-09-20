---
class: gated
---

# Installers — vision

**Input document for `/jk:assess`.** Not a plan: a statement of the outcome, the
current state measured rather than assumed, and the decisions a plan will have
to take. The milestone decomposition below is a proposal to argue with.

## The observation

Poly's release pipeline is further along than "we need installers" suggests, and
further from shipping than the pipeline's existence suggests.

`.github/workflows/release.yml` already runs on a version tag, builds
macOS-universal and Windows-x64, gates each behind the VST3 validator and
pluginval at strictness 8, and publishes a GitHub Release whose body is
generated from `CHANGELOG.md`. `scripts/check-release-workflow.mjs` locks that
shape in 27 contract assertions. None of that needs rebuilding.

What it produces is a **`.zip` containing a `.vst3` bundle**. `README.md` then
asks the user to unzip it and hand-copy the bundle into a platform-specific
folder they are expected to know about — and, on macOS, to run

```
xattr -dr com.apple.quarantine poly_plugin.vst3
```

before it will load at all. That last instruction is the tell. We are asking a
musician to use the terminal to defeat a security mechanism, because the
alternative is a plugin their DAW silently refuses to scan.

**Three facts the assessment must not gloss.**

**No release has ever been cut.** The repository carries one tag,
`v0.1.0-doccov-baseline`, and `gh release list` is empty. The pipeline is
untested against reality: it has never produced an artifact a stranger has
downloaded. Any plan that assumes the release path works because it exists is
assuming.

**The macOS signing steps are gated on secrets that are not provisioned.** The
codesign, notarize and staple steps each carry `if: env.MACOS_… != ''`, so on a
repository without those secrets they are skipped silently and the build still
succeeds. The pipeline does not currently distinguish "signed" from "we quietly
did not sign" — and `README.md` says so plainly, which is to its credit.

**Windows has no signing at all.** There is no `signtool` step, no certificate
secret, no gate. A Windows user downloading an unsigned installer meets
SmartScreen's "Windows protected your PC" dialog, whose default action is
*Don't run*.

## Why an installer without signing is worse than a zip

This is the observation that should shape the programme, and it inverts the
obvious ordering.

An unsigned `.zip` is inert. The user unzips it, copies a folder, and the only
security prompt is Gatekeeper on first load — which the README teaches them to
clear. Annoying, but the trust cost is paid once and it is visibly a file copy.

An unsigned **installer** asks for administrator rights to write into a shared
system directory, from a binary the OS cannot attribute to anyone. macOS refuses
to open an unsigned `.pkg` at all under default settings. Windows SmartScreen
presents it as a probable threat. The user's rational response to an unsigned
installer is to not run it, and they would be right.

So **signing is not a polish step that follows the installers. It is the thing
that makes installers worth building**, and the programme should be ordered that
way even though signing is the part with a purchase order attached.

## Vision

A musician who wants Poly goes to the GitHub Releases page, downloads one file
for their platform, opens it, and answers the questions their operating system
asks — no terminal, no folder conventions, no quarantine flags. When it is done
their DAW finds Poly on the next scan. If they later want it gone, the same
mechanism removes it.

## What success looks like

- One artifact per platform on the Releases page, named so a non-expert can tell
  which is theirs.
- The macOS artifact is signed with a Developer ID, notarized by Apple, and
  stapled, so it opens with no warning and no `xattr`.
- The Windows artifact is signed, so SmartScreen attributes it rather than
  warning against it.
- Installing places the plugin where the DAW looks, for every format Poly
  ships — including the AU, which CI builds today and no release contains.
- The README's install section becomes three sentences and a link, and the
  Gatekeeper workaround section is deleted rather than reworded.
- A release that is **not** signed fails loudly rather than shipping quietly.

## What this is not

- **Not a Linux installer.** Decision D029 stands: Poly is engine/WASM-only on
  Linux and ships no plugin binary there. An installer programme does not
  reopen that.
- **Not an auto-updater.** Download-and-run is the whole scope. In-plugin update
  checks are a different programme with a privacy surface of their own.
- **Not a store submission.** No Apple App Store, no package managers. The
  Releases page is the distribution channel.

## Proposed milestones

### IN1 — A release exists, and its honesty is enforced

**Why first, and why it is not the installers.** The pipeline has never run to
completion against a real tag. Building installers on top of an untested release
path means debugging two new things at once, and the cheapest way to find out
what the release actually produces is to cut one.

This milestone cuts a real pre-release from the current pipeline, downloads what
it produced on both platforms, and installs it by hand on a clean machine —
recording what a stranger would actually experience, including every warning
dialog, verbatim.

It then closes the silent-skip hole: the signing steps' `if:` gates make an
unsigned build indistinguishable from a signed one in the workflow's own output.
A release should state which it is, and an unsigned artifact should not be
publishable without that being a deliberate, recorded choice.

**The open question this milestone must answer, not assume:** whether
`v0.1.0-doccov-baseline` implies a versioning scheme anyone has agreed to, and
what the first real version number is.

### IN2 — The macOS artifact installs without a terminal

**Depends on IN1.**

A signed, notarized `.pkg` that places the VST3 in
`/Library/Audio/Plug-Ins/VST3/` and the AU component in
`/Library/Audio/Plug-Ins/Components/`, with a per-user option.

**The AU is the scope question.** `ci.yml` has a `build-au-macos` job, so the
component is built on every PR and shipped in nothing. Either the installer
carries it — in which case the release pipeline must build it too, which it
currently does not — or the programme records deliberately that Poly ships VST3
only and the AU job exists for validation. Both are defensible; drifting into
one by accident is not.

**Cost that belongs in the open:** an Apple Developer Program membership, and a
notarization round trip on every release that can take minutes and can fail for
reasons unrelated to the build.

### IN3 — The Windows artifact installs without a warning

**Depends on IN1. Independent of IN2.**

An installer that places the VST3 in `C:\Program Files\Common Files\VST3\`,
signed so SmartScreen attributes it.

**Signing is the whole difficulty.** An OV certificate still accumulates
SmartScreen reputation over time, so early downloads may warn anyway; an EV
certificate carries immediate reputation at a higher price and usually a
hardware token, which a GitHub-hosted runner cannot hold without a cloud signing
service. **This is a purchasing decision before it is an engineering one**, and
the assessment should surface the options with prices rather than picking one.

The installer format is a genuine choice — WiX/MSI, Inno Setup, NSIS — and it
should be made on which one signs and uninstalls cleanly in CI, not on taste.

### IN4 — The instructions match the artifacts

**Depends on IN2 and IN3.**

`README.md`'s install section rewritten around the installers, the Gatekeeper
workaround **deleted** rather than softened, and the site's guide pointed at the
Releases page. Small, and worth its own slice because documentation that still
describes the zip is worse than no documentation once an installer exists.

## Sequencing

IN1 blocks everything: the rest is built on a release path nobody has exercised.
IN2 and IN3 are independent of each other and can run in either order or in
parallel — different platforms, different certificates, different installer
tooling, no shared code. IN4 depends on both, because it describes what they
produce.

**The critical path is not code.** It is obtaining two certificates. If that
lands first, IN2 and IN3 are a week of packaging work; if it does not, they
cannot be finished at all — only prepared. A plan should therefore separate
"the installer is built and tested unsigned" from "the installer is signed and
published", so the engineering can complete while the paperwork is pending
rather than blocking on it.

## Out of scope, recorded so a later pass does not rediscover them

- **Linux packaging** — D029.
- **Auto-update** — separate programme.
- **Store or package-manager distribution** — Homebrew, winget, App Store.
- **Installing presets or content** — the plugin carries its 45 factory presets
  in the binary; there is no separate content payload to install.
- **The WASM/site build** — deployed by `deploy-site.yml`, unrelated to plugin
  distribution.
