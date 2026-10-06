#!/usr/bin/env bash
# build-macos-pkg.sh — build Poly's macOS installer package
# (open-source-launch M007/S01, OS35).
#
#   bash scripts/packaging/build-macos-pkg.sh <poly_plugin.vst3> <version> <out.pkg>
#
# Wraps the bundle in a component package that installs to
# /Library/Audio/Plug-Ins/VST3 (or ~/Library/... when the user chooses "only
# me"), marked not relocatable so the installer never "updates" a stray copy
# elsewhere on disk, then into a product archive whose distribution offers the
# two domains. Unsigned: signing, notarizing and stapling are M007/S02's.
# The VST3 alone ships — M003/S03 declined the AU for the first release.
set -euo pipefail

BUNDLE="${1:-}"
VERSION="${2:-}"
OUT="${3:-}"
IDENTIFIER="digital.jk.poly.vst3"
INSTALL_LOCATION="/Library/Audio/Plug-Ins/VST3"
HERE="$(cd "$(dirname "$0")" && pwd)"

die() { echo "build-macos-pkg: $*" >&2; exit 2; }

[ -n "$BUNDLE" ] && [ -d "$BUNDLE" ] || die "no VST3 bundle at '${BUNDLE}' (usage: <poly_plugin.vst3> <version> <out.pkg>)"
[ -n "$VERSION" ] || die "no version given (usage: <poly_plugin.vst3> <version> <out.pkg>)"
[ -n "$OUT" ] || die "no output path given (usage: <poly_plugin.vst3> <version> <out.pkg>)"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

mkdir -p "$WORK/root"
# Stage without resource forks, removable extended attributes or quarantine:
# pkgbuild packs attributes into the payload, so a quarantine flag on the built
# bundle would otherwise ship to every user. (com.apple.provenance survives —
# macOS stamps it on every file a process writes and nothing can remove it; it
# is not quarantine.)
ditto --norsrc --noextattr --noqtn "$BUNDLE" "$WORK/root/poly_plugin.vst3"

pkgbuild --analyze --root "$WORK/root" "$WORK/component.plist" >/dev/null
# Every bundle pkgbuild finds is relocatable by default; turn that off for all.
COUNT="$(plutil -extract . raw -o - "$WORK/component.plist" 2>/dev/null || echo 0)"
for ((i = 0; i < COUNT; i++)); do
  plutil -replace "$i.BundleIsRelocatable" -bool NO "$WORK/component.plist"
done

mkdir -p "$WORK/packages"
pkgbuild --root "$WORK/root" \
  --component-plist "$WORK/component.plist" \
  --identifier "$IDENTIFIER" \
  --version "$VERSION" \
  --install-location "$INSTALL_LOCATION" \
  "$WORK/packages/poly-vst3.pkg" >/dev/null

sed "s/@VERSION@/${VERSION}/" "$HERE/distribution.xml" > "$WORK/distribution.xml"
productbuild --distribution "$WORK/distribution.xml" \
  --package-path "$WORK/packages" \
  "$OUT" >/dev/null

echo "built $OUT ($IDENTIFIER $VERSION, installs to $INSTALL_LOCATION or the user's own)"
