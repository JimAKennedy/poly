import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { OfflineAudioContext } from 'node-web-audio-api';

import { createSampleLoader } from '../src/audio/sample-loader.ts';
import {
  getPatternForPreset,
  listPresetNames,
} from '../src/audio/preset-patterns.ts';

const HERE = dirname(fileURLToPath(import.meta.url));
const SAMPLES_ROOT = join(HERE, '..', 'public', 'samples');
const MANIFEST_PATH = join(SAMPLES_ROOT, 'manifest.json');

async function readManifest() {
  const raw = await readFile(MANIFEST_PATH, 'utf8');
  return JSON.parse(raw);
}

function filesystemFetcher(baseDir) {
  return async (url) => {
    const rel = url.startsWith('/samples/') ? url.slice('/samples/'.length) : url;
    const bytes = await readFile(join(baseDir, rel));
    return bytes.buffer.slice(
      bytes.byteOffset,
      bytes.byteOffset + bytes.byteLength,
    );
  };
}

test('loadNotes decodes AudioBuffers for kick / snare / hat notes', async () => {
  const manifest = await readManifest();
  const context = new OfflineAudioContext(2, 48000, 48000);
  const fetcher = filesystemFetcher(SAMPLES_ROOT);
  const loader = createSampleLoader({ manifest, context, fetcher });

  const buffers = await loader.loadNotes([36, 38, 42]);

  assert.equal(buffers.size, 3, 'expected 3 decoded buffers');
  for (const note of [36, 38, 42]) {
    const buf = buffers.get(note);
    assert.ok(buf, `missing buffer for note ${note}`);
    assert.ok(buf.length > 0, `note ${note} buffer has zero length`);
    assert.ok(
      buf.numberOfChannels >= 1,
      `note ${note} buffer has no channels`,
    );
    assert.ok(buf.sampleRate > 0, `note ${note} buffer has zero sampleRate`);
  }
});

test('loadNotes memoizes shared files across notes', async () => {
  const manifest = await readManifest();
  const context = new OfflineAudioContext(2, 48000, 48000);
  let fetchCount = 0;
  const inner = filesystemFetcher(SAMPLES_ROOT);
  const fetcher = async (url) => {
    fetchCount += 1;
    return inner(url);
  };
  const loader = createSampleLoader({ manifest, context, fetcher });

  await loader.loadNotes([36]);
  const firstCount = fetchCount;
  await loader.loadNotes([36]);
  assert.equal(
    fetchCount,
    firstCount,
    'second call for the same note should hit the cache',
  );
});

test('loadNotes rejects with note number in message for uncovered notes', async () => {
  const manifest = await readManifest();
  const context = new OfflineAudioContext(2, 48000, 48000);
  const fetcher = filesystemFetcher(SAMPLES_ROOT);
  const loader = createSampleLoader({ manifest, context, fetcher });

  await assert.rejects(
    () => loader.loadNotes([999]),
    (err) => err instanceof Error && err.message.includes('999'),
  );
});

// Website-preview role disambiguation. The shared sample manifest has cross-role
// note collisions (36 = cajon + kick, 43 = darbuka + tom) that alphabetical
// find-first resolves to the wrong voice for every factory pattern using a kick
// or tom lane. preferredRoles hints the loader per-note; the plugin path
// resolves samples independently and is unaffected.
//
// If these fixtures ever change (new collisions, roles renamed, manifest
// reordered), fix the underlying wiring — do not silently update the snapshot.

test('preferredRoles: note 36 resolves to a kick when preferred role is kick', async () => {
  const manifest = await readManifest();
  const context = new OfflineAudioContext(2, 48000, 48000);

  const requested = [];
  const fetcher = async (url) => {
    requested.push(url);
    return filesystemFetcher(SAMPLES_ROOT)(url);
  };
  const loader = createSampleLoader({
    manifest,
    context,
    fetcher,
    preferredRoles: new Map([[36, 'kick']]),
  });

  await loader.loadNotes([36]);
  assert.equal(requested.length, 1, 'expected exactly one file fetched');
  assert.match(
    requested[0],
    /\/samples\/kick\//,
    `note 36 with role=kick must resolve inside kick/, got ${requested[0]}`,
  );
});

test('preferredRoles: note 36 without hint hits the alphabetical-first collision (cajon)', async () => {
  const manifest = await readManifest();
  const context = new OfflineAudioContext(2, 48000, 48000);

  const requested = [];
  const fetcher = async (url) => {
    requested.push(url);
    return filesystemFetcher(SAMPLES_ROOT)(url);
  };
  const loader = createSampleLoader({ manifest, context, fetcher });

  await loader.loadNotes([36]);
  assert.equal(requested.length, 1);
  assert.match(
    requested[0],
    /\/samples\/cajon\//,
    'without preferredRoles, first-match still wins — confirms the fix is needed',
  );
});

test('preferredRoles: unknown role falls back to first-match', async () => {
  const manifest = await readManifest();
  const context = new OfflineAudioContext(2, 48000, 48000);

  const requested = [];
  const fetcher = async (url) => {
    requested.push(url);
    return filesystemFetcher(SAMPLES_ROOT)(url);
  };
  const loader = createSampleLoader({
    manifest,
    context,
    fetcher,
    preferredRoles: new Map([[36, 'not-a-real-role']]),
  });

  await loader.loadNotes([36]);
  assert.equal(requested.length, 1);
  assert.match(
    requested[0],
    /\/samples\/cajon\//,
    'unknown role must fall back to the first note-36 entry',
  );
});

