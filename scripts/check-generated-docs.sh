#!/usr/bin/env bash
# Verifies that generated doc regions match what
# `node scripts/generate-param-docs.mjs` (M048 S05) and
# `node scripts/generate-bridge-schema-doc.mjs` (M048 S07) would produce right now.
#
# If site/src/generated/params.json is missing (fresh checkout), we run the
# site-side generator first to build it — same emitter, same schema.
#
# CI wiring: .github/workflows/ci.yml site-lint job.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

APPENDIX="site/src/content/docs/appendix-parameters.mdx"
ENGINE_SPEC="docs/engine-spec.md"
BRIDGE_DOC="webui/bridge-schema.md"
PARAMS_JSON="site/src/generated/params.json"

if [ ! -f "$PARAMS_JSON" ]; then
  echo "[check-generated-docs] $PARAMS_JSON missing — running generate-params-json.mjs"
  node site/scripts/generate-params-json.mjs
fi

# Save the tracked content, run the generators, then diff. Restore tracked
# content unconditionally so the check is idempotent.
TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

cp "$APPENDIX" "$TMPDIR/appendix.tracked"
cp "$ENGINE_SPEC" "$TMPDIR/engine-spec.tracked"
cp "$BRIDGE_DOC" "$TMPDIR/bridge-schema.tracked"

node scripts/generate-param-docs.mjs >/dev/null
node scripts/generate-bridge-schema-doc.mjs >/dev/null

FAIL=0
if ! diff -u "$TMPDIR/appendix.tracked" "$APPENDIX" >"$TMPDIR/appendix.diff"; then
  echo "::error file=$APPENDIX::generated content drifted — run 'node scripts/generate-param-docs.mjs' and commit"
  cat "$TMPDIR/appendix.diff"
  FAIL=1
fi
if ! diff -u "$TMPDIR/engine-spec.tracked" "$ENGINE_SPEC" >"$TMPDIR/engine-spec.diff"; then
  echo "::error file=$ENGINE_SPEC::generated content drifted — run 'node scripts/generate-param-docs.mjs' and commit"
  cat "$TMPDIR/engine-spec.diff"
  FAIL=1
fi
if ! diff -u "$TMPDIR/bridge-schema.tracked" "$BRIDGE_DOC" >"$TMPDIR/bridge-schema.diff"; then
  echo "::error file=$BRIDGE_DOC::generated content drifted — run 'node scripts/generate-bridge-schema-doc.mjs' and commit"
  cat "$TMPDIR/bridge-schema.diff"
  FAIL=1
fi

# Restore tracked content so this script leaves the working tree untouched.
cp "$TMPDIR/appendix.tracked" "$APPENDIX"
cp "$TMPDIR/engine-spec.tracked" "$ENGINE_SPEC"
cp "$TMPDIR/bridge-schema.tracked" "$BRIDGE_DOC"

if [ "$FAIL" -eq 1 ]; then
  echo ""
  echo "Generated doc regions are out of sync with their source of truth."
  echo "Run:  node scripts/generate-param-docs.mjs && node scripts/generate-bridge-schema-doc.mjs"
  echo "Then commit the updated files."
  exit 1
fi

echo "Generated doc regions are in sync (params + bridge schema)"
