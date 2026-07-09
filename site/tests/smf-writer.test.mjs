import { test } from 'node:test';
import assert from 'node:assert/strict';

import {
  writeSMF,
  writeVLQ,
  noteEventsToNormalized,
  TICKS_PER_QUARTER,
} from '../src/audio/smf-writer.ts';

// Golden bytes for a 4-note stream computed by hand from the SMF spec,
// matching engine/src/smf_writer.cpp's algorithm:
//   - MThd length 6, format 0, 1 track, 480 ppq
//   - Tempo meta 500_000 us/quarter (120 BPM)
//   - Note-on ch0 pitch 36 vel 127 at ppq 0, 1, 2, 3 with duration 0.25
//   - Sorted (tick asc, note-off before note-on at same tick)
//   - VLQ deltas (0, 120, 360, 120, 360, 120, 360, 120)
//   - End-of-track meta
const GOLDEN_FOUR_NOTE_120BPM = new Uint8Array([
  0x4d, 0x54, 0x68, 0x64,
  0x00, 0x00, 0x00, 0x06,
  0x00, 0x00,
  0x00, 0x01,
  0x01, 0xe0,
  0x4d, 0x54, 0x72, 0x6b,
  0x00, 0x00, 0x00, 0x2e,
  0x00, 0xff, 0x51, 0x03, 0x07, 0xa1, 0x20,
  0x00, 0x90, 0x24, 0x7f,
  0x78, 0x80, 0x24, 0x00,
  0x82, 0x68, 0x90, 0x24, 0x7f,
  0x78, 0x80, 0x24, 0x00,
  0x82, 0x68, 0x90, 0x24, 0x7f,
  0x78, 0x80, 0x24, 0x00,
  0x82, 0x68, 0x90, 0x24, 0x7f,
  0x78, 0x80, 0x24, 0x00,
  0x00, 0xff, 0x2f, 0x00,
]);

test('writeVLQ: single-byte values', () => {
  const cases = [
    [0, [0x00]],
    [0x7f, [0x7f]],
    [64, [0x40]],
  ];
  for (const [input, expected] of cases) {
    const out = [];
    writeVLQ(input, out);
    assert.deepEqual(out, expected, `VLQ(${input})`);
  }
});

test('writeVLQ: multi-byte values match spec examples', () => {
  const cases = [
    [0x80, [0x81, 0x00]],
    [0x2000, [0xc0, 0x00]],
    [0x3fff, [0xff, 0x7f]],
    [0x4000, [0x81, 0x80, 0x00]],
    [0x0fffffff, [0xff, 0xff, 0xff, 0x7f]],
    [360, [0x82, 0x68]],
    [120, [0x78]],
  ];
  for (const [input, expected] of cases) {
    const out = [];
    writeVLQ(input, out);
    assert.deepEqual(out, expected, `VLQ(${input}=0x${input.toString(16)})`);
  }
});

test('TICKS_PER_QUARTER matches engine kSmfTicksPerQuarter', () => {
  assert.equal(TICKS_PER_QUARTER, 480);
});

test('writeSMF: MThd header structure', () => {
  const bytes = writeSMF([], 120);
  assert.equal(String.fromCharCode(bytes[0], bytes[1], bytes[2], bytes[3]), 'MThd');
  const mthdLen = (bytes[4] << 24) | (bytes[5] << 16) | (bytes[6] << 8) | bytes[7];
  assert.equal(mthdLen, 6);
  const format = (bytes[8] << 8) | bytes[9];
  assert.equal(format, 0);
  const ntrks = (bytes[10] << 8) | bytes[11];
  assert.equal(ntrks, 1);
  const division = (bytes[12] << 8) | bytes[13];
  assert.equal(division, 480);
});

test('writeSMF: tempo meta encodes microseconds per quarter correctly', () => {
  const bytes = writeSMF([], 120);
  const trackStart = 14 + 8;
  assert.equal(bytes[trackStart + 0], 0x00);
  assert.equal(bytes[trackStart + 1], 0xff);
  assert.equal(bytes[trackStart + 2], 0x51);
  assert.equal(bytes[trackStart + 3], 0x03);
  const us =
    (bytes[trackStart + 4] << 16) |
    (bytes[trackStart + 5] << 8) |
    bytes[trackStart + 6];
  assert.equal(us, 500_000);
});

