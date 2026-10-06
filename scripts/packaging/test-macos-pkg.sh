#!/usr/bin/env bash
# test-macos-pkg.sh — install Poly's macOS package on a clean Mac, both ways,
# and prove removal leaves the plug-in folders as they were
# (open-source-launch M007/S01, OS35).
#
#   bash scripts/packaging/test-macos-pkg.sh <poly.pkg>
#
# For each install domain — the whole machine (/Library/…, needs sudo) and the
# current user (~/Library/…) — it snapshots the VST3 folder, installs, checks
# the bundle landed there and the receipt says so, removes exactly the paths
# the receipt lists, forgets the receipt, and checks the folder listing equals
# the snapshot. Removal is what a user does: delete poly_plugin.vst3.
#
# This installs into the real plug-in folders, so it runs only on a clean
# machine — a fresh CI runner, or anywhere no Poly bundle is installed. On a
# developer's Mac, where build.sh deploys a copy to ~/Library, it refuses
# rather than overwrite and then delete that copy.
set -euo pipefail

PKG="${1:-}"
ID="digital.jk.poly.vst3"
BUNDLE="poly_plugin.vst3"
SYSTEM_DIR="/Library/Audio/Plug-Ins/VST3"
USER_DIR="$HOME/Library/Audio/Plug-Ins/VST3"

die() { echo "test-macos-pkg: FAIL: $*" >&2; exit 1; }
say() { echo "test-macos-pkg: $*"; }

[ -n "$PKG" ] && [ -f "$PKG" ] || { echo "test-macos-pkg: usage: <poly.pkg> (no package at '${PKG}')" >&2; exit 2; }

for dir in "$SYSTEM_DIR" "$USER_DIR"; do
  if [ -e "$dir/$BUNDLE" ]; then
    echo "test-macos-pkg: refusing to run: $dir/$BUNDLE already exists, so this is not a clean machine;" >&2
    echo "test-macos-pkg: installing and removing would overwrite and then delete that copy" >&2
    exit 2
  fi
done

snapshot() { ls -A "$1" | sort; }

# remove_installed <volume> <sudo-or-empty>: delete exactly what the receipt
# lists — files first, then the bundle's directories deepest first.
remove_installed() {
  local volume="$1" sudo="$2" location
  location="$(pkgutil --volume "$volume" --pkg-info "$ID" | sed -n 's/^location: //p')"
  [ -n "$location" ] || die "no install location in the receipt on $volume"
  local base="${volume%/}/${location}"
  while IFS= read -r f; do
    [ -n "$f" ] && $sudo rm -f "$base/$f"
  done < <(pkgutil --volume "$volume" --only-files --files "$ID")
  while IFS= read -r d; do
    [ -n "$d" ] && $sudo rmdir "$base/$d"
  done < <(pkgutil --volume "$volume" --only-dirs --files "$ID" | sort -r)
  $sudo pkgutil --volume "$volume" --forget "$ID" >/dev/null
}

check_domain() {
  local name="$1" dir="$2" volume="$3" target="$4" sudo="$5"
  say "== $name: $dir"
  if [ ! -d "$dir" ]; then
    $sudo mkdir -p "$dir"
    say "the folder did not exist; created it empty, as a DAW-less Mac has it"
  fi
  local before after
  before="$(snapshot "$dir")"
  say "before: $(printf '%s' "$before" | grep -c . || true) entries"

  $sudo installer -pkg "$PKG" -target "$target" >/dev/null
  [ -f "$dir/$BUNDLE/Contents/Info.plist" ] || die "$name: no $BUNDLE in $dir after installing"
  pkgutil --volume "$volume" --pkg-info "$ID" | grep -q '^location: Library/Audio/Plug-Ins/VST3$' \
    || die "$name: the receipt does not name Library/Audio/Plug-Ins/VST3"
  say "installed: $dir/$BUNDLE, receipt $ID on $volume"

  remove_installed "$volume" "$sudo"
  [ ! -e "$dir/$BUNDLE" ] || die "$name: $BUNDLE is still in $dir after removal"
  pkgutil --volume "$volume" --pkg-info "$ID" >/dev/null 2>&1 && die "$name: the receipt survived --forget"
  after="$(snapshot "$dir")"
  [ "$before" = "$after" ] || die "$name: the folder differs after removal (before: [$before] after: [$after])"
  say "removed: the folder is as it was"
}

check_domain "all users" "$SYSTEM_DIR" "/" "/" "sudo"
check_domain "only me" "$USER_DIR" "$HOME" "CurrentUserHomeDirectory" ""
say "PASS — installs and removes cleanly, system-wide and per-user"
