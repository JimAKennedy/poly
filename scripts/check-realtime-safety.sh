#!/bin/bash
# Real-time safety checker for Poly audio-thread code.
# Scans processor and engine files for RT-unsafe operations.
# Suppress false positives with '// RT-SAFE-OK' on the same line.

set -euo pipefail

ERRORS=0

# Files in the audio-thread call chain:
#   process() -> renderRange() -> euclidean(), deterministicRand()
RT_FILES=(
    plugin/source/processor.cpp
    plugin/source/processor.h
    engine/src/engine.cpp
    engine/src/euclidean.cpp
    engine/include/poly/engine.h
    engine/include/poly/euclidean.h
    engine/include/poly/rng.h
    engine/include/poly/types.h
)

PATTERNS=(
    '\bnew\b'
    '\bdelete\b'
    '\bmalloc\b'
    '\bfree\b'
    '\bmake_unique\b'
    '\bmake_shared\b'
    '\bmutex\b'
    '\block_guard\b'
    '\bunique_lock\b'
    '\bscoped_lock\b'
    '\bthrow\b'
    '\bcatch\s*\('
    '\bcout\b'
    '\bcerr\b'
    '\bprintf\b'
    '\bfprintf\b'
    '\bfopen\b'
    '\bfwrite\b'
    '\bpush_back\b'
    '\bemplace_back\b'
    '\bresize\b'
    '\breserve\b'
    '\bstd::string\b'
    '\ballocateMessage\b'
    '\bsendMessage\b'
)

for file in "${RT_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        continue
    fi

    for pattern in "${PATTERNS[@]}"; do
        matches=$(grep -nE "$pattern" "$file" | grep -v 'RT-SAFE-OK' | grep -v '^\s*//' || true)
        if [ -n "$matches" ]; then
            while IFS= read -r line; do
                echo "RT-UNSAFE $file:$line"
                ERRORS=$((ERRORS + 1))
            done <<< "$matches"
        fi
    done
done

if [ $ERRORS -gt 0 ]; then
    echo ""
    echo "$ERRORS real-time safety violation(s) found."
    echo "Suppress false positives with '// RT-SAFE-OK' comment."
    exit 1
fi

echo "Real-time safety check passed."
