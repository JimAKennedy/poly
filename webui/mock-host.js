'use strict';
/**
 * Mock host: browser-standalone PolyHost implementation.
 * Owns the groove model and a WebAudio preview engine so the UI can be
 * developed, demoed, and CI-tested without the plugin. Provides all 14
 * factory presets (matching presets.cpp) with representative lane data
 * for the most distinct presets.
 */
(function () {
  const { euclid, rotArr, cyc8, onsets, laneHitAt, hitVelocity } = window.PolyGrooveMath;

  const HUES = ['#F5B54A', '#E4604E', '#5AC8DA', '#9BC46B', '#B48AE0', '#E8A33D', '#4EBBE0', '#D47AB8'];

  function mkLane(name, role, note, ch, steps, stepLen, hits, vel, opts = {}) {
    const subdivision = opts.subdivision ?? Math.round(8 / stepLen);
    const l = {
      name, role, note, ch, steps, subdivision, stepLen, vel, prob: opts.prob ?? 1.0,
      spread: opts.spread ?? 0, ghost: opts.ghost ?? 0, push: opts.push ?? 0,
      hits, rot: opts.rot ?? 0, timeline: opts.timeline ?? false,
      fixed: opts.fixed ?? null,
      pattern: opts.fixed ? opts.fixed.slice() : rotArr(euclid(hits, steps), opts.rot ?? 0),
      cells: opts.cells ?? null, cellCount: opts.cells ? opts.cells.length : 0,
      mt: new Array(steps).fill(0),
      envs: opts.envs ?? [], hue: HUES[(ch - 1) % 8],
      active: opts.active ?? true,
      humanize: opts.humanize ?? 0, swing: opts.swing ?? 0,
      duration: opts.duration ?? 0.5, emphasisProb: opts.emphasisProb ?? 0,
      timingOffset: opts.timingOffset ?? 0,
      mutationRate: opts.mutationRate ?? 0, driftRate: opts.driftRate ?? 0,
      phraseLength: opts.phraseLength ?? 0, phraseGap: opts.phraseGap ?? 0,
      phraseOffset: opts.phraseOffset ?? 0,
      tempoMultiplier: opts.tempoMultiplier ?? 1.0,
      kotekanSource: opts.kotekanSource ?? -1,
      accents: new Array(steps).fill(0),
    };
    if (opts.mt) opts.mt.forEach((v, i) => { l.mt[i] = v; });
    return l;
  }

  const PRESETS = [
    { name: 'Four on the Floor', description: 'Classic club groove with straight 8th hats and polymetric open hat' },
    { name: 'Polymetric Drift', description: 'Prime-number cycles (3, 5, 7, 11) creating evolving phase patterns' },
    { name: 'Sparse Pulse', description: 'Minimal, spacious groove with wide spacing and gentle ghost notes' },
    { name: 'Breakbeat', description: 'Syncopated kick with punchy snare, fast hats and ghost toms' },
    { name: 'Latin Feel', description: 'Clave-inspired pattern with conga, shaker and cowbell ornaments' },
    { name: 'Afro-House Phrases', description: 'Offset phrase loops — shaker continuous, conga and djembe breathe on staggered cycles' },
    { name: 'Reich Phasing', description: 'Two identical patterns gradually phase apart creating emergent resultant rhythms' },
    { name: 'Kotekan Interlock', description: 'Balinese interlocking pair — polos and sangsih fill each other\'s gaps' },
    { name: 'Pocket Groove', description: 'J Dilla-style micro-timing — kick pushes late, snare pulls early, gentle mutation' },
    { name: 'Afrobeat 12/8', description: 'Compound-time groove with timeline bell pattern, four-on-the-floor kick and conga ghosts' },
    { name: 'Balkan Aksak', description: '7/8 aksak [2+2+3] additive cells — davul, rim, zurna and darbuka' },
    { name: 'Bossa Nova', description: 'Clave timeline with ginga micro-timing — surdo, tamborim, agogo and pandeiro' },
    { name: 'Carnatic Tala', description: 'Adi tala [4+2+2] additive cells — mridangam, ghatam and kanjira' },
    { name: 'IDM Glitch', description: 'Irregular additive cells with heavy mutation and erratic micro-timing offsets' },
  ];

  const PRESET_NAMES = [
    ['Kick', 'Snare', 'Hi-Hat', 'Open Hat'],
    ['Kick', 'Rim', 'Tom', 'Hi-Hat'],
    ['Kick', 'Rim', 'Ghost'],
    ['Kick', 'Snare', 'Hi-Hat', 'Ghost Tom'],
    ['Clave', 'Conga', 'Shaker', 'Cowbell'],
    ['Kick', 'Shaker', 'Conga', 'Djembe', 'Perc'],
    ['Fixed', 'Drifting', 'Pulse'],
    ['Polos', 'Sangsih', 'Gong', 'Shimmer'],
    ['Kick', 'Snare', 'Hi-Hat', 'Ghost'],
    ['Bell', 'Kick', 'Snare', 'Shaker', 'Conga'],
    ['Davul', 'Rim', 'Zurna', 'Darbuka'],
    ['Surdo', 'Tamborim', 'Agogo', 'Pandeiro'],
    ['Mrid Bass', 'Mrid Treble', 'Ghatam', 'Kanjira'],
    ['Kick', 'Snare', 'Hi-Hat', 'Perc', 'Glitch'],
  ];

  function makePresetLanes(index) {
    switch (index) {
      case 0: return [ // Four on the Floor — 4 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 4, 2, 4, 110),
        mkLane('Snare', 'Backbeat', 38, 2, 4, 2, 2, 100),
        mkLane('Hi-Hat', 'Shimmer', 42, 3, 8, 1, 8, 80, { prob: 0.95, spread: 0.08 }),
        mkLane('Open Hat', 'Ghost', 46, 4, 7, 1, 4, 60, { prob: 0.7, ghost: 25, spread: 0.12 }),
      ];
      case 2: return [ // Sparse Pulse — 3 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 2, 4, 1, 100),
        mkLane('Rim', 'Ornament', 37, 2, 3, 1, 2, 70, { prob: 0.8 }),
        mkLane('Ghost', 'Ghost', 39, 3, 5, 1, 2, 45, { prob: 0.6, ghost: 20, spread: 0.2 }),
      ];
      case 6: return [ // Reich Phasing — 3 lanes
        mkLane('Fixed', 'Anchor pulse', 76, 1, 5, 1, 3, 90),
        mkLane('Drifting', 'Anchor pulse', 76, 2, 5, 1, 3, 90),
        mkLane('Pulse', 'Shimmer', 42, 3, 4, 2, 4, 45, { prob: 0.8, spread: 0.05 }),
      ];
      case 9: return [ // Afrobeat 12/8 — 5 lanes (default)
        mkLane('Bell', 'Anchor pulse', 56, 1, 12, 1, 7, 90, {
          timeline: true, fixed: [1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1],
          envs: [{ target: 'Velocity', period: 4, depth: 0.35, on: true }],
        }),
        mkLane('Kick', 'Backbeat', 36, 2, 4, 2, 4, 112, { mt: [3, 0, 2, 0] }),
        mkLane('Snare', 'Accent', 38, 3, 3, 1, 2, 95, {
          prob: 0.9, mt: [0, -2, 0],
          envs: [{ target: 'Probability', period: 7, depth: 0.5, on: false }],
        }),
        mkLane('Shaker', 'Shimmer', 70, 4, 12, 1, 12, 55, {
          prob: 0.9, spread: 0.12,
          envs: [{ target: 'Velocity', period: 7, depth: 0.45, on: true }],
        }),
        mkLane('Conga', 'Ghost', 63, 5, 5, 1, 3, 65, { prob: 0.8, ghost: 25, mt: [0, 1, -1, 0, 1] }),
      ];
      case 13: return [ // IDM Glitch — 5 lanes with cells
        mkLane('Kick', 'Anchor pulse', 36, 1, 4, 4, 3, 115, { prob: 0.9, cells: [3, 2, 5, 3] }),
        mkLane('Snare', 'Backbeat', 38, 2, 5, 1, 2, 100, { prob: 0.75, spread: 0.2 }),
        mkLane('Hi-Hat', 'Shimmer', 42, 3, 9, 1, 6, 70, { prob: 0.85, spread: 0.18 }),
        mkLane('Perc', 'Ghost', 45, 4, 7, 1, 3, 55, { prob: 0.6, ghost: 15, spread: 0.25 }),
        mkLane('Glitch', 'Ornament', 56, 5, 11, 1, 4, 75, { prob: 0.7 }),
      ];
      default: {
        const names = PRESET_NAMES[index] || PRESET_NAMES[0];
        const roles = ['Anchor pulse', 'Backbeat', 'Shimmer', 'Ghost', 'Ornament', 'Ghost', 'Fill', 'Custom'];
        const notes = [36, 38, 42, 45, 46, 39, 43, 50];
        const steps = [4, 4, 8, 5, 7, 3, 6, 9];
        const hits =  [4, 2, 6, 3, 4, 2, 4, 5];
        return names.map((n, i) =>
          mkLane(n, roles[i] || 'Custom', notes[i] || 36, i + 1, steps[i] || 4, 1, hits[i] || 2, 80 + (i === 0 ? 30 : 0), { prob: 0.9 })
        );
      }
    }
  }

  const PRESET_MACROS = [
    { complexity: 0.5, density: 0.5, syncopation: 0, swing: 0, tension: 0, humanize: 0 },
    { complexity: 0.7, density: 0.4, syncopation: 0, swing: 0, tension: 0.3, humanize: 0 },
    { complexity: 0, density: 0.25, syncopation: 0, swing: 0, tension: 0, humanize: 0.3 },
    { complexity: 0.6, density: 0, syncopation: 0.5, swing: 0, tension: 0.4, humanize: 0 },
    { complexity: 0.4, density: 0, syncopation: 0, swing: 0.3, tension: 0, humanize: 0.2 },
    { complexity: 0, density: 0.45, syncopation: 0, swing: 0.15, tension: 0, humanize: 0.15 },
    { complexity: 0.2, density: 0.3, syncopation: 0, swing: 0, tension: 0, humanize: 0 },
    { complexity: 0.3, density: 0.4, syncopation: 0, swing: 0, tension: 0, humanize: 0 },
    { complexity: 0, density: 0.4, syncopation: 0.3, swing: 0, tension: 0, humanize: 0.25 },
    { complexity: 0.45, density: 0.5, syncopation: 0.3, swing: 0.1, tension: 0.25, humanize: 0.15 },
    { complexity: 0.4, density: 0.45, syncopation: 0, swing: 0, tension: 0, humanize: 0 },
    { complexity: 0.35, density: 0, syncopation: 0, swing: 0.2, tension: 0, humanize: 0.2 },
    { complexity: 0.5, density: 0.4, syncopation: 0, swing: 0, tension: 0, humanize: 0 },
    { complexity: 0.8, density: 0.35, syncopation: 0.5, swing: 0, tension: 0.6, humanize: 0 },
  ];

  const PRESET_SEEDS = [1, 7, 23, 42, 13, 31, 47, 55, 71, 88, 33, 17, 44, 99];

  const TEMPO = 126;
  const EIGHTH = 60 / TEMPO / 2;
  const CONV = 120;

  function identityNoteMap() {
    const m = new Array(128);
    for (let i = 0; i < 128; i++) m[i] = i;
    return m;
  }

  const state = {
    preset: 'Afrobeat 12/8', seed: 88, tempo: TEMPO,
    scene: 'A', morph: 0,
    chain: { enabled: false, mode: 0, entryCount: 0, entries: [] },
    macros: { complexity: 0.45, density: 0.5, syncopation: 0.3, swing: 0.1, tension: 0.25, humanize: 0.15 },
    lanes: makePresetLanes(9),
    presets: PRESETS,
    noteMap: identityNoteMap(),
  };

  const stateSubs = [];
  const frameSubs = [];
  let asyncMode = false;
  let pendingPush = false;

  const emitState = () => {
    if (asyncMode) { pendingPush = true; return; }
    stateSubs.forEach((cb) => cb(state));
  };

  function flushState() {
    if (pendingPush) {
      pendingPush = false;
      stateSubs.forEach((cb) => cb(state));
    }
  }

  function applyPreset(index) {
    if (index === -1) {
      state.preset = 'Init';
      state.seed = 0;
      state.scene = 'A';
      state.morph = 0;
      state.chain = { enabled: false, mode: 0, entryCount: 0, entries: [] };
      state.macros = { complexity: 0, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0 };
      state.lanes = makePresetLanes(0);
      state.noteMap = identityNoteMap();
      return;
    }
    if (index < 0 || index >= PRESETS.length) return;
    state.preset = PRESETS[index].name;
    state.seed = PRESET_SEEDS[index] ?? 0;
    state.macros = { ...PRESET_MACROS[index] };
    state.lanes = makePresetLanes(index);
  }

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
      state.lanes.forEach((l, li) => {
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
      lanes: state.lanes.map((l) => {
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
        state.scene = payload.scene === 'Morph' ? 'Morph' : (payload.scene === 'B' ? 'B' : 'A');
        break;
      case 'chainAddEntry': {
        if (state.chain.entryCount < 16) {
          state.chain.entries.push({ scene: 0, bars: 4 });
          state.chain.entryCount++;
        }
        break;
      }
      case 'chainRemoveEntry': {
        const ci = payload.index;
        if (ci >= 0 && ci < state.chain.entryCount) {
          state.chain.entries.splice(ci, 1);
          state.chain.entryCount--;
        }
        break;
      }
      case 'applyPreset':
        applyPreset(payload.index ?? -1);
        break;
      case 'setAccent': {
        const al = state.lanes[payload.lane];
        if (al && payload.step >= 0 && payload.step < al.accents.length)
          al.accents[payload.step] = payload.value;
        break;
      }
      case 'setNoteMap': {
        const ni = payload.note;
        if (ni >= 0 && ni < 128) state.noteMap[ni] = Math.max(0, Math.min(127, payload.output));
        break;
      }
      case 'resetNoteMap':
        state.noteMap = identityNoteMap();
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
      if (gesture !== 'perform') return;

      if (paramId.startsWith('macro.')) {
        state.macros[paramId.slice(6)] = value;
        emitState();
        return;
      }

      if (paramId === 'scene.morph') {
        state.morph = value;
        emitState();
        return;
      }

      if (paramId === 'chain.enabled') {
        state.chain.enabled = value >= 0.5;
        emitState();
        return;
      }
      if (paramId === 'chain.mode') {
        state.chain.mode = Math.round(value * 2);
        emitState();
        return;
      }

      const cm = paramId.match(/^chain\.entry\.(\d+)\.(scene|bars)$/);
      if (cm) {
        const ei = parseInt(cm[1]);
        if (ei < state.chain.entryCount) {
          if (cm[2] === 'scene') state.chain.entries[ei].scene = Math.round(value * 2);
          else state.chain.entries[ei].bars = Math.round(value * 31) + 1;
        }
        emitState();
        return;
      }
      if (paramId === 'seed') {
        state.seed = Math.round(value * 999999);
        emitState();
        return;
      }

      const m = paramId.match(/^lane\.(\d+)\.(.+)$/);
      if (m) {
        const lane = state.lanes[parseInt(m[1])];
        if (!lane) return;
        const field = m[2];
        const LANE_EDITS = {
          velocity:     v => { lane.vel = Math.round(v * 127); },
          probability:  v => { lane.prob = v; },
          emphasisProb: v => { lane.emphasisProb = v; },
          ghostFloor:   v => { lane.ghost = Math.round(v * 127); },
          spread:       v => { lane.spread = v; },
          swing:        v => { lane.swing = v; },
          humanize:     v => { lane.humanize = v * 50; },
          duration:     v => { lane.duration = v * 4; },
          active:       v => { lane.active = v >= 0.5; },
          phraseLength: v => { lane.phraseLength = v * 64; },
          phraseGap:    v => { lane.phraseGap = v * 64; },
          phraseOffset: v => { lane.phraseOffset = v * 64; },
          mutationRate: v => { lane.mutationRate = v; },
          driftRate:    v => { lane.driftRate = v * 8 - 4; },
          timingOffset: v => { lane.timingOffset = v * 40 - 20; },
          kotekanSource:v => { lane.kotekanSource = Math.round(v * 8) - 1; },
          note:         v => { lane.note = Math.round(v * 127); },
          channel:      v => { lane.ch = Math.round(v * 15) + 1; },
          steps:        v => {
            lane.steps = Math.round(v * 63) + 1;
            lane.hits = Math.min(lane.hits, lane.steps);
            lane.pattern = rotArr(euclid(lane.hits, lane.steps), lane.rot);
            lane.mt = new Array(lane.steps).fill(0);
            lane.accents = new Array(lane.steps).fill(0);
          },
          hits:         v => {
            lane.hits = Math.min(Math.round(v * 64), lane.steps);
            lane.pattern = rotArr(euclid(lane.hits, lane.steps), lane.rot);
          },
          rotation:     v => {
            lane.rot = Math.round(v * 63) % lane.steps;
            lane.pattern = rotArr(euclid(lane.hits, lane.steps), lane.rot);
          },
          subdivision:  v => {
            const subs = [1, 2, 4, 8, 16];
            lane.subdivision = subs[Math.round(v * 4)] || 4;
            lane.stepLen = 8 / lane.subdivision;
          },
          tempoMult:    v => { lane.tempoMultiplier = v; },
          cellCount:    v => {
            const count = Math.round(v * 64);
            if (count > 0 && !lane.cells) lane.cells = new Array(count).fill(2);
            else if (count === 0) lane.cells = null;
            lane.cellCount = count;
          },
          timeline:     v => { lane.timeline = v >= 0.5; },
        };
        if (LANE_EDITS[field]) {
          LANE_EDITS[field](value);
          emitState();
        }
      }
    },
    action,
    onFrame: (cb) => frameSubs.push(cb),
    _pushState: emitState,
    setAsyncMode: (enabled) => { asyncMode = !!enabled; pendingPush = false; },
    flushState,
    hasPendingPush: () => pendingPush,
  };
})();
