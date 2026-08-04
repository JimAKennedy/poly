// Pure derivation of the appendix-presets parameter tables from the generated
// presets.json (schemaVersion 3, see engine/tools/emit_presets.cpp and
// site/scripts/generate-presets-json.mjs).
//
// PresetTable.astro renders EXACTLY the {headers, rows, macros} this module
// returns, and preset-table-conformance.test.mjs re-derives every cell straight
// from the raw presets.json fields — so no displayed parameter can diverge from
// engine truth (M071 D026/D4: appendix-presets.mdx carries zero hand tables).
//
// This module is intentionally dependency-free (no Astro, no fs) so both the
// component and the test import the same derivation.

function capitalize(label) {
  return label ? label.charAt(0).toUpperCase() + label.slice(1) : label;
}

// Render a number tersely: integers stay bare, fractions drop trailing zeros
// (0.15 -> "0.15", -1.5 -> "-1.5", 0 -> "0", 3 -> "3").
export function formatNumber(value) {
  if (Number.isInteger(value)) return String(value);
  return String(parseFloat(value.toFixed(3)));
}

// Ordered column catalogue. `always` columns render for every preset; the rest
// render only when at least one lane in the preset satisfies `active(lane)`.
// This reproduces the per-preset column variation of the old hand tables
// (Phrase/Kotekan/Cells/Micro-timing/etc.) directly from the data.
export const COLUMNS = [
  { key: 'lane', header: 'Lane', always: true, render: (lane, index) => String(index + 1) },
  { key: 'role', header: 'Role', always: true, render: (lane) => capitalize(lane.roleLabel) },
  { key: 'steps', header: 'Steps', always: true, render: (lane) => String(lane.cycleSteps) },
  { key: 'hits', header: 'Hits', always: true, render: (lane) => String(lane.hits) },
  { key: 'rotation', header: 'Rotation', always: true, render: (lane) => String(lane.rotation) },
  { key: 'subdivision', header: 'Subdivision', always: true, render: (lane) => `1/${lane.subdivision}` },
  { key: 'note', header: 'Note', always: true, render: (lane) => String(lane.noteNumber) },
  { key: 'velocity', header: 'Velocity', always: true, render: (lane) => String(lane.velocity) },
  { key: 'ghost', header: 'Ghost', always: true, render: (lane) => String(lane.ghostFloor) },
  {
    key: 'swing',
    header: 'Swing',
    active: (lane) => lane.swingAmount !== 0,
    render: (lane) => formatNumber(lane.swingAmount),
  },
  {
    key: 'mutation',
    header: 'Mutation',
    active: (lane) => lane.mutationRate !== 0,
    render: (lane) => formatNumber(lane.mutationRate),
  },
  {
    key: 'drift',
    header: 'Drift',
    active: (lane) => lane.driftRate !== 0,
    render: (lane) => formatNumber(lane.driftRate),
  },
  {
    key: 'timing',
    header: 'Timing',
    active: (lane) => lane.timingOffsetMs !== 0,
    render: (lane) => `${formatNumber(lane.timingOffsetMs)}ms`,
  },
  {
    key: 'humanize',
    header: 'Humanize',
    active: (lane) => lane.humanizeMs !== 0,
    render: (lane) => `${formatNumber(lane.humanizeMs)}ms`,
  },
  {
    key: 'micro',
    header: 'Micro-timing',
    active: (lane) => lane.hasMicroTiming,
    render: (lane) => (lane.hasMicroTiming ? 'yes' : '—'),
  },
  {
    key: 'kotekan',
    header: 'Kotekan',
    active: (lane) => lane.kotekanSourceLane !== -1,
    render: (lane) => (lane.kotekanSourceLane === -1 ? 'off' : `L${lane.kotekanSourceLane + 1}`),
  },
  {
    key: 'phraseLength',
    header: 'Phrase Len',
    active: (lane) => lane.phraseLength !== 0,
    render: (lane) => String(lane.phraseLength),
  },
  {
    key: 'phraseGap',
    header: 'Gap',
    active: (lane) => lane.phraseLength !== 0,
    render: (lane) => String(lane.phraseGap),
  },
  {
    key: 'phraseOffset',
    header: 'Offset',
    active: (lane) => lane.phraseLength !== 0,
    render: (lane) => String(lane.phraseOffset),
  },
  {
    key: 'cells',
    header: 'Cells',
    active: (lane) => lane.cellCount > 0,
    render: (lane) => (lane.cellCount > 0 ? `[${lane.cellSizes.join('+')}]` : '—'),
  },
  {
    key: 'timeline',
    header: 'Timeline',
    active: (lane) => lane.timeline,
    render: (lane) => (lane.timeline ? `fixed(${lane.fixedPatternLength})` : '—'),
  },
];

// The six macros, in the fixed display order the appendix Macros line uses.
export const MACRO_ORDER = [
  ['Density', 'density'],
  ['Complexity', 'complexity'],
  ['Syncopation', 'syncopation'],
  ['Swing', 'swing'],
  ['Tension', 'tension'],
  ['Humanize', 'humanize'],
];

// Find a preset record by its exact factory name. Throws (loud build/test
// failure) when the name does not resolve — no silent empty table.
export function findPreset(presets, name) {
  const record = presets.find((preset) => preset.name === name);
  if (!record) {
    throw new Error(`PresetTable: preset "${name}" not found in presets.json`);
  }
  return record;
}

// Build the rendered table + macros line for one preset record. Returns
// { headers, keys, rows, macros } where rows is a matrix of pre-formatted
// strings aligned to headers/keys, and macros is the "Density 0.5, ..." parts.
export function buildPresetTable(preset) {
  const lanes = preset.lanes;
  const columns = COLUMNS.filter((column) => column.always || lanes.some((lane) => column.active(lane)));
  const rows = lanes.map((lane, index) => columns.map((column) => column.render(lane, index)));
  const macros = MACRO_ORDER.map(([label, key]) => `${label} ${formatNumber(preset.macros[key])}`);
  return {
    headers: columns.map((column) => column.header),
    keys: columns.map((column) => column.key),
    rows,
    macros,
  };
}
