'use strict';
/**
 * Pure groove math shared by hosts and the UI renderer.
 * Mirrors engine semantics (euclidean.cpp Bresenham form; deterministic
 * position-hash velocity shading echoing rng.h).
 */
(function () {
  function euclid(k, n) {
    const p = [];
    for (let i = 0; i < n; i++) p.push(Math.floor((i * k) / n) !== Math.floor(((i - 1) * k) / n) ? 1 : 0);
    if (k > 0) p[0] = 1;
    return p;
  }
  function rotArr(a, r) {
    const n = a.length;
    return a.map((_, i) => a[(((i - r) % n) + n) % n]);
  }
  const cyc8 = (l) => (l.cells ? l.cells.reduce((a, b) => a + b, 0) : l.steps * l.stepLen);
  const onsets = (l) => {
    const o = [];
    let a = 0;
    for (const c of l.cells) {
      o.push(a);
      a += c;
    }
    return o;
  };
  function shade(li, tick) {
    let h = ((li * 2654435761) ^ (tick * 40503)) >>> 0;
    h = ((h ^ (h >>> 13)) * 0x5bd1e995) >>> 0;
    return ((h >>> 8) % 1000) / 1000;
  }
  function envVelFactor(l, tick) {
    let f = 1;
    for (const e of l.envs) {
      if (!e.on || e.target !== 'Velocity') continue;
      f *= 1 + e.depth * Math.sin((2 * Math.PI * (tick / 12)) / e.period);
    }
    return Math.max(0.15, Math.min(1.6, f));
  }
  function laneHitAt(l, tick) {
    const cyc = cyc8(l);
    const tin = ((tick % cyc) + cyc) % cyc;
    if (l.cells) {
      const os = onsets(l);
      const idx = os.indexOf(tin);
      return idx < 0 ? null : { step: idx, acc: idx === 0 };
    }
    if (tick % l.stepLen) return null;
    const step = (tick / l.stepLen) % l.steps;
    return l.pattern[step] ? { step, acc: step === 0 } : null;
  }
  function hitVelocity(l, li, tick, hit) {
    let vel = l.vel / 127;
    if (l.spread) vel *= 1 - l.spread * 1.5 + l.spread * 3 * shade(li, tick);
    if (l.ghost) vel *= hit.step === 0 ? 1 : 0.55 + 0.2 * shade(li, tick);
    if (l.timeline && (hit.step === 0 || hit.step === 7)) vel *= 1.12;
    if (l.cells) vel *= hit.acc ? 1.1 : 0.8;
    vel *= envVelFactor(l, tick);
    return Math.min(1, vel);
  }
  window.PolyGrooveMath = { euclid, rotArr, cyc8, onsets, shade, envVelFactor, laneHitAt, hitVelocity };
})();
