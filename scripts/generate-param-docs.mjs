#!/usr/bin/env node
// Regenerates the parameter tables in:
//   site/src/content/docs/appendix-parameters.mdx  (lane-core + lane-expr)
//   docs/engine-spec.md                            (LaneConfig)
//
// Reads site/src/generated/params.json (single source of truth, produced by
// site/scripts/generate-params-json.mjs from engine/include/poly/params_def.h).
//
// Regions in the docs are delimited by:
//   <!-- BEGIN GENERATED: <region-name> -->
//   ...generated content...
//   <!-- END GENERATED: <region-name> -->
// Only the content between the markers is rewritten. Descriptions come from
// the local overrides table below so we don't lose human-authored prose.

import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, '..');
const PARAMS = JSON.parse(
  readFileSync(resolve(REPO_ROOT, 'site', 'src', 'generated', 'params.json'), 'utf8'),
);

const APPENDIX_PATH = resolve(REPO_ROOT, 'site', 'src', 'content', 'docs', 'appendix-parameters.mdx');
const ENGINE_SPEC_PATH = resolve(REPO_ROOT, 'docs', 'engine-spec.md');

// Human descriptions keyed by param offset within family. Ranges/defaults come
// from the registry; only prose lives here. Update when adding new params.
const EXPR_DESCRIPTIONS = {
  0: 'Chance each note triggers per cycle',
  1: 'Default MIDI velocity',
  2: 'Probability of accented hits',
  3: 'Minimum velocity for ghost notes',
  4: 'Random velocity variation around base velocity',
  5: 'Swing feel (delays alternate steps)',
  6: 'Random timing jitter per note',
  7: 'MIDI note-off duration in beats',
  8: 'Enable or disable the lane',
  9: 'Phrase gating period in beats; 0 = no gating',
  10: 'Silence between phrases in beats',
  11: 'Phase offset for this lane\'s phrase cycle in beats',
  12: 'Per-step random pattern variation each cycle',
  13: 'Phase drift speed (Reich phasing)',
  14: 'Fixed per-lane timing offset (positive = late)',
  15: '-1 = independent; 0–7 = complement source lane',
};

const CORE_DESCRIPTIONS = {
  0: 'Cycle length in steps',
  1: 'Step grid resolution: 1=whole, 2=half, 4=quarter, 8=eighth, 16=sixteenth',
  2: 'Number of active hits per cycle (Euclidean distribution)',
  3: 'Pattern rotation offset',
  4: 'GM drum note assignment',
  5: '0 = standard Euclidean; >0 = additive cell mode (aksak)',
  6: 'Use fixed pattern instead of Euclidean distribution',
  7: '0 = use Steps value; >0 = explicit timeline length',
  8: 'Per-lane tempo scaling (Nancarrow-style)',
  9: '-1 = lane index as channel; 0–15 = explicit MIDI channel',
};

// LaneConfig table maps struct field name -> {type, description}. The default
// comes from the registry's defaultEngine for the corresponding param.
// Type + description are human prose that doesn't live in params_def.h.
const LANECONFIG_ROWS = [
  { field: 'id', type: 'int', default: '0', description: 'Lane identifier (used in RNG keying)', fromRegistry: null },
  { field: 'role', type: 'Role', default: 'Custom', description: 'Semantic role for UI grouping', fromRegistry: null },
  { field: 'midiNote', type: 'int16', default: 'MIDI Note default', description: 'MIDI pitch (drum map)', fromRegistry: { family: 'core', offset: 4 } },
  { field: 'cycle', type: 'Cycle', default: 'Steps × Subdivision defaults', description: 'Cycle length and subdivision', fromRegistry: null },
  { field: 'hitCount', type: 'int', default: 'Hits default', description: 'Euclidean pulse count', fromRegistry: { family: 'core', offset: 2 } },
  { field: 'rotation', type: 'int', default: 'Rotation default', description: 'Pattern rotation offset', fromRegistry: { family: 'core', offset: 3 } },
  { field: 'probability', type: 'float', default: 'Probability default', description: 'Per-step trigger probability', fromRegistry: { family: 'expr', offset: 0 } },
  { field: 'baseVelocity', type: 'uint8', default: 'Base Velocity default', description: 'Nominal velocity (0–127)', fromRegistry: { family: 'expr', offset: 1 } },
  { field: 'accents', type: 'AccentMask', default: 'all false', description: 'Step positions with emphasis', fromRegistry: null },
  { field: 'emphasisProb', type: 'float', default: 'Emphasis default', description: 'Accent expression probability', fromRegistry: { family: 'expr', offset: 2 } },
  { field: 'ghostFloor', type: 'uint8', default: 'Ghost Floor default', description: 'Minimum ghost-note velocity', fromRegistry: { family: 'expr', offset: 3 } },
  { field: 'velocitySpread', type: 'float', default: 'Spread default', description: 'Velocity randomization range', fromRegistry: { family: 'expr', offset: 4 } },
  { field: 'humanizeMs', type: 'float', default: 'Humanize default', description: 'Timing jitter in milliseconds', fromRegistry: { family: 'expr', offset: 6 } },
  { field: 'active', type: 'bool', default: 'Active default', description: 'Lane on/off', fromRegistry: { family: 'expr', offset: 8 } },
  { field: 'envelopes', type: 'EnvelopeAssign[4]', default: '—', description: 'Per-lane envelope assignments', fromRegistry: null },
  { field: 'envelopeCount', type: 'int', default: '0', description: 'Active envelope count', fromRegistry: null },
];

