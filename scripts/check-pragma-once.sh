#!/bin/bash
ERRORS=0
for f in $(find . -name '*.h' -o -name '*.hpp' | grep -v build | grep -v _deps | grep -v node_modules); do
  if ! grep -q '#pragma once' "$f"; then
    echo "Missing #pragma once: $f"
    ERRORS=$((ERRORS + 1))
  fi
done
exit $ERRORS
