'use strict';
/**
 * Pure groove math shared by hosts and the UI renderer.
 * Mirrors engine semantics (euclidean.cpp Bresenham form; deterministic
 * position-hash velocity shading echoing rng.h).
 */
(function () {
  // M073 S02: preview mirror of the engine's deterministic explicit-accent boost
  // (engine.cpp kAccentVelocityBoost). One numerically-identical proportional-
  // headroom formula on both sides: vel += accentVal * ACCENT_VELOCITY_BOOST *
  // (1 - vel). Always boosts a set step, seed-independent, and never saturates a
  // mid-velocity lane (result stays strictly below 1 for vel < 1). The engine's
  // separate probabilistic-emphasis layer is RNG-driven and not modelled by this
  // decorative preview.
  const ACCENT_VELOCITY_BOOST = 0.6;
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
  // Pure mirror of engine.cpp computeDriftedCycleStep (D009): the drift the
  // engine applies to a lane's cycle step is floor(barPos * driftRate) reduced
  // into [0, steps). Returning just the offset (not the drifted step) lets the
  // ring rotate by driftOffset/steps*360deg and the playhead land on
  // (cycleStep + driftOffset) % steps — algebraically identical to the engine's
  // (cycleStep + driftSteps) mod stepsInCycle. barPos is ppq / kPpqPerBar (4.0).
  // steps mirrors ctx.stepsInCycle: cells.length for additive lanes, else steps.
  function driftSteps(l) {
    return l.cells ? l.cells.length : l.steps;
  }
  function driftOffset(l, barPos) {
    const steps = driftSteps(l);
    const rate = l.driftRate || 0;
    if (!steps || rate === 0) return 0;
    const off = Math.floor(barPos * rate);
    return ((off % steps) + steps) % steps;
  }
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
    // baseVelocity 0 mutes the lane entirely (mirrors engine classifyStep's
    // Silent short-circuit, M073 S01): overrides ghost/spread/env shaping so a
    // zero-velocity lane draws no hit bar. Any nonzero vel falls through unchanged.
    if (l.vel === 0) return 0;
    let vel = l.vel / 127;
    if (l.spread) vel *= 1 - l.spread * 1.5 + l.spread * 3 * shade(li, tick);
    // Deterministic explicit accent: mirrors engine computeStepVelocity — applied
    // to the base+spread velocity, unconditionally boosting a set step by the
    // proportional-headroom amount so toggling the accent visibly raises the drawn
    // hit bar. accents is index-aligned to cycle steps (hit.step); absent/zero => no boost.
    const accentVal = l.accents ? l.accents[hit.step] || 0 : 0;
    if (accentVal > 0) vel += accentVal * ACCENT_VELOCITY_BOOST * (1 - vel);
    if (l.ghost) vel *= hit.step === 0 ? 1 : 0.55 + 0.2 * shade(li, tick);
    if (l.timeline && (hit.step === 0 || hit.step === 7)) vel *= 1.12;
    if (l.cells) vel *= hit.acc ? 1.1 : 0.8;
    vel *= envVelFactor(l, tick);
    return Math.min(1, vel);
  }
  window.PolyGrooveMath = { euclid, rotArr, cyc8, onsets, driftOffset, shade, envVelFactor, laneHitAt, hitVelocity };
})();
