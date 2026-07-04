'use strict';
/**
 * Mock host: browser-standalone PolyHost implementation.
 * Owns the groove model and a WebAudio preview engine so the UI can be
 * developed, demoed, and CI-tested without the plugin. Ships the Afrobeat
 * 12/8 factory preset (presets.cpp makeAfrobeat12_8) as its model.
 */
(function () {
  const { euclid, rotArr, cyc8, onsets, laneHitAt, hitVelocity } = window.PolyGrooveMath;

  const LANES = [
    { name: 'Bell', role: 'Anchor pulse', note: 56, ch: 1, steps: 12, stepLen: 1, vel: 90, prob: 1.0, spread: 0, ghost: 0, push: 0, hits: 7, rot: 0, timeline: true,
      fixed: [1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1], pattern: [1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1], cells: null,
      envs: [{ target: 'Velocity', period: 4, depth: 0.35, on: true }], hue: '#F5B54A' },
    { name: 'Kick', role: 'Backbeat', note: 36, ch: 2, steps: 4, stepLen: 2, vel: 112, prob: 1.0, spread: 0, ghost: 0, push: 3, hits: 4, rot: 0, timeline: false,
      fixed: null, pattern: euclid(4, 4), cells: null, envs: [], hue: '#E4604E' },
    { name: 'Snare', role: 'Accent', note: 38, ch: 3, steps: 3, stepLen: 1, vel: 95, prob: 0.9, spread: 0, ghost: 0, push: -2, hits: 2, rot: 0, timeline: false,
      fixed: null, pattern: euclid(2, 3), cells: null, envs: [{ target: 'Probability', period: 7, depth: 0.5, on: false }], hue: '#5AC8DA' },
    { name: 'Shaker', role: 'Shimmer', note: 70, ch: 4, steps: 12, stepLen: 1, vel: 55, prob: 0.9, spread: 0.12, ghost: 0, push: 0, hits: 12, rot: 0, timeline: false,
      fixed: null, pattern: euclid(12, 12), cells: null, envs: [{ target: 'Velocity', period: 7, depth: 0.45, on: true }], hue: '#9BC46B' },
    { name: 'Conga', role: 'Ghost', note: 63, ch: 5, steps: 5, stepLen: 1, vel: 65, prob: 0.8, spread: 0, ghost: 25, push: 0, hits: 3, rot: 0, timeline: false,
      fixed: null, pattern: euclid(3, 5), cells: null, envs: [], hue: '#B48AE0' },
  ];
  LANES.forEach((l) => { l.mt = new Array(l.steps).fill(0); });
  LANES[1].mt = [3, 0, 2, 0];
  LANES[2].mt = [0, -2, 0];
  LANES[4].mt = [0, 1, -1, 0, 1];

  const TEMPO = 126;
  const EIGHTH = 60 / TEMPO / 2;
  const CONV = 120;

  const state = {
    preset: 'Afrobeat 12/8', seed: 88, tempo: TEMPO,
    scene: 'A', morph: 0,
    macros: { complexity: 0.45, density: 0.5, syncopation: 0.3, swing: 0.1, tension: 0.25, humanize: 0.15 },
    lanes: LANES,
  };

  const stateSubs = [];
  const frameSubs = [];
  const emitState = () => stateSubs.forEach((cb) => cb(state));

  /* ---------- WebAudio preview voices ---------- */
  let ctx = null, playing = false, startAt = 0, schedTimer = null, nextTick = 0, _nb = null;
  const now8 = () => (playing && ctx ? (ctx.currentTime - startAt) / EIGHTH : 0);
  function noiseBuf() {
    if (_nb) return _nb;
    _nb = ctx.createBuffer(1, ctx.sampleRate * 0.2, ctx.sampleRate);
    const d = _nb.getChannelData(0);
    for (let i = 0; i < d.length; i++) d[i] = Math.random() * 2 - 1;
    return _nb;
  }
  function voice(l, when, v) {
    v *= 0.5;
    if (l.name === 'Kick') {
      const o = ctx.createOscillator(), g = ctx.createGain();
      o.type = 'sine';
      o.frequency.setValueAtTime(120, when);
      o.frequency.exponentialRampToValueAtTime(44, when + 0.11);
      g.gain.setValueAtTime(v, when);
      g.gain.exponentialRampToValueAtTime(0.001, when + 0.24);
      o.connect(g); g.connect(ctx.destination); o.start(when); o.stop(when + 0.26);
    } else if (l.name === 'Bell') {
      [562, 845].forEach((f, i) => {
        const o = ctx.createOscillator(), g = ctx.createGain(), bp = ctx.createBiquadFilter();
        o.type = 'square'; o.frequency.value = f;
        bp.type = 'bandpass'; bp.frequency.value = f; bp.Q.value = 4;
        g.gain.setValueAtTime(v * (i ? 0.4 : 0.62), when);
        g.gain.exponentialRampToValueAtTime(0.001, when + 0.12);
        o.connect(bp); bp.connect(g); g.connect(ctx.destination); o.start(when); o.stop(when + 0.14);
      });
    } else if (l.name === 'Snare') {
      const s = ctx.createBufferSource(), g = ctx.createGain(), hp = ctx.createBiquadFilter();
      s.buffer = noiseBuf(); hp.type = 'highpass'; hp.frequency.value = 1400;
      g.gain.setValueAtTime(v * 0.8, when);
      g.gain.exponentialRampToValueAtTime(0.001, when + 0.14);
      s.connect(hp); hp.connect(g); g.connect(ctx.destination); s.start(when); s.stop(when + 0.15);
    } else if (l.name === 'Shaker') {
      const s = ctx.createBufferSource(), g = ctx.createGain(), hp = ctx.createBiquadFilter();
      s.buffer = noiseBuf(); hp.type = 'highpass'; hp.frequency.value = 6200;
      g.gain.setValueAtTime(v * 0.55, when);
      g.gain.exponentialRampToValueAtTime(0.001, when + 0.055);
      s.connect(hp); hp.connect(g); g.connect(ctx.destination); s.start(when); s.stop(when + 0.06);
    } else {
      const o = ctx.createOscillator(), g = ctx.createGain();
      o.type = 'sine';
      o.frequency.setValueAtTime(340, when);
      o.frequency.exponentialRampToValueAtTime(255, when + 0.07);
      g.gain.setValueAtTime(v * 0.6, when);
      g.gain.exponentialRampToValueAtTime(0.001, when + 0.12);
      o.connect(g); g.connect(ctx.destination); o.start(when); o.stop(when + 0.13);
    }
  }
  function schedule() {
    const ahead = ctx.currentTime + 0.14;
    while (startAt + nextTick * EIGHTH < ahead) {
      LANES.forEach((l, li) => {
        const hit = laneHitAt(l, nextTick);
        if (!hit) return;
        const vel = hitVelocity(l, li, nextTick, hit);
        const mtMs = (l.mt && l.mt[hit.step]) || 0;
        const t = startAt + nextTick * EIGHTH + (l.push + mtMs) / 1000;
        voice(l, Math.max(ctx.currentTime + 0.001, t), vel);
      });
      nextTick++;
    }
  }
  function togglePlay() {
    try {
      if (!ctx) ctx = new (window.AudioContext || window.webkitAudioContext)();
      if (ctx.state === 'suspended') ctx.resume();
    } catch (e) {
      ctx = null; // headless CI: keep transport state without audio
    }
    playing = !playing;
    if (playing) {
      startAt = ctx ? ctx.currentTime + 0.06 : 0;
      nextTick = 0;
      if (ctx) schedTimer = setInterval(schedule, 30);
    } else {
      clearInterval(schedTimer);
    }
  }

  /* ---------- frame pump ---------- */
  function pump() {
    const t8 = now8();
    const convLeft = playing ? (CONV - (Math.floor(t8) % CONV)) % CONV || CONV : CONV;
    const frame = {
      t8,
      playing,
      convLeft,
      lanes: LANES.map((l) => {
        const cyc = cyc8(l);
        const tin = Math.floor(t8 % cyc);
        let step;
        if (l.cells) {
          const os = onsets(l);
          step = 0;
          for (let i = os.length - 1; i >= 0; i--)
            if (tin >= os[i]) { step = i; break; }
        } else {
          step = Math.floor(tin / l.stepLen);
        }
        return { ph: (t8 / cyc) % 1, step };
      }),
    };
    frameSubs.forEach((cb) => cb(frame));
    requestAnimationFrame(pump);
  }
  requestAnimationFrame(pump);

  /* ---------- actions ---------- */
  function action(name, payload = {}) {
    const l = state.lanes[payload.lane];
    switch (name) {
      case 'togglePlay':
        togglePlay();
        break;
      case 'toggleStep':
        l.pattern[payload.step] = l.pattern[payload.step] ? 0 : 1;
        l.hits = l.pattern.filter(Boolean).length;
        break;
      case 'setEuclid': {
        if (payload.steps !== undefined) {
          l.steps = Math.max(2, Math.min(16, payload.steps));
          l.hits = Math.min(l.hits, l.steps);
          l.mt = new Array(l.steps).fill(0);
        }
        if (payload.hits !== undefined) l.hits = Math.max(0, Math.min(l.steps, payload.hits));
        if (payload.rotation !== undefined) l.rot = ((payload.rotation % l.steps) + l.steps) % l.steps;
        l.pattern = rotArr(euclid(l.hits, l.steps), l.rot);
        break;
      }
      case 'setCells':
        l.cells = payload.cells ? payload.cells.map((c) => Math.max(1, Math.min(4, c))) : null;
        l.mt = new Array(l.cells ? l.cells.length : l.steps).fill(0);
        break;
      case 'setFixedStep':
        if (l.fixed) {
          l.fixed[payload.step] = payload.on ? 1 : 0;
          l.pattern = l.fixed.slice();
        }
        break;
      case 'setMicroTiming':
        l.mt[payload.step] = Math.max(-20, Math.min(20, payload.ms));
        break;
      case 'setEnvelope':
        if (payload.envelope === null) l.envs.splice(payload.index, 1);
        else if (payload.index >= l.envs.length) l.envs.push(payload.envelope);
        else l.envs[payload.index] = payload.envelope;
        break;
      case 'selectScene':
        state.scene = payload.scene === 'B' ? 'B' : 'A';
        break;
      case 'exportRequest':
        console.info('[mock-host] exportRequest — native host runs the SMF export path here');
        break;
      default:
        console.warn('[mock-host] unknown action', name, payload);
        return;
    }
    if (name !== 'togglePlay') emitState();
  }

  window.PolyMockHost = {
    schemaVersion: window.POLY_SCHEMA_VERSION,
    getState: () => state,
    onState: (cb) => stateSubs.push(cb),
    edit: (paramId, value, gesture) => {
      // The mock keeps continuous params in the model directly; gestures are
      // logged so gesture-correctness is visible during development.
      if (gesture === 'perform' && paramId.startsWith('macro.')) {
        state.macros[paramId.slice(6)] = value;
        emitState();
      }
    },
    action,
    onFrame: (cb) => frameSubs.push(cb),
    _pushState: emitState,
  };
})();