test('writeSMF: empty track has just tempo + end-of-track', () => {
  const bytes = writeSMF([], 120);
  assert.equal(bytes.length, 14 + 8 + 7 + 4);
  const trackLen =
    (bytes[18] << 24) | (bytes[19] << 16) | (bytes[20] << 8) | bytes[21];
  assert.equal(trackLen, 11);
  const eot = bytes.slice(-4);
  assert.deepEqual([...eot], [0x00, 0xff, 0x2f, 0x00]);
});

test('writeSMF: 4-note stream matches golden bytes from C++ writer semantics', () => {
  const src = [
    { ppqPosition: 0, duration: 0.25, pitch: 36, velocity: 1.0, channel: 0 },
    { ppqPosition: 1, duration: 0.25, pitch: 36, velocity: 1.0, channel: 0 },
    { ppqPosition: 2, duration: 0.25, pitch: 36, velocity: 1.0, channel: 0 },
    { ppqPosition: 3, duration: 0.25, pitch: 36, velocity: 1.0, channel: 0 },
  ];
  const normalized = noteEventsToNormalized(src);
  const bytes = writeSMF(normalized, 120);
  assert.deepEqual(
    [...bytes],
    [...GOLDEN_FOUR_NOTE_120BPM],
    'SMF byte stream must match golden hex sequence',
  );
});

test('writeSMF: note-off sorts before note-on at identical tick', () => {
  const events = [
    { tick: 480, note: 36, velocity: 127, channel: 0, isOn: true },
    { tick: 480, note: 42, velocity: 0, channel: 0, isOn: false },
  ];
  const bytes = writeSMF(events, 120);
  const trackDataStart = 14 + 8 + 7;
  // VLQ(480) = 0x83 0x60 (480 = 3<<7 | 0x60), then note-off ch0 note 42 vel 0,
  // then delta 0 (same tick), then note-on ch0 note 36 vel 127.
  assert.deepEqual(
    [...bytes.slice(trackDataStart, trackDataStart + 9)],
    [0x83, 0x60, 0x80, 0x2a, 0x00, 0x00, 0x90, 0x24, 0x7f],
  );
});

test('writeSMF: channel and pitch/velocity are clamped to MIDI ranges', () => {
  const src = [
    { ppqPosition: 0, duration: 0.25, pitch: 200, velocity: 2.0, channel: 99 },
    { ppqPosition: 1, duration: 0.25, pitch: -5, velocity: -1.0, channel: -3 },
  ];
  const normalized = noteEventsToNormalized(src);
  const highOn = normalized.find((e) => e.isOn && e.tick === 0);
  assert.ok(highOn);
  assert.equal(highOn.note, 127);
  assert.equal(highOn.velocity, 127);
  assert.equal(highOn.channel, 15);
  const lowOn = normalized.find((e) => e.isOn && e.tick === 480);
  assert.ok(lowOn);
  assert.equal(lowOn.note, 0);
  assert.equal(lowOn.velocity, 0);
  assert.equal(lowOn.channel, 0);
});

test('noteEventsToNormalized: emits paired on/off events per source note', () => {
  const src = [
    { ppqPosition: 0.5, duration: 0.25, pitch: 60, velocity: 0.75, channel: 1 },
  ];
  const norm = noteEventsToNormalized(src);
  assert.equal(norm.length, 2);
  const on = norm.find((e) => e.isOn);
  const off = norm.find((e) => !e.isOn);
  assert.ok(on && off);
  assert.equal(on.tick, Math.round(0.5 * 480));
  assert.equal(off.tick, Math.round(0.75 * 480));
  assert.equal(on.velocity, Math.round(0.75 * 127));
  assert.equal(off.velocity, 0);
  assert.equal(on.channel, 1);
  assert.equal(on.note, 60);
});

test('noteEventsToNormalized: negative ppq offset clamps to zero', () => {
  const src = [
    { ppqPosition: 0.1, duration: 0.25, pitch: 60, velocity: 1.0, channel: 0 },
  ];
  const norm = noteEventsToNormalized(src, 0.5);
  const on = norm.find((e) => e.isOn);
  assert.equal(on.tick, 0);
});
