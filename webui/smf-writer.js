'use strict';
/**
 * Plain-JS port of site/src/audio/smf-writer.ts. Byte-for-byte parity with the
 * TS module and engine/src/smf_writer.cpp so the S13 parity harness can diff
 * card-Play (TS) vs Try-It (this) vs engine (C++) dumps meaningfully.
 *
 * Exposed as window.PolySmfWriter for the standalone webui/ bundle.
 */
(function () {
  const TICKS_PER_QUARTER = 480;

  function writeVLQ(value, out) {
    if (value < 0x80) {
      out.push(value & 0x7f);
      return;
    }
    const temp = [];
    temp.push(value & 0x7f);
    let v = value >>> 7;
    while (v > 0) {
      temp.push((v & 0x7f) | 0x80);
      v >>>= 7;
    }
    for (let i = temp.length - 1; i >= 0; i--) out.push(temp[i]);
  }

  function pushBE16(out, value) {
    out.push((value >> 8) & 0xff, value & 0xff);
  }

  function pushBE32(out, value) {
    out.push(
      (value >>> 24) & 0xff,
      (value >>> 16) & 0xff,
      (value >>> 8) & 0xff,
      value & 0xff,
    );
  }

  function clamp(value, lo, hi) {
    return value < lo ? lo : value > hi ? hi : value;
  }

  function writeSMF(events, tempo, ppq) {
    if (typeof ppq !== 'number') ppq = TICKS_PER_QUARTER;
    const data = [];

    data.push(0x4d, 0x54, 0x68, 0x64);
    pushBE32(data, 6);
    pushBE16(data, 0);
    pushBE16(data, 1);
    pushBE16(data, ppq);

    const track = [];

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

  function downloadSMF(bytes, filename) {
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

  window.PolySmfWriter = {
    TICKS_PER_QUARTER,
    writeVLQ,
    writeSMF,
    downloadSMF,
  };
})();
