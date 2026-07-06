import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { OfflineAudioContext } from 'node-web-audio-api';

import { createSampleLoader } from '../src/audio/sample-loader.ts';

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
