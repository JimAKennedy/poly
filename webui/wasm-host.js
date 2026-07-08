'use strict';
/**
 * WASM host: PolyHost implementation backed by the real C++ engine compiled
 * to WebAssembly. Uses poly_render() for audio event generation and reads
 * back lane state via poly_lane_int/float() after mutations so the UI stays
 * in sync with the sanitized engine state.
 *
 * WebAudio voices are synthesized (same as mock-host). The bridge protocol
 * is identical — ui.js doesn't know or care which host is active.
 */
(function () {
  const { euclid, rotArr, cyc8, onsets, laneHitAt, hitVelocity } = window.PolyGrooveMath;

  const HUES = ['#F5B54A', '#E4604E', '#5AC8DA', '#9BC46B', '#B48AE0', '#E8A33D', '#4EBBE0', '#D47AB8'];
  const ROLES = ['Anchor pulse', 'Backbeat', 'Shimmer', 'Accent', 'Ghost', 'Ornament', 'Fill', 'Custom'];
  const ENV_TARGETS = ['Velocity', 'Density', 'Probability', 'AccentBias', 'NoteLength', 'TimingLooseness', 'ActivationWeight', 'FillLikelihood'];

  const LaneFieldInt = {
    MidiNote: 0, MidiChannel: 1, HitCount: 2, Rotation: 3, BaseVelocity: 4,
    GhostFloor: 5, Active: 6, Subdivision: 7, CycleSteps: 8, KotekanSource: 9,
    Timeline: 10, EnvelopeCount: 11, CellCount: 12,
  };

  const LaneFieldFloat = {
    Probability: 0, VelocitySpread: 1, HumanizeMs: 2, SwingAmount: 3,
    NoteDuration: 4, PhraseLength: 5, PhraseGap: 6, PhraseOffset: 7,
    MutationRate: 8, DriftRate: 9, TimingOffsetMs: 10, SyncopationOffset: 11,
    TempoMultiplier: 12, EmphasisProb: 13,
  };

  let Module = null;
  let engineCtx = null;

  let TEMPO = 126;
  const eighthSec = () => 60 / TEMPO / 2;
  const CONV = 120;

  const stateSubs = [];
  const frameSubs = [];
  let asyncMode = false;
  let pendingPush = false;

  function identityNoteMap() {
    const m = new Array(128);
    for (let i = 0; i < 128; i++) m[i] = i;
    return m;
  }

  // Engine-emitted preset lane metadata (M043 S11). Populated at boot from
  // /poly/webui/presets.json — same file the site card consumes — so lane
  // labels, MIDI notes, and role hints never drift from the C++ engine.
  // presetsById[index] = { name, lanes: [{ noteNumber, roleLabel, ... }] }.
  let presetsById = null;

  // Human-readable per-lane labels derived from GM role: what the modal
  // shows above each strip. Falls back to "Lane N" until presets.json loads.
  const ROLE_LABEL_TO_NAME = {
    kick: 'Kick', snare: 'Snare', hat: 'Hi-hat', openhat: 'Open Hat',
    clap: 'Clap', ghost: 'Ghost', clave: 'Clave', woodblock: 'Woodblock',
    cowbell: 'Cowbell', tom: 'Tom', shaker: 'Shaker', tambourine: 'Tambourine',
    conga: 'Conga', bongo: 'Bongo', cajon: 'Cajon', darbuka: 'Darbuka',
    handclap: 'Handclap', castanets: 'Castanets', agogo: 'Agogo',
  };
  const ENGINE_ROLE_TO_UI = {
    AnchorPulse: 'Anchor pulse', Backbeat: 'Backbeat',
    Shimmer: 'Shimmer', Accent: 'Accent', Ghost: 'Ghost',
    Ornament: 'Ornament', Fill: 'Fill', Custom: 'Custom',
  };

  let currentPresetIndex = 9;
  let laneNames = ['Lane 1', 'Lane 2', 'Lane 3', 'Lane 4'];
  let laneRoles = ['Anchor pulse', 'Backbeat', 'Shimmer', 'Ghost'];
  // Role hint per lane, sourced from presets.json for sample resolution.
  // Cleared on init/apply of -1 so unknown lanes fall through to NAME_TO_ROLE.
  let laneRoleLabels = [];

  const state = {
    preset: '', seed: 0, tempo: TEMPO,
    scene: 'A', morph: 0,
    chain: { enabled: false, mode: 0, entryCount: 0, entries: [] },
    macros: { complexity: 0, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0 },
    lanes: [],
    presets: [],
    noteMap: identityNoteMap(),
  };

  function readLaneFromWasm(laneIdx) {
    const li = Module._poly_lane_int;
    const lf = Module._poly_lane_float;

    const steps = li(engineCtx, laneIdx, LaneFieldInt.CycleSteps);
    const hits = li(engineCtx, laneIdx, LaneFieldInt.HitCount);
    const rot = li(engineCtx, laneIdx, LaneFieldInt.Rotation);
    const subdivision = li(engineCtx, laneIdx, LaneFieldInt.Subdivision);
    const stepLen = Math.round(8 / subdivision);
    const cellCount = li(engineCtx, laneIdx, LaneFieldInt.CellCount);
    const timeline = li(engineCtx, laneIdx, LaneFieldInt.Timeline) !== 0;

    let pattern;
    if (timeline) {
      const bufPtr = Module._malloc(steps * 4);
      const len = Module._poly_lane_fixed_pattern(engineCtx, laneIdx, bufPtr, steps);
      pattern = [];
      for (let i = 0; i < len; i++) pattern.push(Module.HEAP32[(bufPtr >> 2) + i]);
      Module._free(bufPtr);
    } else {
      pattern = rotArr(euclid(hits, steps), rot);
    }

    let cells = null;
    if (cellCount > 0) {
      const cellsPtr = Module._poly_lane_cells_ptr(engineCtx, laneIdx);
      if (cellsPtr) {
        cells = [];
        for (let i = 0; i < cellCount; i++) cells.push(Module.HEAP32[(cellsPtr >> 2) + i]);
      }
    }

    const mtPtr = Module._poly_lane_micro_timing_ptr(engineCtx, laneIdx);
    const mt = new Array(steps).fill(0);
    if (mtPtr) {
      for (let i = 0; i < steps; i++) mt[i] = Module.HEAPF32[(mtPtr >> 2) + i];
    }

    const accentsPtr = Module._poly_lane_accents_ptr(engineCtx, laneIdx);
    const accents = new Array(steps).fill(0);
    if (accentsPtr) {
      for (let i = 0; i < steps; i++) accents[i] = Module.HEAPF32[(accentsPtr >> 2) + i];
    }

    const envCount = Module._poly_lane_envelope_count(engineCtx, laneIdx);
    const envs = [];
    for (let i = 0; i < envCount; i++) {
      envs.push({
        target: ENV_TARGETS[Module._poly_lane_envelope_target(engineCtx, laneIdx, i)] || 'Velocity',
        period: Module._poly_lane_envelope_period(engineCtx, laneIdx, i),
        depth: Module._poly_lane_envelope_depth(engineCtx, laneIdx, i),
        on: Module._poly_lane_envelope_active(engineCtx, laneIdx, i) !== 0,
      });
    }

    return {
      name: laneNames[laneIdx] || `Lane ${laneIdx + 1}`,
      role: laneRoles[laneIdx] || 'Custom',
      note: li(engineCtx, laneIdx, LaneFieldInt.MidiNote),
      ch: li(engineCtx, laneIdx, LaneFieldInt.MidiChannel) + 1,
      steps,
      subdivision,
      stepLen,
      vel: li(engineCtx, laneIdx, LaneFieldInt.BaseVelocity),
      prob: lf(engineCtx, laneIdx, LaneFieldFloat.Probability),
      spread: lf(engineCtx, laneIdx, LaneFieldFloat.VelocitySpread),
      ghost: li(engineCtx, laneIdx, LaneFieldInt.GhostFloor),
      push: lf(engineCtx, laneIdx, LaneFieldFloat.TimingOffsetMs),
      hits,
      rot,
      timeline,
      fixed: timeline ? pattern.slice() : null,
      pattern,
      cells,
      cellCount,
      mt,
      envs,
      hue: HUES[laneIdx % 8],
      active: li(engineCtx, laneIdx, LaneFieldInt.Active) !== 0,
      humanize: lf(engineCtx, laneIdx, LaneFieldFloat.HumanizeMs),
      swing: lf(engineCtx, laneIdx, LaneFieldFloat.SwingAmount),
      duration: lf(engineCtx, laneIdx, LaneFieldFloat.NoteDuration),
      emphasisProb: lf(engineCtx, laneIdx, LaneFieldFloat.EmphasisProb),
      timingOffset: lf(engineCtx, laneIdx, LaneFieldFloat.TimingOffsetMs),
      mutationRate: lf(engineCtx, laneIdx, LaneFieldFloat.MutationRate),
      driftRate: lf(engineCtx, laneIdx, LaneFieldFloat.DriftRate),
      phraseLength: lf(engineCtx, laneIdx, LaneFieldFloat.PhraseLength),
      phraseGap: lf(engineCtx, laneIdx, LaneFieldFloat.PhraseGap),
      phraseOffset: lf(engineCtx, laneIdx, LaneFieldFloat.PhraseOffset),
      tempoMultiplier: lf(engineCtx, laneIdx, LaneFieldFloat.TempoMultiplier),
      kotekanSource: li(engineCtx, laneIdx, LaneFieldInt.KotekanSource),
      accents,
    };
  }

  function syncStateFromWasm() {
    const laneCount = Module._poly_active_lane_count(engineCtx);
    state.lanes = [];
    for (let i = 0; i < laneCount; i++) state.lanes.push(readLaneFromWasm(i));
    state.macros = {
      complexity: Module._poly_macro_value(engineCtx, 0),
      density: Module._poly_macro_value(engineCtx, 1),
      syncopation: Module._poly_macro_value(engineCtx, 2),
      swing: Module._poly_macro_value(engineCtx, 3),
      tension: Module._poly_macro_value(engineCtx, 4),
      humanize: Module._poly_macro_value(engineCtx, 5),
    };
    state.seed = Number(Module._poly_seed(engineCtx));
  }

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

  function labelsFromPresetEntry(entry) {
    const names = [];
    const roles = [];
    const labels = [];
    for (const lane of entry.lanes || []) {
      const roleLabel = (lane.roleLabel || '').toLowerCase();
      names.push(ROLE_LABEL_TO_NAME[roleLabel] || roleLabel.replace(/^./, (c) => c.toUpperCase()) || 'Lane');
      roles.push(ENGINE_ROLE_TO_UI[lane.role] || 'Custom');
      labels.push(roleLabel);
    }
    return { names, roles, labels };
  }

  function applyPreset(index) {
    if (index === -1) {
      Module._poly_action_apply_preset(engineCtx, 0);
      currentPresetIndex = -1;
      laneNames = ['Lane 1', 'Lane 2', 'Lane 3', 'Lane 4'];
      laneRoles = ['Anchor pulse', 'Backbeat', 'Shimmer', 'Ghost'];
      laneRoleLabels = [];
      state.preset = 'Init';
      state.scene = 'A';
      state.morph = 0;
      state.chain = { enabled: false, mode: 0, entryCount: 0, entries: [] };
      state.noteMap = identityNoteMap();
    } else if (index >= 0 && index < Module._poly_preset_count()) {
      Module._poly_action_apply_preset(engineCtx, index);
      currentPresetIndex = index;
      const engineName = Module.UTF8ToString(Module._poly_preset_name(index));
      const entry = presetsById && presetsById[index];
      if (entry) {
        const meta = labelsFromPresetEntry(entry);
        laneNames = meta.names;
        laneRoles = meta.roles;
        laneRoleLabels = meta.labels;
      } else {
        // presets.json missing or index out of range: keep engine-driven state,
        // fall back to generic labels so the modal still renders.
        const laneCount = Module._poly_active_lane_count(engineCtx);
        laneNames = Array.from({ length: laneCount }, (_, i) => `Lane ${i + 1}`);
        laneRoles = laneNames.map(() => 'Custom');
        laneRoleLabels = [];
      }
      state.preset = engineName;
      console.info(`[wasm-host] applied ${engineName} (${laneNames.length} lanes)`);
    }
    probe.currentPreset = state.preset;
    probe.laneRoleLabels = laneRoleLabels.slice();
    syncStateFromWasm();
  }

  /* ---------- WebAudio preview voices ---------- */
  let ctx = null, playing = false, startAt = 0, schedTimer = null, nextTick = 0, _nb = null;
  const now8 = () => (playing && ctx ? (ctx.currentTime - startAt) / eighthSec() : 0);

  // Sample-backed playback (M043 S09). Manifest lookup is by role, so we map
  // each lane's display name to a manifest role. Unmapped names get a benign
  // default so unusual lane sets still play something; a full manifest failure
  // flips fallbackActive so E2E can tell "sample path" from "synth rescue".
  const NAME_TO_ROLE = {
    'kick': 'kick', 'bass': 'kick', 'davul': 'kick', 'surdo': 'kick',
    'mrid bass': 'kick', 'fixed': 'kick',
    'snare': 'snare', 'rim': 'snare', 'tamborim': 'snare', 'mrid treble': 'snare',
    'hi-hat': 'hat', 'open hat': 'hat', 'pandeiro': 'hat', 'shimmer': 'hat',
    'kanjira': 'hat', 'glitch': 'hat', 'pulse': 'hat',
    'tom': 'tom', 'ghost tom': 'tom', 'ghost': 'tom', 'gong': 'tom', 'drifting': 'tom',
    'shaker': 'shaker', 'perc': 'shaker',
    'clave': 'clave',
    'conga': 'conga', 'djembe': 'conga',
    'darbuka': 'darbuka',
    'cowbell': 'cowbell', 'bell': 'cowbell', 'ghatam': 'cowbell', 'sangsih': 'cowbell',
    'agogo': 'agogo', 'polos': 'agogo', 'zurna': 'agogo',
  };
  const DEFAULT_ROLE = 'hat';

  // Role aliases for presets whose roleLabel doesn't have a matching manifest
  // entry. Mirrors the card runtime's note-based fallback (createSampleLoader
  // falls back to any entry whose midiNotes includes the requested note, which
  // implicitly resolves clap→handclap etc.). Without this the WASM host would
  // silently synth-fallback for every clap/openhat/rim lane while the card is
  // audibly playing samples for the same chapter.
  const ROLE_ALIASES = {
    clap: 'handclap',
    openhat: 'hat',
    rim: 'snare',
  };

  function roleForLaneName(name, laneIdx) {
    // Prefer the engine's per-lane roleLabel (from presets.json) when available.
    // Falls back to the name→role table for synthetic/init lanes that never
    // came from a preset entry.
    if (typeof laneIdx === 'number' && laneRoleLabels[laneIdx]) {
      return laneRoleLabels[laneIdx];
    }
    return NAME_TO_ROLE[(name || '').toLowerCase().trim()] || DEFAULT_ROLE;
  }

  const buffersByRole = new Map();
  // S11 T06 extends the probe with:
  //   - missingRoles / missingRolesFired / lastError: first CI-observable
  //     signal that sampleVoice() fell through to synthVoice(); only populated
  //     when !fallbackActive so single-role misses don't get double-attributed
  //     to the manifest-total-failure branch.
  //   - measuredRmsDb: rolling RMS (dBFS) of the shared voice bus for
  //     cross-surface loudness parity with the card scheduler.
  const probe = {
    nodesStarted: 0,
    samplesLoaded: 0,
    currentPreset: '',
    fallbackActive: false,
    laneRoleLabels: [],
    missingRoles: [],
    missingRolesFired: false,
    lastError: null,
    measuredRmsDb: -Infinity,
  };
  window.__polyAudioProbe = probe;
  let samplesPromise = null;
  // True once samplesPromise has resolved (successfully or not). Gates
  // missingRoles population so cold-start voices — which fire before decode
  // completes — don't get double-attributed as manifest gaps.
  let samplesReady = false;
  // Shared voice bus + analyser tap (S11 T06). Created lazily inside
  // togglePlay() the first time ctx exists so tests always see them by the
  // time nodesStarted > 0.
  let voiceBus = null;
  let analyser = null;
  let rmsFrame = null;

  function ensureAnalyserTap() {
    if (!ctx || voiceBus) return;
    voiceBus = ctx.createGain();
    voiceBus.gain.value = 1;
    analyser = ctx.createAnalyser();
    analyser.fftSize = 2048;
    analyser.smoothingTimeConstant = 0;
    voiceBus.connect(analyser);
    analyser.connect(ctx.destination);
  }

  function voiceOut() {
    // Preserves the pre-S11-T06 topology (direct ctx.destination) when the
    // analyser tap hasn't been created yet (e.g. cold ctx). Once ensureAnalyserTap
    // runs, all voices flow through the shared bus and the analyser sees the
    // real mix.
    return voiceBus || ctx.destination;
  }

  function startRmsSampler() {
    if (rmsFrame !== null || !analyser) return;
    const buf = new Float32Array(analyser.fftSize);
    const tick = () => {
      if (!analyser) return;
      analyser.getFloatTimeDomainData(buf);
      let sum = 0;
      for (let i = 0; i < buf.length; i++) sum += buf[i] * buf[i];
      const rms = Math.sqrt(sum / buf.length);
      probe.measuredRmsDb = rms > 0 ? 20 * Math.log10(rms) : -Infinity;
      rmsFrame = requestAnimationFrame(tick);
    };
    rmsFrame = requestAnimationFrame(tick);
  }

  function stopRmsSampler() {
    if (rmsFrame !== null) {
      cancelAnimationFrame(rmsFrame);
      rmsFrame = null;
    }
    probe.measuredRmsDb = -Infinity;
  }

  function samplesBasePath() {
    return new URL('../samples/', location.href).href;
  }

  async function ensureSamplesLoaded() {
    if (samplesPromise) return samplesPromise;
    if (!ctx) return; // deferred until first togglePlay creates ctx
    samplesPromise = (async () => {
      const base = samplesBasePath();
      try {
        const res = await fetch(base + 'manifest.json');
        if (!res.ok) throw new Error(`manifest ${res.status}`);
        const manifest = await res.json();

        // One sample per role — first-hit wins the slot.
        const roleToFile = {};
        for (const s of manifest.samples || []) {
          if (s.role && !roleToFile[s.role]) roleToFile[s.role] = s.file;
        }
        await Promise.all(Object.entries(roleToFile).map(async ([role, file]) => {
          try {
            const r = await fetch(base + file);
            if (!r.ok) throw new Error(`fetch ${file} -> ${r.status}`);
            const buf = await r.arrayBuffer();
            const audio = await ctx.decodeAudioData(buf);
            buffersByRole.set(role, audio);
            probe.samplesLoaded++;
          } catch (e) {
            console.warn('[wasm-host] sample skipped:', file, e.message);
          }
        }));
        if (buffersByRole.size === 0) throw new Error('no samples decoded');
      } catch (e) {
        console.warn('[wasm-host] manifest load failed, synth fallback active:', e.message);
        probe.fallbackActive = true;
      } finally {
        samplesReady = true;
      }
    })();
    return samplesPromise;
  }

  function noiseBuf() {
    if (_nb) return _nb;
    _nb = ctx.createBuffer(1, ctx.sampleRate * 0.2, ctx.sampleRate);
    const d = _nb.getChannelData(0);
    for (let i = 0; i < d.length; i++) d[i] = Math.random() * 2 - 1;
    return _nb;
  }

  function sampleVoice(role, when, v) {
    let buf = buffersByRole.get(role);
    if (!buf && role && ROLE_ALIASES[role]) {
      buf = buffersByRole.get(ROLE_ALIASES[role]);
    }
    if (!buf) {
      // S11 T06: only populate missing-role probe fields when samples have
      // finished loading (samplesReady) AND we're NOT in the manifest-total-
      // failure branch (fallbackActive). Guard 1 avoids double-attributing
      // cold-start races; guard 2 avoids double-attributing the fallback that
      // the S10 gate already catches.
      if (samplesReady && !probe.fallbackActive && role) {
        if (!probe.missingRoles.includes(role)) probe.missingRoles.push(role);
        probe.missingRolesFired = true;
        probe.lastError = `no sample for role "${role}"`;
      }
      return false;
    }
    const src = ctx.createBufferSource();
    const g = ctx.createGain();
    src.buffer = buf;
    // Unified gain staging with the site card scheduler (M043 S11 T03):
    // both surfaces play sample buffers at raw velocity/127 with no per-role
    // trim. Keeps RMS delta between card and Try It within ±3 dB.
    g.gain.setValueAtTime(Math.max(0, Math.min(1, v)), when);
    src.connect(g); g.connect(voiceOut());
    src.start(when);
    probe.nodesStarted++;
    return true;
  }

  function synthVoice(l, when, v) {
    if (l.name === 'Kick') {
      const o = ctx.createOscillator(), g = ctx.createGain();
      o.type = 'sine';
      o.frequency.setValueAtTime(120, when);
      o.frequency.exponentialRampToValueAtTime(44, when + 0.11);
      g.gain.setValueAtTime(v, when);
      g.gain.exponentialRampToValueAtTime(0.001, when + 0.24);
      o.connect(g); g.connect(voiceOut()); o.start(when); o.stop(when + 0.26);
    } else if (l.name === 'Bell') {
      [562, 845].forEach((f, i) => {
        const o = ctx.createOscillator(), g = ctx.createGain(), bp = ctx.createBiquadFilter();
        o.type = 'square'; o.frequency.value = f;
        bp.type = 'bandpass'; bp.frequency.value = f; bp.Q.value = 4;
        g.gain.setValueAtTime(v * (i ? 0.4 : 0.62), when);
        g.gain.exponentialRampToValueAtTime(0.001, when + 0.12);
        o.connect(bp); bp.connect(g); g.connect(voiceOut()); o.start(when); o.stop(when + 0.14);
      });
    } else if (l.name === 'Snare') {
      const s = ctx.createBufferSource(), g = ctx.createGain(), hp = ctx.createBiquadFilter();
      s.buffer = noiseBuf(); hp.type = 'highpass'; hp.frequency.value = 1400;
      g.gain.setValueAtTime(v * 0.8, when);
      g.gain.exponentialRampToValueAtTime(0.001, when + 0.14);
      s.connect(hp); hp.connect(g); g.connect(voiceOut()); s.start(when); s.stop(when + 0.15);
    } else if (l.name === 'Shaker') {
      const s = ctx.createBufferSource(), g = ctx.createGain(), hp = ctx.createBiquadFilter();
      s.buffer = noiseBuf(); hp.type = 'highpass'; hp.frequency.value = 6200;
      g.gain.setValueAtTime(v * 0.55, when);
      g.gain.exponentialRampToValueAtTime(0.001, when + 0.055);
      s.connect(hp); hp.connect(g); g.connect(voiceOut()); s.start(when); s.stop(when + 0.06);
    } else {
      const o = ctx.createOscillator(), g = ctx.createGain();
      o.type = 'sine';
      o.frequency.setValueAtTime(340, when);
      o.frequency.exponentialRampToValueAtTime(255, when + 0.07);
      g.gain.setValueAtTime(v * 0.6, when);
      g.gain.exponentialRampToValueAtTime(0.001, when + 0.12);
      o.connect(g); g.connect(voiceOut()); o.start(when); o.stop(when + 0.13);
    }
    probe.nodesStarted++;
  }

  function voice(l, li, when, v) {
    // v is the shaped velocity from hitVelocity() in [0,1]; the card scheduler
    // uses raw velocity/127 with no attenuation. Match its staging.
    const role = roleForLaneName(l.name, li);
    if (sampleVoice(role, when, v)) return;
    synthVoice(l, when, v);
  }

  function schedule() {
    const ahead = ctx.currentTime + 0.14;
    const step = eighthSec();
    while (startAt + nextTick * step < ahead) {
      state.lanes.forEach((l, li) => {
        const hit = laneHitAt(l, nextTick);
        if (!hit) return;
        const vel = hitVelocity(l, li, nextTick, hit);
        const mtMs = (l.mt && l.mt[hit.step]) || 0;
        const t = startAt + nextTick * step + (l.push + mtMs) / 1000;
        voice(l, li, Math.max(ctx.currentTime + 0.001, t), vel);
      });
      nextTick++;
    }
  }

  function togglePlay() {
    try {
      if (!ctx) ctx = new (window.AudioContext || window.webkitAudioContext)();
      if (ctx.state === 'suspended') ctx.resume();
    } catch (e) {
      ctx = null;
    }
    if (ctx) ensureAnalyserTap();
    // Fire-and-forget: schedule() falls back to synth voices until buffers
    // land, so nothing is silent while the first fetch is in flight.
    if (ctx) ensureSamplesLoaded();
    playing = !playing;
    if (playing) {
      // Reset per-run probe fields (missing-role, RMS). fallbackActive is
      // NOT reset — it's a one-way sticky flag set by ensureSamplesLoaded.
      probe.missingRoles = [];
      probe.missingRolesFired = false;
      probe.lastError = null;
      startAt = ctx ? ctx.currentTime + 0.06 : 0;
      nextTick = 0;
      if (ctx) {
        schedTimer = setInterval(schedule, 30);
        startRmsSampler();
      }
    } else {
      clearInterval(schedTimer);
      stopRmsSampler();
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

  /* ---------- edit handler ---------- */
  function edit(paramId, value, gesture) {
    if (gesture === 'begin') return;

    if (paramId.startsWith('macro.')) {
      const macroNames = ['complexity', 'density', 'syncopation', 'swing', 'tension', 'humanize'];
      const idx = macroNames.indexOf(paramId.slice(6));
      if (idx >= 0) Module._poly_set_macro(engineCtx, idx, value);
      syncStateFromWasm();
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
      Module._poly_set_seed(engineCtx, Math.round(value * 999999));
      syncStateFromWasm();
      emitState();
      return;
    }

    const m = paramId.match(/^lane\.(\d+)\.(.+)$/);
    if (!m) return;
    const laneIdx = parseInt(m[1]);
    const field = m[2];

    const INT_EDITS = {
      note:         v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.MidiNote, Math.round(v * 127)),
      channel:      v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.MidiChannel, Math.round(v * 15)),
      velocity:     v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.BaseVelocity, Math.round(v * 127)),
      ghostFloor:   v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.GhostFloor, Math.round(v * 127)),
      active:       v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.Active, v >= 0.5 ? 1 : 0),
      steps:        v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.CycleSteps, Math.round(v * 63) + 1),
      hits:         v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.HitCount, Math.round(v * 64)),
      rotation:     v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.Rotation, Math.round(v * 63)),
      subdivision:  v => {
        const subs = [1, 2, 4, 8, 16];
        Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.Subdivision, subs[Math.round(v * 4)] || 4);
      },
      kotekanSource:v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.KotekanSource, Math.round(v * 8) - 1),
      timeline:     v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.Timeline, v >= 0.5 ? 1 : 0),
      cellCount:    v => Module._poly_edit_lane_int(engineCtx, laneIdx, LaneFieldInt.CellCount, Math.round(v * 64)),
    };

    const FLOAT_EDITS = {
      probability:  v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.Probability, v),
      emphasisProb: v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.EmphasisProb, v),
      spread:       v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.VelocitySpread, v),
      swing:        v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.SwingAmount, v),
      humanize:     v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.HumanizeMs, v * 50),
      duration:     v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.NoteDuration, v * 4),
      mutationRate: v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.MutationRate, v),
      driftRate:    v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.DriftRate, v * 8 - 4),
      timingOffset: v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.TimingOffsetMs, v * 40 - 20),
      phraseLength: v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.PhraseLength, v * 64),
      phraseGap:    v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.PhraseGap, v * 64),
      phraseOffset: v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.PhraseOffset, v * 64),
      tempoMult:    v => Module._poly_edit_lane_float(engineCtx, laneIdx, LaneFieldFloat.TempoMultiplier, v),
    };

    if (INT_EDITS[field]) INT_EDITS[field](value);
    else if (FLOAT_EDITS[field]) FLOAT_EDITS[field](value);
    else return;

    syncStateFromWasm();
    emitState();
  }

  /* ---------- action handler ---------- */
  function action(name, payload = {}) {
    switch (name) {
      case 'togglePlay':
        togglePlay();
        return;
      case 'toggleStep':
        Module._poly_action_toggle_step(engineCtx, payload.lane, payload.step);
        break;
      case 'setEuclid':
        Module._poly_action_set_euclid(
          engineCtx, payload.lane,
          payload.steps ?? -1, payload.hits ?? -1, payload.rotation ?? 0
        );
        break;
      case 'setCells': {
        if (payload.cells) {
          const buf = Module._malloc(payload.cells.length * 4);
          for (let i = 0; i < payload.cells.length; i++)
            Module.HEAP32[(buf >> 2) + i] = payload.cells[i];
          Module._poly_action_set_cells(engineCtx, payload.lane, buf, payload.cells.length);
          Module._free(buf);
        } else {
          Module._poly_action_clear_cells(engineCtx, payload.lane);
        }
        break;
      }
      case 'setFixedStep':
        Module._poly_action_set_fixed_step(engineCtx, payload.lane, payload.step, payload.on ? 1 : 0);
        break;
      case 'setMicroTiming':
        Module._poly_action_set_micro_timing(engineCtx, payload.lane, payload.step, payload.ms);
        break;
      case 'setEnvelope':
        if (payload.envelope === null) {
          Module._poly_action_remove_envelope(engineCtx, payload.lane, payload.index);
        } else {
          const targetIdx = ENV_TARGETS.indexOf(payload.envelope.target);
          Module._poly_action_set_envelope(
            engineCtx, payload.lane, payload.index,
            targetIdx >= 0 ? targetIdx : 0,
            payload.envelope.period, payload.envelope.depth,
            payload.envelope.on ? 1 : 0
          );
        }
        break;
      case 'selectScene':
        state.scene = payload.scene === 'Morph' ? 'Morph' : (payload.scene === 'B' ? 'B' : 'A');
        break;
      case 'chainAddEntry':
        if (state.chain.entryCount < 16) {
          state.chain.entries.push({ scene: 0, bars: 4 });
          state.chain.entryCount++;
        }
        break;
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
      case 'setTempo': {
        const bpm = Number(payload.bpm);
        if (Number.isFinite(bpm) && bpm >= 20 && bpm <= 300) {
          TEMPO = bpm;
          state.tempo = bpm;
        }
        break;
      }
      case 'setAccent':
        Module._poly_action_set_accent(engineCtx, payload.lane, payload.step, payload.value);
        break;
      case 'setNoteMap': {
        const ni = payload.note;
        if (ni >= 0 && ni < 128) state.noteMap[ni] = Math.max(0, Math.min(127, payload.output));
        break;
      }
      case 'resetNoteMap':
        state.noteMap = identityNoteMap();
        break;
      case 'exportRequest':
        console.info('[wasm-host] exportRequest — web mode, no-op');
        break;
      default:
        console.warn('[wasm-host] unknown action', name, payload);
        return;
    }
    syncStateFromWasm();
    if (name !== 'togglePlay') emitState();
  }

  async function loadPresetsJson() {
    // Same file the site card scheduler consumes — served from
    // site/public/webui/presets.json (dev) and copied there by copy-webui
    // (build). Fetch relative to the current document so a mounted /poly base
    // path resolves correctly.
    try {
      const url = new URL('./presets.json', location.href).href;
      const res = await fetch(url);
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const parsed = await res.json();
      if (!Array.isArray(parsed.presets)) throw new Error('presets array missing');
      presetsById = {};
      for (const p of parsed.presets) presetsById[p.index] = p;
      console.info(`[wasm-host] loaded ${parsed.presets.length} presets (schemaVersion ${parsed.schemaVersion})`);
    } catch (e) {
      console.warn('[wasm-host] presets.json load failed; falling back to engine-only lane labels:', e.message);
      presetsById = null;
    }
  }

  /* ---------- initialization ---------- */
  async function initWasmHost(wasmModule) {
    Module = wasmModule;
    engineCtx = Module._poly_create();

    await loadPresetsJson();

    const presetCount = Module._poly_preset_count();
    state.presets = [];
    for (let i = 0; i < presetCount; i++) {
      state.presets.push({
        name: Module.UTF8ToString(Module._poly_preset_name(i)),
        description: Module.UTF8ToString(Module._poly_preset_description(i)),
      });
    }

    applyPreset(9);
    syncStateFromWasm();

    window.PolyWasmHost = {
      schemaVersion: window.POLY_SCHEMA_VERSION,
      capabilities: { canExport: false },
      getState: () => state,
      onState: (cb) => stateSubs.push(cb),
      edit,
      action,
      onFrame: (cb) => frameSubs.push(cb),
      _pushState: emitState,
      setAsyncMode: (enabled) => { asyncMode = !!enabled; pendingPush = false; },
      flushState,
      hasPendingPush: () => pendingPush,
      // S11 T06: cross-surface consistency accessor. Returns the same shape
      // the site card exposes via __polyPatterns.resolvePreset — engine name
      // plus per-lane {noteNumber, roleLabel}. Sourced from presetsById (the
      // shared presets.json) so both surfaces agree on structure by definition.
      getResolvedPreset: () => {
        if (currentPresetIndex === -1 || !presetsById) {
          return { engineName: '', lanes: [] };
        }
        const entry = presetsById[currentPresetIndex];
        if (!entry) return { engineName: '', lanes: [] };
        return {
          engineName: entry.name || state.preset || '',
          lanes: (entry.lanes || []).map((l) => ({
            noteNumber: l.noteNumber,
            roleLabel: l.roleLabel,
          })),
        };
      },
    };

    return window.PolyWasmHost;
  }

  window.initPolyWasmHost = initWasmHost;
})();
