#!/usr/bin/env bash
# check-sample-manifest.sh — validate site/public/samples/manifest.json.
#
# Default: every audio file has a manifest entry, every manifest entry
#          references an existing file, all SPDX IDs are allowlisted.
# --strict: CC-BY / Artistic-2.0 / MIT entries must have non-empty attribution.
# --coverage: report GM MIDI note coverage from docs/midi-note-mapping.md;
#             exit non-zero if any listed note has 0 samples.
#
# Pure bash + node (no npm install).

set -euo pipefail

STRICT=0
COVERAGE=0
for arg in "$@"; do
  case "$arg" in
    --strict) STRICT=1 ;;
    --coverage) COVERAGE=1 ;;
    -h|--help)
      echo "Usage: $0 [--strict] [--coverage]"
      exit 0
      ;;
    *)
      echo "unknown argument: $arg" >&2
      exit 2
      ;;
  esac
done

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MANIFEST="$REPO_ROOT/site/public/samples/manifest.json"
SAMPLES_DIR="$REPO_ROOT/site/public/samples"
NOTE_MAP="$REPO_ROOT/docs/midi-note-mapping.md"

if [ ! -f "$MANIFEST" ]; then
  echo "manifest missing: $MANIFEST" >&2
  exit 1
fi

STRICT="$STRICT" COVERAGE="$COVERAGE" \
MANIFEST="$MANIFEST" SAMPLES_DIR="$SAMPLES_DIR" NOTE_MAP="$NOTE_MAP" \
node - <<'NODE'
const fs = require('fs');
const path = require('path');

const strict = process.env.STRICT === '1';
const coverage = process.env.COVERAGE === '1';
const manifestPath = process.env.MANIFEST;
const samplesDir = process.env.SAMPLES_DIR;
const noteMapPath = process.env.NOTE_MAP;

const ALLOWED_SPDX = new Set([
  'CC0-1.0', 'CC-BY-4.0', 'CC-BY-3.0', 'Artistic-2.0', 'MIT',
]);
const ATTRIBUTION_REQUIRED = new Set([
  'CC-BY-4.0', 'CC-BY-3.0', 'Artistic-2.0', 'MIT',
]);
const AUDIO_EXT = new Set(['.ogg', '.mp3', '.wav', '.flac']);

let manifest;
try {
  manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
} catch (e) {
  console.error('manifest is not valid JSON: ' + e.message);
  process.exit(1);
}
if (manifest.version !== 1 || !Array.isArray(manifest.samples)) {
  console.error('manifest schema mismatch: version must be 1, samples must be an array');
  process.exit(1);
}

const errors = [];
const entriesByFile = new Map();

for (const entry of manifest.samples) {
  if (!entry || typeof entry.file !== 'string') {
    errors.push('entry missing "file" field: ' + JSON.stringify(entry));
    continue;
  }
  const abs = path.join(samplesDir, entry.file);
  if (!fs.existsSync(abs)) {
    errors.push('manifest entry points to missing file: ' + entry.file);
  }
  if (!ALLOWED_SPDX.has(entry.spdx)) {
    errors.push('entry ' + entry.file + ': spdx "' + entry.spdx + '" not in allowlist');
  }
  if (strict && ATTRIBUTION_REQUIRED.has(entry.spdx)) {
    const attr = entry.attribution;
    if (typeof attr !== 'string' || attr.trim() === '') {
      errors.push('entry ' + entry.file + ': --strict requires non-empty attribution for ' + entry.spdx);
    }
  }
  if (entriesByFile.has(entry.file)) {
    errors.push('duplicate manifest entry: ' + entry.file);
  }
  entriesByFile.set(entry.file, entry);
}

function walk(dir) {
  const out = [];
  if (!fs.existsSync(dir)) return out;
  for (const name of fs.readdirSync(dir)) {
    if (name === 'LICENSES') continue;
    const full = path.join(dir, name);
    const st = fs.statSync(full);
    if (st.isDirectory()) {
      out.push(...walk(full));
    } else if (AUDIO_EXT.has(path.extname(name).toLowerCase())) {
      out.push(full);
    }
  }
  return out;
}

const audioFiles = walk(samplesDir);
for (const abs of audioFiles) {
  const rel = path.relative(samplesDir, abs);
  if (!entriesByFile.has(rel)) {
    errors.push('audio file has no manifest entry: ' + rel);
  }
}

let coverageFailed = false;
if (coverage) {
  const notes = [];
  const seen = new Set();
  if (fs.existsSync(noteMapPath)) {
    const text = fs.readFileSync(noteMapPath, 'utf8');
    let inInventory = false;
    for (const rawLine of text.split('\n')) {
      const line = rawLine.trim();
      if (/^##\s+Note Inventory/i.test(line)) { inInventory = true; continue; }
      if (inInventory && /^##\s+/.test(line)) { inInventory = false; continue; }
      if (!inInventory) continue;
      const m = line.match(/^\|\s*(\d{1,3})\s*\|/);
      if (m) {
        const n = parseInt(m[1], 10);
        if (n >= 0 && n <= 127 && !seen.has(n)) {
          seen.add(n);
          notes.push(n);
        }
      }
    }
  }
  const covered = new Set();
  for (const entry of manifest.samples) {
    if (Array.isArray(entry.midiNotes)) {
      for (const n of entry.midiNotes) covered.add(n);
    }
  }
  const missing = notes.filter((n) => !covered.has(n));
  const total = notes.length;
  const hit = total - missing.length;
  console.log('GM coverage: ' + hit + '/' + total + ' notes covered');
  if (missing.length > 0) {
    console.log('  missing notes: ' + missing.join(', '));
    coverageFailed = total > 0;
  }
}

if (errors.length > 0) {
  for (const e of errors) console.error('ERR: ' + e);
  process.exit(1);
}

if (coverageFailed) {
  console.error('coverage failure: some GM notes have 0 samples');
  process.exit(1);
}

console.log('OK: ' + manifest.samples.length + ' manifest entries, ' + audioFiles.length + ' audio files.');
NODE
