#!/usr/bin/env node
// Regenerate the action/message reference tables in webui/bridge-schema.md from
// webui/bridge.schema.json. Follows the S05 generate-param-docs pattern: content
// inside <!-- BEGIN GENERATED: name --> / <!-- END GENERATED: name --> markers
// is fully rewritten; content outside is preserved verbatim.

import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, '..');
const SCHEMA_PATH = resolve(REPO_ROOT, 'webui', 'bridge.schema.json');
const DOC_PATH = resolve(REPO_ROOT, 'webui', 'bridge-schema.md');

function fail(msg) {
  process.stderr.write(`[generate-bridge-schema-doc] ${msg}\n`);
  process.exit(1);
}

const schema = JSON.parse(readFileSync(SCHEMA_PATH, 'utf8'));

// ---------------- actions table ----------------
// Columns: Action name | Payload type | Required fields | Notes
const actions = schema?.definitions?.msgAction?.properties?.name?.enum ?? [];
const defs = schema?.definitions ?? {};

function payloadRefFor(action) {
  const cap = action[0].toUpperCase() + action.slice(1);
  const specific = `payload${cap}`;
  if (defs[specific]) return { key: specific, def: defs[specific] };
  return { key: 'payloadEmpty', def: defs.payloadEmpty };
}

function requiredList(payloadDef) {
  const req = payloadDef?.required;
  if (!Array.isArray(req) || req.length === 0) return '—';
  return req.map((f) => `\`${f}\``).join(', ');
}

function payloadNotes(payloadDef) {
  const desc = payloadDef?.description;
  if (typeof desc === 'string' && desc.length > 0) return desc.replace(/\|/g, '\\|');
  return '';
}

const actionRows = actions.map((a) => {
  const { key, def } = payloadRefFor(a);
  const required = requiredList(def);
  const notes = payloadNotes(def);
  return `| \`${a}\` | \`${key}\` | ${required} | ${notes} |`;
});

const actionsTable = [
  '| Action | Payload type | Required fields | Notes |',
  '|---|---|---|---|',
  ...actionRows,
].join('\n');

// ---------------- messages table ----------------
// Columns: msg type | direction | fields | required
const messageOrder = ['msgReady', 'msgEdit', 'msgAction', 'msgState', 'msgFrame'];
const directions = {
  msgReady: 'JS → C++',
  msgEdit: 'JS → C++',
  msgAction: 'JS → C++',
  msgState: 'C++ → JS',
  msgFrame: 'C++ → JS',
};

function propFieldSummary(propDef) {
  // Return a compact one-line summary of the type/enum/ref for the table cell.
  if (propDef.const !== undefined) return `\`"${propDef.const}"\``;
  if (propDef.$ref) return `\`${propDef.$ref.split('/').pop()}\``;
  if (propDef.enum) return propDef.enum.map((v) => `\`${v}\``).join('\\|');
  if (propDef.type === 'integer' || propDef.type === 'number') {
    const bits = [propDef.type];
    if (propDef.minimum !== undefined) bits.push(`≥ ${propDef.minimum}`);
    if (propDef.maximum !== undefined) bits.push(`≤ ${propDef.maximum}`);
    return bits.join(' ');
  }
  return propDef.type ?? '?';
}

const messageRows = messageOrder.map((mt) => {
  const def = defs[mt];
  if (!def) return `| \`${mt}\` | | (missing) | |`;
  const dir = directions[mt] ?? '';
  const propNames = Object.keys(def.properties ?? {});
  const fields = propNames
    .map((p) => `\`${p}\`: ${propFieldSummary(def.properties[p])}`)
    .join('<br>');
  const required = (def.required ?? []).map((r) => `\`${r}\``).join(', ') || '—';
  return `| \`${mt}\` | ${dir} | ${fields} | ${required} |`;
});

const messagesTable = [
  '| Message type | Direction | Fields | Required |',
  '|---|---|---|---|',
  ...messageRows,
].join('\n');

// ---------------- rewrite doc ----------------
const rewriteRegion = (source, regionName, newBody) => {
  const begin = `<!-- BEGIN GENERATED: ${regionName} -->`;
  const end = `<!-- END GENERATED: ${regionName} -->`;
  const startIdx = source.indexOf(begin);
  const endIdx = source.indexOf(end);
  if (startIdx === -1 || endIdx === -1) {
    fail(`markers for region '${regionName}' not found in ${DOC_PATH}`);
  }
  if (endIdx < startIdx) {
    fail(`END marker precedes BEGIN for region '${regionName}'`);
  }
  const before = source.slice(0, startIdx + begin.length);
  const after = source.slice(endIdx);
  return `${before}\n${newBody}\n${after}`;
};

let doc = readFileSync(DOC_PATH, 'utf8');
doc = rewriteRegion(doc, 'actions', actionsTable);
doc = rewriteRegion(doc, 'messages', messagesTable);
writeFileSync(DOC_PATH, doc);

process.stdout.write(
  `[generate-bridge-schema-doc] rewrote actions (${actions.length} rows) + messages (${messageOrder.length} rows) in webui/bridge-schema.md\n`,
);
