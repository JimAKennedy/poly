#!/usr/bin/env node
// Verify every action in webui/bridge.schema.json's msgAction.name.enum has
// either a dedicated payload<Cap> definition or is listed in payloadEmpty's
// description as an empty-payload action.
//
// Why: the schema previously drifted (M047 S06 fixed applyPreset typing) when
// an action was added to the enum without a corresponding payload shape.
// This check makes that class of drift a red build.

import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const SCHEMA_PATH = resolve(HERE, '..', 'webui', 'bridge.schema.json');

function fail(msg) {
  process.stderr.write(`::error file=webui/bridge.schema.json::${msg}\n`);
  process.exit(1);
}

const schema = JSON.parse(readFileSync(SCHEMA_PATH, 'utf8'));

const actions = schema?.definitions?.msgAction?.properties?.name?.enum;
if (!Array.isArray(actions) || actions.length === 0) {
  fail('msgAction.name.enum missing or empty');
}

const defs = Object.keys(schema?.definitions ?? {});
const emptyDesc = schema?.definitions?.payloadEmpty?.description ?? '';

const missing = [];
for (const action of actions) {
  const cap = action[0].toUpperCase() + action.slice(1);
  const specific = `payload${cap}`;
  if (defs.includes(specific)) continue;
  // Fall back to payloadEmpty membership: check the description names this action.
  const wordBoundaryRe = new RegExp(`\\b${action}\\b`);
  if (wordBoundaryRe.test(emptyDesc)) continue;
  missing.push(action);
}

if (missing.length > 0) {
  fail(
    `Action(s) [${missing.join(', ')}] declared in msgAction.name.enum but have ` +
      `no matching payload<Cap> definition AND are not listed in payloadEmpty.description. ` +
      `Either add payload${missing[0][0].toUpperCase()}${missing[0].slice(1)} or extend ` +
      `payloadEmpty's description to include the new action name.`,
  );
}

process.stdout.write(
  `Bridge schema coverage OK — ${actions.length} action(s), ` +
    `${defs.filter((d) => d.startsWith('payload')).length} payload def(s)\n`,
);
