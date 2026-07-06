import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { OfflineAudioContext } from 'node-web-audio-api';

import { createSampleLoader } from '../src/audio/sample-loader.ts';
import { createScheduler } from '../src/audio/midi-scheduler.ts';

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

function maxAbs(channelData) {
  let peak = 0;
  for (let i = 0; i < channelData.length; i++) {
    const v = Math.abs(channelData[i]);
    if (v > peak) peak = v;
  }
  return peak;
}

test('scheduler renders scheduled notes into an OfflineAudioContext', async () => {
  const manifest = await readManifest();
  const sampleRate = 48000;
  const durationSec = 3;
  const context = new OfflineAudioContext(2, sampleRate * durationSec, sampleRate);
  const loader = createSampleLoader({
    manifest,
    context,
    fetcher: filesystemFetcher(SAMPLES_ROOT),
  });

  // 6 events across ~2 seconds: kick on beats 0, 1, 2, 3; hat on beats 0.5, 1.5, 2.5, 3.5.
  // At 120 BPM (0.5s/beat), event at beat 3 fires at t=1.5s; hat at beat 3.5 fires at t=1.75s.
  const pattern = {
    bpm: 120,
    loopBeats: 4,
    events: [
      { beat: 0.0, note: 36, velocity: 110 },
      { beat: 0.5, note: 42, velocity: 80 },
      { beat: 1.0, note: 36, velocity: 100 },
      { beat: 1.5, note: 42, velocity: 80 },
      { beat: 2.0, note: 36, velocity: 105 },
      { beat: 2.5, note: 42, velocity: 80 },
      { beat: 3.0, note: 36, velocity: 100 },
      { beat: 3.5, note: 42, velocity: 80 },
    ],
  };

  const scheduler = createScheduler({
    context,
    loader,
    pattern,
    batchDurationSec: durationSec,
  });

  await scheduler.start();
  assert.ok(
    scheduler.nodesStarted >= 6,
    `expected >= 6 buffer sources fired, got ${scheduler.nodesStarted}`,
  );

  const rendered = await context.startRendering();
  assert.equal(rendered.length, sampleRate * durationSec);
  const peak = maxAbs(rendered.getChannelData(0));
  assert.ok(
    peak > 0.001,
    `rendered buffer is silent (peak=${peak}) — sources did not connect`,
  );
});

test('scheduler stop() is safe to call before and after start', async () => {
  const manifest = await readManifest();
  const sampleRate = 48000;
  const context = new OfflineAudioContext(2, sampleRate, sampleRate);
  const loader = createSampleLoader({
    manifest,
    context,
    fetcher: filesystemFetcher(SAMPLES_ROOT),
  });

  const pattern = {
    bpm: 120,
    loopBeats: 4,
    events: [
      { beat: 0.0, note: 36, velocity: 100 },
      { beat: 1.0, note: 36, velocity: 100 },
    ],
  };

  const scheduler = createScheduler({
    context,
    loader,
    pattern,
    batchDurationSec: 1,
  });

  // Calling stop() before start() must not throw.
  scheduler.stop();
  await scheduler.start();
  const firedAfterFirstStart = scheduler.nodesStarted;
  assert.ok(firedAfterFirstStart >= 1, 'first start should schedule at least the beat-0 event');
  scheduler.stop();
  // stop() twice is idempotent.
  scheduler.stop();
});