test('preferredRoles: note 43 with role=tom picks tom (not darbuka)', async () => {
  const manifest = await readManifest();
  const context = new OfflineAudioContext(2, 48000, 48000);

  const requested = [];
  const fetcher = async (url) => {
    requested.push(url);
    return filesystemFetcher(SAMPLES_ROOT)(url);
  };
  const loader = createSampleLoader({
    manifest,
    context,
    fetcher,
    preferredRoles: new Map([[43, 'tom']]),
  });

  await loader.loadNotes([43]);
  assert.equal(requested.length, 1);
  assert.match(requested[0], /\/samples\/tom\//, `got ${requested[0]}`);
});

// Snapshot: exercise every (note, role) pair used across all factory patterns
// and lock the resolved file. This is the "spot mismatches" gate — a new
// preset, a new manifest entry, or a rename that changes any resolution here
// must be a conscious edit to this table, not a silent behavior change.
test('preferredRoles: pattern-lane resolution snapshot covers all factory presets', async () => {
  const manifest = await readManifest();
  const context = new OfflineAudioContext(2, 48000, 48000);

  const pairs = new Map(); // key = `${note}:${role}` -> {note, role}
  for (const name of listPresetNames()) {
    const pattern = getPatternForPreset(name);
    assert.ok(pattern, `getPatternForPreset('${name}') returned null`);
    for (const lane of pattern.lanes ?? []) {
      pairs.set(`${lane.note}:${lane.role}`, { note: lane.note, role: lane.role });
    }
  }

  // Expected file per (note, role). Roles with no matching manifest entry
  // fall back through first-note-match; those fallbacks are called out below
  // so a manifest edit that shadows them fails loudly instead of silently
  // switching voices.
  const expected = {
    '36:kick':      'kick/boochi44-kick.wav',           // was cajon before fix
    '38:snare':     'snare/drskit-snare.flac',
    '42:hat':       'hat/boochi44-hat.wav',
    '43:tom':       'tom/muldjord-tom-lo.flac',         // was darbuka before fix
    '45:tom':       'tom/muldjord-tom-mid.flac',
    '47:tom':       'tom/muldjord-tom-mid.flac',        // T05: mid tom widened to cover 47 (low-mid tom)
    '50:tom':       'tom/muldjord-tom-hi.flac',         // Manding Djembe hi-tom lane
    '44:hat':       'hat/boochi44-hat.wav',             // T06: alias-retired presets use foot-hat (fallback:pedal-hat)
    '46:openhat':   'hat/fischer808-oh.wav',            // manifest role is 'hat'; falls back to only 46 entry
    '48:tom':       'tom/muldjord-tom-hi.flac',         // T06: hi-mid tom (fallback:hi-mid-tom, T05 widened)
    '51:cymbal':    'cymbal/muldjord-ride.flac',        // T06: Muldjord ride, GM note 51
    '55:cymbal':    'cymbal/muldjord-china.flac',       // T06: Muldjord china tagged fallback:china for splash
    '56:cowbell':   'cowbell/fischer808-cowbell.wav',
    '60:bongo':     'bongo/freepats-bongo-hi.flac',     // T06: only 60-carrying entry
    '62:conga':     'conga/freepats-conga-open.flac',   // T06: open conga (fallback:mute-hi-conga, T05 widened)
    '63:conga':     'conga/freepats-conga-open.flac',
    '64:conga':     'conga/freepats-conga-low.flac',    // T06: FreePats LowConga, GM note 64
    '67:agogo':     'agogo/vcsl-hi.wav',
    '70:shaker':    'shaker/dimcabasa-cabasa.flac',
    '72:shaker':    'shaker/dimcabasa-cabasa.flac',     // T06: same file covers 70+72 (fallback:long-shaker, T05 widened)
    '74:guiro':     'guiro/vcsl-guiro-long.wav',        // T06: VCSL Guiro_Slow, GM note 74 (Long Guiro)
    '75:clave':     'clave/fischer808-clave.wav',
    '76:woodblock': 'woodblock/vcsl-hi.wav',
    '77:woodblock': 'woodblock/vcsl-lo.wav',
    // Fallbacks — no manifest entry matches the requested role. These land on
    // the alphabetical first-match. If a future manifest edit adds a matching
    // role, this test will flag it; either add the entry to the table (with the
    // new file) or reconcile the pattern's role name.
    '37:rim':       'clave/fischer808-clave.wav',       // clave has fallback:sidestick tag
    '39:clap':      'handclap/freepats-clap.flac',      // manifest role is 'handclap'
  };

  const missing = [];
  for (const key of pairs.keys()) {
    if (!(key in expected)) missing.push(key);
  }
  assert.deepEqual(
    missing,
    [],
    `pattern uses new (note, role) pair not in snapshot: ${missing.join(', ')} — add it to expected{} above`,
  );

  for (const [key, { note, role }] of pairs) {
    const requested = [];
    const fetcher = async (url) => {
      requested.push(url);
      return filesystemFetcher(SAMPLES_ROOT)(url);
    };
    const loader = createSampleLoader({
      manifest,
      context,
      fetcher,
      preferredRoles: new Map([[note, role]]),
    });
    await loader.loadNotes([note]);
    assert.equal(requested.length, 1, `${key}: expected 1 fetch, got ${requested.length}`);
    const rel = requested[0].replace(/^\/samples\//, '');
    assert.equal(rel, expected[key], `${key}: expected ${expected[key]}, got ${rel}`);
  }
});