function lookup(family, offset) {
  const arr = family === 'core' ? PARAMS.coreParams : PARAMS.expressionParams;
  const entry = arr.find((e) => e.offset === offset);
  if (!entry) throw new Error(`no registry entry for ${family} offset ${offset}`);
  return entry;
}

function exprTable() {
  const rows = PARAMS.expressionParams.map((p) => {
    const desc = EXPR_DESCRIPTIONS[p.offset] ?? '';
    return `| ${p.name} | ${p.offset} | ${p.engineRange} | ${p.defaultDisplay} | ${desc} |`;
  });
  return [
    '| Parameter | ID Offset | Range | Default | Description |',
    '|---|---|---|---|---|',
    ...rows,
  ].join('\n');
}

function coreTable() {
  const rows = PARAMS.coreParams.map((p) => {
    const desc = CORE_DESCRIPTIONS[p.offset] ?? '';
    return `| ${p.name} | core ${p.offset} | ${p.engineRange} | ${p.defaultDisplay} | ${desc} |`;
  });
  return [
    '| Parameter | ID Offset | Range | Default | Description |',
    '|---|---|---|---|---|',
    ...rows,
  ].join('\n');
}

function laneConfigTable() {
  const rows = LANECONFIG_ROWS.map((r) => {
    let def = r.default;
    if (r.fromRegistry) {
      const entry = lookup(r.fromRegistry.family, r.fromRegistry.offset);
      def = entry.defaultDisplay;
    }
    return `| ${r.field} | ${r.type} | ${def} | ${r.description} |`;
  });
  return [
    '| Field | Type | Default | Description |',
    '|-------|------|---------|-------------|',
    ...rows,
  ].join('\n');
}

function replaceRegion(source, regionName, newContent, syntax) {
  // syntax: 'html' for .md files, 'mdx' for .mdx files (JSX comments)
  const [begin, end] =
    syntax === 'mdx'
      ? [`{/* BEGIN GENERATED: ${regionName} */}`, `{/* END GENERATED: ${regionName} */}`]
      : [`<!-- BEGIN GENERATED: ${regionName} -->`, `<!-- END GENERATED: ${regionName} -->`];
  const beginIdx = source.indexOf(begin);
  const endIdx = source.indexOf(end);
  if (beginIdx === -1 || endIdx === -1) {
    throw new Error(`markers for region "${regionName}" (${syntax}) not found in target file`);
  }
  if (endIdx < beginIdx) {
    throw new Error(`END marker precedes BEGIN for region "${regionName}"`);
  }
  const before = source.slice(0, beginIdx + begin.length);
  const after = source.slice(endIdx);
  return `${before}\n${newContent}\n${after}`;
}

function main() {
  // appendix-parameters.mdx: two regions (lane-core, lane-expr) — MDX syntax
  let appendix = readFileSync(APPENDIX_PATH, 'utf8');
  appendix = replaceRegion(appendix, 'lane-core', coreTable(), 'mdx');
  appendix = replaceRegion(appendix, 'lane-expr', exprTable(), 'mdx');
  writeFileSync(APPENDIX_PATH, appendix);
  console.log(`[generate-param-docs] rewrote 2 region(s) in ${APPENDIX_PATH.replace(REPO_ROOT + '/', '')}`);

  // engine-spec.md: one region (laneconfig) — HTML comment syntax
  let engineSpec = readFileSync(ENGINE_SPEC_PATH, 'utf8');
  engineSpec = replaceRegion(engineSpec, 'laneconfig', laneConfigTable(), 'html');
  writeFileSync(ENGINE_SPEC_PATH, engineSpec);
  console.log(`[generate-param-docs] rewrote 1 region(s) in ${ENGINE_SPEC_PATH.replace(REPO_ROOT + '/', '')}`);
}

main();
