#!/usr/bin/env node
// Runs the C++ `poly_params_emit` binary and writes its JSON output to
// site/src/generated/params.json. Invoked as part of the site prebuild so
// the doc-regen step (scripts/generate-param-docs.mjs) always sees a fresh
// snapshot of engine/include/poly/params_def.h — the single source of truth.
//
// Contract with the emitter (engine/tools/emit_params.cpp):
//   { schemaVersion: 1, expressionParams: [...], coreParams: [...] }

import { execFileSync, execSync } from 'node:child_process';
import { existsSync, mkdirSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, '..', '..');
// Reuse the same engine-only build tree as the presets emitter.
const BUILD_DIR = resolve(REPO_ROOT, 'build-presets');
const EMITTER = resolve(BUILD_DIR, 'engine', 'tools', 'poly_params_emit');
const OUT_PATH = resolve(REPO_ROOT, 'site', 'src', 'generated', 'params.json');

function log(msg) {
  process.stdout.write(`[generate-params] ${msg}\n`);
}

function fail(msg) {
  process.stderr.write(`[generate-params] ${msg}\n`);
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
  log('building poly_params_emit');
  execSync(`cmake --build "${BUILD_DIR}" --target poly_params_emit`, {
    stdio: 'inherit',
  });
  if (!existsSync(EMITTER)) {
    fail(`emitter not produced at ${EMITTER} after build`);
  }
}

function runEmitter() {
  return execFileSync(EMITTER, { encoding: 'utf8', maxBuffer: 4 * 1024 * 1024 });
}

function validate(parsed) {
  if (parsed.schemaVersion !== 1) {
    fail(`unexpected schemaVersion ${parsed.schemaVersion} — regenerate/update consumers`);
  }
  if (!Array.isArray(parsed.expressionParams) || parsed.expressionParams.length !== 16) {
    fail(`expressionParams: expected 16 entries, got ${parsed.expressionParams?.length}`);
  }
  if (!Array.isArray(parsed.coreParams) || parsed.coreParams.length !== 10) {
    fail(`coreParams: expected 10 entries, got ${parsed.coreParams?.length}`);
  }
  const all = [...parsed.expressionParams, ...parsed.coreParams];
  for (const p of all) {
    if (typeof p.name !== 'string' || !p.name.length) {
      fail(`param offset ${p.offset}: missing name`);
    }
    if (typeof p.kind !== 'string' || !p.kind.length) {
      fail(`param "${p.name}": missing kind`);
    }
    if (typeof p.engineRange !== 'string' || !p.engineRange.length) {
      fail(`param "${p.name}": missing engineRange`);
    }
  }
}

ensureEmitter();
const raw = runEmitter();
let parsed;
try {
  parsed = JSON.parse(raw);
} catch (e) {
  fail(`emitter output is not valid JSON: ${e.message}`);
}
validate(parsed);

mkdirSync(dirname(OUT_PATH), { recursive: true });
writeFileSync(OUT_PATH, JSON.stringify(parsed, null, 2) + '\n');
log(
  `wrote ${OUT_PATH.replace(REPO_ROOT + '/', '')} — expr:${parsed.expressionParams.length} core:${parsed.coreParams.length}`,
);
