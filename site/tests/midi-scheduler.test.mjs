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

test('setLaneMuted skips events tagged with the muted lane', async () => {
  const manifest = await readManifest();
  const sampleRate = 48000;
  const durationSec = 2;
  const context = new OfflineAudioContext(2, sampleRate * durationSec, sampleRate);
  const loader = createSampleLoader({
    manifest,
    context,
    fetcher: filesystemFetcher(SAMPLES_ROOT),
  });

  // Two lanes with 4 events each. Muting lane 1 should drop half the sources.
  const pattern = {
    bpm: 120,
    loopBeats: 4,
    events: [
      { beat: 0.0, note: 36, velocity: 100, lane: 0 },
      { beat: 0.5, note: 42, velocity: 80, lane: 1 },
      { beat: 1.0, note: 36, velocity: 100, lane: 0 },
      { beat: 1.5, note: 42, velocity: 80, lane: 1 },
      { beat: 2.0, note: 36, velocity: 100, lane: 0 },
      { beat: 2.5, note: 42, velocity: 80, lane: 1 },
      { beat: 3.0, note: 36, velocity: 100, lane: 0 },
      { beat: 3.5, note: 42, velocity: 80, lane: 1 },
    ],
    lanes: [
      { label: 'Kick', role: 'kick', note: 36 },
      { label: 'Hat',  role: 'hat',  note: 42 },
    ],
  };

  const scheduler = createScheduler({
    context,
    loader,
    pattern,
    batchDurationSec: durationSec,
  });

  // Mute lane 1 before start — batch scheduling should skip all 4 of its events.
  scheduler.setLaneMuted(1, true);
  await scheduler.start();

  // 4 kick events fire in loop 0 (beats 0, 1, 2, 3 -> t=0, 0.5, 1, 1.5s).
  // Loop 1 wraps at absBeat=4 -> t=2.0s == untilTime, which is <= boundary
  // and also schedules. Hat lane is fully muted so contributes zero.
  assert.equal(
    scheduler.nodesStarted,
    5,
    `expected 5 sources fired (lane 0 only, incl. loop-wrap), got ${scheduler.nodesStarted}`,
  );
  assert.ok(scheduler.mutedLanes.has(1), 'lane 1 should still be muted after start');
});

test('setLaneMuted(idx, false) restores scheduling after unmute', async () => {
  const manifest = await readManifest();
  const sampleRate = 48000;
  const durationSec = 2;
  const context = new OfflineAudioContext(2, sampleRate * durationSec, sampleRate);
  const loader = createSampleLoader({
    manifest,
    context,
    fetcher: filesystemFetcher(SAMPLES_ROOT),
  });

  const pattern = {
    bpm: 120,
    loopBeats: 4,
    events: [
      { beat: 0.0, note: 36, velocity: 100, lane: 0 },
      { beat: 1.0, note: 36, velocity: 100, lane: 0 },
      { beat: 2.0, note: 36, velocity: 100, lane: 0 },
      { beat: 3.0, note: 36, velocity: 100, lane: 0 },
    ],
    lanes: [{ label: 'Kick', role: 'kick', note: 36 }],
  };

  const scheduler = createScheduler({
    context,
    loader,
    pattern,
    batchDurationSec: durationSec,
  });

  scheduler.setLaneMuted(0, true);
  scheduler.setLaneMuted(0, false);
  await scheduler.start();
  assert.ok(
    scheduler.nodesStarted >= 3,
    `after unmute, expected at least 3 sources fired, got ${scheduler.nodesStarted}`,
  );
  assert.ok(!scheduler.mutedLanes.has(0), 'lane 0 should be unmuted');
});

test('scheduler.start() throws when a required note has no manifest entry (no silent fallback)', async () => {
  const manifest = await readManifest();
  const sampleRate = 48000;
  const context = new OfflineAudioContext(2, sampleRate, sampleRate);
  const loader = createSampleLoader({
    manifest,
    context,
    fetcher: filesystemFetcher(SAMPLES_ROOT),
  });

  // Note 200 is deliberately out of range — no entry in the manifest.
  const pattern = {
    bpm: 120,
    loopBeats: 4,
    events: [{ beat: 0.0, note: 200, velocity: 100 }],
  };

  const scheduler = createScheduler({
    context,
    loader,
    pattern,
    batchDurationSec: 1,
  });

  await assert.rejects(
    () => scheduler.start(),
    /no sample for MIDI note 200/,
    'scheduler.start must reject rather than silently skip the missing note',
  );
});

test('scheduler throws (not silently skips) when a scheduled note lacks a buffer', async () => {
  // Loader stub returns an empty buffer map, forcing the scheduler tick loop
  // to encounter a missing buffer. Prior behavior: silent skip. New contract:
  // throw with the note number so the failure surfaces to the caller.
  const sampleRate = 48000;
  const context = new OfflineAudioContext(2, sampleRate, sampleRate);
  const emptyLoader = {
    async loadNotes() {
      return new Map();
    },
  };
  const pattern = {
    bpm: 120,
    loopBeats: 4,
    events: [{ beat: 0.0, note: 42, velocity: 100 }],
  };

  const scheduler = createScheduler({
    context,
    loader: emptyLoader,
    pattern,
    batchDurationSec: 1,
  });

  await assert.rejects(
    () => scheduler.start(),
    /no buffer loaded for MIDI note 42/,
    'scheduler must throw when tick-time buffer lookup misses (contract broken)',
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
