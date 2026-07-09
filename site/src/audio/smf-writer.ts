// SMF (Standard MIDI File) writer. Byte-for-byte parity with
// engine/src/smf_writer.cpp so the equivalence spec can diff dumps from the
// site card path (engine scheduler) and the WASM Try It path.
//
// Format: SMF format 0, single MTrk, 480 ppq (kSmfTicksPerQuarter).

export const TICKS_PER_QUARTER = 480;

export interface NormalizedEvent {
  tick: number;
  note: number;
  velocity: number;
  channel: number;
  isOn: boolean;
}

export interface NoteEventLike {
  ppqPosition: number;
  duration: number;
  pitch: number;
  velocity: number;
  channel: number;
}

export function writeVLQ(value: number, out: number[]): void {
  if (value < 0x80) {
    out.push(value & 0x7f);
    return;
  }
  const temp: number[] = [];
  temp.push(value & 0x7f);
  let v = value >>> 7;
  while (v > 0) {
    temp.push((v & 0x7f) | 0x80);
    v >>>= 7;
  }
  for (let i = temp.length - 1; i >= 0; i--) out.push(temp[i]);
}

function pushBE16(out: number[], value: number): void {
  out.push((value >> 8) & 0xff, value & 0xff);
}

function pushBE32(out: number[], value: number): void {
  out.push(
    (value >>> 24) & 0xff,
    (value >>> 16) & 0xff,
    (value >>> 8) & 0xff,
    value & 0xff,
  );
}

function clamp(value: number, lo: number, hi: number): number {
  return value < lo ? lo : value > hi ? hi : value;
}

export function noteEventsToNormalized(
  events: NoteEventLike[],
  ppqOffset = 0,
): NormalizedEvent[] {
  const out: NormalizedEvent[] = [];
  for (const e of events) {
    const ppq = Math.max(0, e.ppqPosition - ppqOffset);
    const onTick = Math.round(ppq * TICKS_PER_QUARTER);
    const offTick = Math.round((ppq + e.duration) * TICKS_PER_QUARTER);
    const vel = clamp(Math.round(e.velocity * 127), 0, 127);
    const ch = clamp(e.channel | 0, 0, 15);
    const note = clamp(e.pitch | 0, 0, 127);
    out.push({ tick: onTick, note, velocity: vel, channel: ch, isOn: true });
    out.push({ tick: offTick, note, velocity: 0, channel: ch, isOn: false });
  }
  return out;
}

export function writeSMF(
  events: NormalizedEvent[],
  tempo: number,
  ppq: number = TICKS_PER_QUARTER,
): Uint8Array {
  const data: number[] = [];

  data.push(0x4d, 0x54, 0x68, 0x64);
  pushBE32(data, 6);
  pushBE16(data, 0);
  pushBE16(data, 1);
  pushBE16(data, ppq);

  const track: number[] = [];

  const usPerQuarter = Math.round(60_000_000 / tempo);
  writeVLQ(0, track);
  track.push(0xff, 0x51, 0x03);
  track.push(
    (usPerQuarter >> 16) & 0xff,
    (usPerQuarter >> 8) & 0xff,
    usPerQuarter & 0xff,
  );

  // Match engine sort: primary by tick, secondary by (status & 0xF0) so 0x80
  // (note-off) precedes 0x90 (note-on) at the same tick.
  const sorted = events.slice().sort((a, b) => {
    if (a.tick !== b.tick) return a.tick - b.tick;
    const sa = a.isOn ? 0x90 : 0x80;
    const sb = b.isOn ? 0x90 : 0x80;
    return sa - sb;
  });

  let prevTick = 0;
  for (const m of sorted) {
    const delta = m.tick - prevTick;
    writeVLQ(delta, track);
    const ch = clamp(m.channel | 0, 0, 15);
    const status = (m.isOn ? 0x90 : 0x80) | ch;
    const note = clamp(m.note | 0, 0, 127);
    const vel = clamp(m.velocity | 0, 0, 127);
    track.push(status, note, vel);
    prevTick = m.tick;
  }

  writeVLQ(0, track);
  track.push(0xff, 0x2f, 0x00);

  data.push(0x4d, 0x54, 0x72, 0x6b);
  pushBE32(data, track.length);
  for (let i = 0; i < track.length; i++) data.push(track[i]);

  return new Uint8Array(data);
}

export function downloadSMF(bytes: Uint8Array, filename: string): void {
  if (typeof document === 'undefined' || typeof URL === 'undefined') return;
  const blob = new Blob([bytes], { type: 'audio/midi' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = filename;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}
