#!/usr/bin/env node
// Runs the C++ `poly_presets_emit` binary and writes its JSON output to
// site/src/generated/presets.json. Invoked as the site's prebuild step so the
// card runtime, chapter docs, and WASM host all consume engine-authored lane
// data from the same file instead of drifting hand-copies.
//
// Contract with the emitter (engine/tools/emit_presets.cpp):
//   { schemaVersion, presetCount, categories: [...], presets: [{ index, name, category, notesInBar, lanes: [...] }] }
//
// If the emitter binary is missing, we configure (only if needed) and build it.
// A native build failure exits non-zero and fails the site build — that is
// intentional: a stale presets.json is a silent correctness bug we already
// paid for in S11's motivating incident.

import { execFileSync, execSync } from 'node:child_process';
import { existsSync, mkdirSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, '..', '..');
// Dedicated engine-only build tree. The emitter depends only on poly_engine
// (see engine/tools/CMakeLists.txt), so we configure with -DPOLY_ENGINE_ONLY=ON.
// This avoids VST3 SDK / X11 requirements on Linux CI and keeps the shared
// `build/` cache untouched so a plugin build later isn't poisoned.
const BUILD_DIR = resolve(REPO_ROOT, 'build-presets');
const EMITTER = resolve(BUILD_DIR, 'engine', 'tools', 'poly_presets_emit');
const OUT_PATH = resolve(REPO_ROOT, 'site', 'src', 'generated', 'presets.json');
// The WASM Try It modal fetches this file at boot from /webui/presets.json.
// Writing it here alongside the build-time copy keeps dev-mode consumers in
// sync without depending on `npm run copy-webui`.
const WEBUI_OUT_PATH = resolve(REPO_ROOT, 'site', 'public', 'webui', 'presets.json');

function log(msg) {
  process.stdout.write(`[generate-presets] ${msg}\n`);
}

function fail(msg) {
  process.stderr.write(`[generate-presets] ${msg}\n`);
  process.exit(1);
}

function ensureEmitter() {
  if (existsSync(EMITTER)) return;
  if (!existsSync(resolve(BUILD_DIR, 'CMakeCache.txt'))) {
    log('configuring cmake build-presets/ (first run, engine-only)');
    execSync(`cmake -S "${REPO_ROOT}" -B "${BUILD_DIR}" -DPOLY_ENGINE_ONLY=ON`, {
      stdio: 'inherit',
    });
  }
  log('building poly_presets_emit');
  execSync(`cmake --build "${BUILD_DIR}" --target poly_presets_emit`, {
    stdio: 'inherit',
  });
  if (!existsSync(EMITTER)) {
    fail(`emitter not produced at ${EMITTER} after build`);
  }
}

function runEmitter() {
  return execFileSync(EMITTER, { encoding: 'utf8', maxBuffer: 8 * 1024 * 1024 });
}

function validate(parsed) {
  if (typeof parsed.schemaVersion !== 'number') {
    fail('missing schemaVersion');
  }
  if (parsed.schemaVersion !== 2) {
    fail(`unexpected schemaVersion ${parsed.schemaVersion} — regenerate/update consumers`);
  }
  if (!Array.isArray(parsed.categories) || parsed.categories.length === 0) {
    fail('categories array missing or empty');
  }
  const categorySet = new Set(parsed.categories);
  if (!Array.isArray(parsed.presets)) {
    fail('presets array missing');
  }
  if (parsed.presets.length !== parsed.presetCount) {
    fail(
      `presetCount (${parsed.presetCount}) disagrees with presets.length (${parsed.presets.length})`,
    );
  }
  for (const p of parsed.presets) {
    if (typeof p.name !== 'string' || !p.name.length) {
      fail(`preset index ${p.index}: missing name`);
    }
    if (typeof p.category !== 'string' || !p.category.length) {
      fail(`preset "${p.name}": missing category`);
    }
    if (!categorySet.has(p.category)) {
      fail(`preset "${p.name}": category "${p.category}" not in declared enum`);
    }
    if (!Array.isArray(p.lanes) || p.lanes.length === 0) {
      fail(`preset "${p.name}": no lanes`);
    }
    for (const lane of p.lanes) {
      if (typeof lane.roleLabel !== 'string' || !lane.roleLabel.length) {
        fail(`preset "${p.name}" lane ${lane.laneIndex}: missing roleLabel`);
      }
    }
  }
}

// region:presets-emitter-flow
ensureEmitter();
const raw = runEmitter();
let parsed;
try {
  parsed = JSON.parse(raw);
} catch (err) {
  fail(`emitter output is not valid JSON: ${err.message}`);
}
validate(parsed);

const serialized = `${JSON.stringify(parsed, null, 2)}\n`;
mkdirSync(dirname(OUT_PATH), { recursive: true });
writeFileSync(OUT_PATH, serialized);
log(`wrote ${parsed.presetCount} presets → ${OUT_PATH}`);
mkdirSync(dirname(WEBUI_OUT_PATH), { recursive: true });
writeFileSync(WEBUI_OUT_PATH, serialized);
log(`wrote ${parsed.presetCount} presets → ${WEBUI_OUT_PATH}`);
// endregion:presets-emitter-flow
