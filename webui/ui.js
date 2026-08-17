'use strict';
/**
 * Poly web UI renderer. Talks only to window.PolyHost (host-iface.js).
 * All groove truth lives in the host; this file renders state, forwards
 * edits, and animates from feedback frames.
 */
(function () {
  const host = window.PolyHost;
  const { cyc8, onsets, laneHitAt, hitVelocity } = window.PolyGrooveMath;
  const CONV = 120;
  const REDUCED = matchMedia('(prefers-reduced-motion: reduce)').matches;
  const NS = 'http://www.w3.org/2000/svg';
  // M032 S02 (T03): per-lane export/drag affordance is plugin-only, gated on the
  // same canExport capability as the global Export chip. The mock flips it true
  // via the ?export=1 seam so the per-lane-export spec can exercise the trigger
  // contract without a native build.
  const CAN_EXPORT = !!(host.capabilities && host.capabilities.canExport);

  let S = null; // latest state snapshot
  let lastFrame = { t8: 0, playing: false, convLeft: CONV, lanes: [] };
  let mode = 'desk';
  let expanded = -1;
  let strips = [], rings = [], hands = [], ladders = [], vus = [], playeds = [];
  const tabState = {};
  // M045 S01 T03: per-lane, per-step overlay class diff cache. Populated by
  // updateEmissionOverlay each frame; reset in buildDesk when the ladder DOM
  // is torn down and rebuilt. Declared here so buildDesk can reset it during
  // the initial boot() call before the frame handler wires up.
  const overlayLastKind = [];

  /* ================= chrome ================= */
  const embedded = !!window.__POLY_EMBEDDED__;
  const playBtn = document.getElementById('play');
  // The Play button is visually gated on state.samplesReady in renderChrome
  // (opacity/cursor/title). The click itself is NOT blocked here — the host's
  // togglePlay defers the transport-flag flip until decodeAudioData finishes,
  // so a click during decode is a no-op audibly and visually (no desk
  // animation) but is still accepted so e2e specs that click Play synchronously
  // after modal open keep working.
  playBtn.addEventListener('click', () => { if (!embedded) host.action('togglePlay', {}); });
  if (embedded) {
    // Fill the plugin iframe: no centered gutter / border / radius / shadow.
    document.body.classList.add('embedded');
    playBtn.style.opacity = '0.35';
    playBtn.style.cursor = 'default';
    playBtn.title = 'Transport controlled by host DAW';
  }
  addEventListener('keydown', (e) => {
    if (e.code === 'Space' && !e.repeat) {
      if (embedded) return;
      e.preventDefault();
      host.action('togglePlay', {});
    }
    if (e.key === 'l' || e.key === 'L') toggleLearn();
    if (e.key === 'Escape') { closePresetMenu(); expandStrip(-1); }
    if (e.key === '1') setMode('cloth');
    if (e.key === '2') setMode('desk');
  });
  function toggleLearn() {
    document.body.classList.toggle('learn');
    document.getElementById('learnBtn').classList.toggle('on');
  }
  document.getElementById('learnBtn').addEventListener('click', toggleLearn);
  document.getElementById('scA').addEventListener('click', () => host.action('selectScene', { scene: 'A' }));
  document.getElementById('scB').addEventListener('click', () => host.action('selectScene', { scene: 'B' }));
  document.getElementById('scM').addEventListener('click', () => host.action('selectScene', { scene: 'Morph' }));

  /* --- seed dice + history (M034 S03) ---
     The global seed already renders in the chrome (#seedVal). The dice rolls a
     fresh seed through the existing 'seed' edit path; a ring buffer of visited
     seeds lets back/forward restore earlier rolls. A navigation restores an
     existing entry (it does not branch or duplicate history); only a genuinely
     new seed (dice, preset, or any external seed change) appends a visit. */
  const SEED_MAX = 999999;
  const SEED_HIST_CAP = 32;
  const seedDice = document.getElementById('seedDice');
  const seedBack = document.getElementById('seedBack');
  const seedFwd = document.getElementById('seedFwd');
  const seedHistPos = document.getElementById('seedHistPos');
  let seedHistory = [];
  let seedHistIdx = -1;
  let seedNavigating = false;

  function emitSeed(seed) {
    const norm = Math.max(0, Math.min(1, seed / SEED_MAX));
    host.edit('seed', norm, 'begin');
    host.edit('seed', norm, 'perform');
    host.edit('seed', norm, 'end');
  }
  function updateSeedNav() {
    if (seedBack) seedBack.disabled = seedHistIdx <= 0;
    if (seedFwd) seedFwd.disabled = seedHistIdx < 0 || seedHistIdx >= seedHistory.length - 1;
    if (seedHistPos)
      seedHistPos.textContent = seedHistory.length ? `${seedHistIdx + 1}/${seedHistory.length}` : '';
  }
  function recordSeed(seed) {
    // A back/forward restore re-emits a known seed — do not record it as a new
    // visit (that would duplicate the entry and strand the forward branch).
    if (seedNavigating) return;
    if (seedHistIdx >= 0 && seedHistory[seedHistIdx] === seed) return;
    // A new roll from anywhere but the tail drops the stale forward branch.
    seedHistory = seedHistory.slice(0, seedHistIdx + 1);
    seedHistory.push(seed);
    if (seedHistory.length > SEED_HIST_CAP) seedHistory.shift();
    seedHistIdx = seedHistory.length - 1;
    updateSeedNav();
  }
  function navigateSeed(delta) {
    const next = seedHistIdx + delta;
    if (next < 0 || next >= seedHistory.length) return;
    seedHistIdx = next;
    // Suppress the record that the resulting state push would otherwise trigger.
    seedNavigating = true;
    emitSeed(seedHistory[seedHistIdx]);
    seedNavigating = false;
    updateSeedNav();
  }
  if (seedDice) {
    seedDice.addEventListener('click', () => {
      // Roll a seed distinct from the current head so the readout always moves.
      let seed;
      do { seed = Math.floor(Math.random() * (SEED_MAX + 1)); }
      while (seedHistory.length && seed === seedHistory[seedHistIdx]);
      emitSeed(seed);
    });
  }
  if (seedBack) seedBack.addEventListener('click', () => navigateSeed(-1));
  if (seedFwd) seedFwd.addEventListener('click', () => navigateSeed(1));

  /* --- morph slider --- */
  const morphSlider = document.getElementById('morphSlider');
  const morphTrack = morphSlider.querySelector('.morph-track');
  const morphFill = morphTrack.querySelector('i');
  (function initMorphSlider() {
    const calc = (e) => {
      const r = morphTrack.getBoundingClientRect();
      return Math.max(0, Math.min(1, (e.clientX - r.left) / r.width));
    };
    const paint = (v) => { morphFill.style.width = `${(v * 100).toFixed(1)}%`; };
    morphTrack.addEventListener('pointerdown', (e) => {
      e.preventDefault();
      let lastV = calc(e); paint(lastV);
      host.edit('scene.morph', lastV, 'begin');
      host.edit('scene.morph', lastV, 'perform');
      const mv = (ev) => { lastV = calc(ev); paint(lastV); host.edit('scene.morph', lastV, 'perform'); };
      const up = () => {
        window.removeEventListener('pointermove', mv);
        host.edit('scene.morph', lastV, 'end');
      };
      window.addEventListener('pointermove', mv);
      window.addEventListener('pointerup', up, { once: true });
    });
  })();

  /* --- chain popover --- */
  const chainBtn = document.getElementById('chainBtn');
  let chainPopover = null;
  // M044 S07: dismiss handler is stored so rebuild-on-state-emit doesn't leave
  // stale listeners on document. Without this, each rebuild's stale handler
  // (with its own captured `pop`) fires on the next click bubble, sees the
  // click as "outside" the removed popover, and immediately re-closes.
  let chainDismiss = null;
  function buildChainPopover() {
    if (chainPopover) { chainPopover.remove(); chainPopover = null; }
    if (chainDismiss) { document.removeEventListener('click', chainDismiss); chainDismiss = null; }
    const pop = document.createElement('div');
    pop.className = 'chain-popover open';
    pop.id = 'chainPopover';
    const chain = S.chain || { enabled: false, mode: 0, entryCount: 0, entries: [] };
    const MODE_NAMES = ['1-Shot', 'Loop', 'Ping-Pong'];
    const SCENE_NAMES = ['A', 'B', 'Morph'];
    let html = `<h4>Scene Chain</h4>`;
    html += `<div class="chain-row"><label>Enable</label><span class="switch"><button class="${chain.enabled ? 'on' : ''}" data-chain-enable aria-label="Enable chain"><i></i></button></span></div>`;
    html += `<div class="chain-row"><label>Mode</label><div class="chain-modes">${MODE_NAMES.map((n, i) =>
      `<button class="chip${chain.mode === i ? ' on' : ''}" data-chain-mode="${i}">${n}</button>`).join('')}</div></div>`;
    for (let ei = 0; ei < chain.entryCount; ei++) {
      const entry = chain.entries[ei] || { scene: 0, bars: 4 };
      html += `<div class="chain-entry" data-entry="${ei}">
        <span class="idx">${ei + 1}.</span>
        <div class="scene-btns">${SCENE_NAMES.map((n, si) =>
          `<button class="chip${entry.scene === si ? ' on' : ''}" data-entry-scene="${ei}" data-sv="${si}">${n}</button>`).join('')}</div>
        <div class="bars-ctl"><button data-bars-dec="${ei}">−</button><span>${entry.bars}</span><button data-bars-inc="${ei}">+</button></div>
        <button class="rm" data-rm="${ei}" aria-label="Remove entry">×</button>
      </div>`;
    }
    html += `<div class="chain-add"><button data-chain-add aria-label="Add entry">+</button></div>`;
    pop.innerHTML = html;
    document.getElementById('chrome').appendChild(pop);
    chainPopover = pop;

    pop.querySelector('[data-chain-enable]').addEventListener('click', () => {
      // M044 S07: capture the target value BEFORE the edit sequence. `chain`
      // is a live reference into state.chain — the 'perform' call synchronously
      // mutates it, so re-reading `chain.enabled` on 'end' flips the value
      // right back to 0 and the toggle appears to do nothing.
      const nv = chain.enabled ? 0 : 1;
      host.edit('chain.enabled', nv, 'begin');
      host.edit('chain.enabled', nv, 'perform');
      host.edit('chain.enabled', nv, 'end');
    });
    pop.querySelectorAll('[data-chain-mode]').forEach((b) =>
      b.addEventListener('click', () => {
        const m = parseInt(b.dataset.chainMode);
        host.edit('chain.mode', m / 2, 'begin');
        host.edit('chain.mode', m / 2, 'perform');
        host.edit('chain.mode', m / 2, 'end');
      }));
    pop.querySelectorAll('[data-entry-scene]').forEach((b) =>
      b.addEventListener('click', () => {
        const ei = parseInt(b.dataset.entryScene);
        const sv = parseInt(b.dataset.sv);
        host.edit(`chain.entry.${ei}.scene`, sv / 2, 'begin');
        host.edit(`chain.entry.${ei}.scene`, sv / 2, 'perform');
        host.edit(`chain.entry.${ei}.scene`, sv / 2, 'end');
      }));
    pop.querySelectorAll('[data-bars-dec]').forEach((b) =>
      b.addEventListener('click', () => {
        const ei = parseInt(b.dataset.barsDec);
        const entry = chain.entries[ei] || { bars: 4 };
        const nb = Math.max(1, entry.bars - 1);
        host.edit(`chain.entry.${ei}.bars`, (nb - 1) / 31, 'begin');
        host.edit(`chain.entry.${ei}.bars`, (nb - 1) / 31, 'perform');
        host.edit(`chain.entry.${ei}.bars`, (nb - 1) / 31, 'end');
      }));
    pop.querySelectorAll('[data-bars-inc]').forEach((b) =>
      b.addEventListener('click', () => {
        const ei = parseInt(b.dataset.barsInc);
        const entry = chain.entries[ei] || { bars: 4 };
        const nb = Math.min(32, entry.bars + 1);
        host.edit(`chain.entry.${ei}.bars`, (nb - 1) / 31, 'begin');
        host.edit(`chain.entry.${ei}.bars`, (nb - 1) / 31, 'perform');
        host.edit(`chain.entry.${ei}.bars`, (nb - 1) / 31, 'end');
      }));
    pop.querySelectorAll('[data-rm]').forEach((b) =>
      b.addEventListener('click', () => {
        host.action('chainRemoveEntry', { index: parseInt(b.dataset.rm) });
      }));
    pop.querySelector('[data-chain-add]').addEventListener('click', () => {
      host.action('chainAddEntry', {});
    });

    const dismiss = (e) => {
      if (!pop.contains(e.target) && e.target !== chainBtn) {
        closeChainPopover();
      }
    };
    chainDismiss = dismiss;
    setTimeout(() => document.addEventListener('click', dismiss), 0);
  }
  function closeChainPopover() {
    if (chainPopover) { chainPopover.remove(); chainPopover = null; }
    if (chainDismiss) { document.removeEventListener('click', chainDismiss); chainDismiss = null; }
  }
  chainBtn.addEventListener('click', (e) => {
    e.stopPropagation();
    if (chainPopover) closeChainPopover();
    else buildChainPopover();
  });

  /* --- export button (plugin-only) — opens a native Save As… dialog for the
       CURRENT pattern. Result arrives via polyHostPush {type:'exportResult'}.
       Cancels resolve silently. M053 S11: ungated — the native side renders the
       current pattern offline (poly::renderPatternToSMF) on every click/drag,
       so Export works regardless of DAW transport or capture state. */
  if (host.capabilities && host.capabilities.canExport) {
    const btn = document.getElementById('exportBtn');
    btn.title = 'Save the current pattern as a .mid file, or drag it straight into your DAW';
    btn.addEventListener('click', () => {
      host.action('exportSaveAs', {});
      btn.classList.add('on');
    });
    // M053 S03d (G06): the Export chip doubles as a drag source. A click runs
    // the Save-As path above; dragging it opens the native drag-source window
    // (platform_drag_source) that carries the .mid straight into the DAW.
    // M053 S11: ungated — the native offline render always has a pattern to emit.
    btn.setAttribute('draggable', 'true');
    btn.addEventListener('dragstart', () => {
      host.action('beginMidiDrag', {});
      btn.classList.add('dragging');
    });
    btn.addEventListener('dragend', () => btn.classList.remove('dragging'));
    window.addEventListener('polyExportResult', (e) => {
      btn.classList.remove('on');
      const path = (e && e.detail && e.detail.savedPath) || '';
      if (!path) return;
      // Lightweight toast — auto-clears after 3s.
      let toast = document.getElementById('exportToast');
      if (!toast) {
        toast = document.createElement('div');
        toast.id = 'exportToast';
        toast.style.cssText = 'position:absolute;bottom:14px;left:50%;transform:translateX(-50%);background:rgba(20,17,13,.92);color:var(--text);border:1px solid var(--line);border-radius:6px;padding:8px 14px;font-size:11px;letter-spacing:.06em;z-index:60;pointer-events:none;';
        document.getElementById('win').appendChild(toast);
      }
      const name = path.split('/').pop().split('\\').pop();
      toast.textContent = 'Saved ' + name;
      toast.style.opacity = '1';
      clearTimeout(toast._t);
      toast._t = setTimeout(() => { toast.style.opacity = '0'; toast.style.transition = 'opacity .4s'; }, 3000);
    });
  } else {
    document.getElementById('exportBtn').style.display = 'none';
  }

  /* --- M051 S08 T05: capture header controls (Cloth-only cluster) ---------
     G07: the bars picker is a 1-32 stepper over the kCaptureLength window
     (native export_controls_view parity), replacing the old fixed {4,8,16,32}
     cycle. The Arm chip flips to Reset once the machine leaves idle; the Export
     chip above is ungated (M053 S11 offline render). The Arm/bars chips reflect
     host truth from the feedback frame (lastFrame.capState/capBars) — never
     local state — so the header and the Cloth timeline can never disagree. */
  const CAP_BARS_MIN = 1;
  const CAP_BARS_MAX = 32;
  const capCtl = document.getElementById('capCtl');
  const capBarsBtn = document.getElementById('capBars');
  const armBtn = document.getElementById('armBtn');
  const exportChip = document.getElementById('exportBtn');

  if (capBarsBtn) {
    // Step the capture window by ±1 bar, wrapping across the 1-32 domain so any
    // integer length is reachable. Left-click / wheel-up increments; right-click
    // / wheel-down decrements. Locked once capture latches (state >= 2): the
    // window is frozen and kCaptureLength no longer applies until Reset. Reads
    // host truth (lastFrame.capBars) — never a local counter.
    const stepCaptureBars = (delta) => {
      if ((lastFrame.capState | 0) >= 2) return;
      const cur = Math.min(CAP_BARS_MAX, Math.max(CAP_BARS_MIN, lastFrame.capBars || 8));
      let next = cur + delta;
      if (next > CAP_BARS_MAX) next = CAP_BARS_MIN;
      else if (next < CAP_BARS_MIN) next = CAP_BARS_MAX;
      host.action('setCaptureBars', { bars: next });
    };
    capBarsBtn.addEventListener('click', () => stepCaptureBars(1));
    capBarsBtn.addEventListener('contextmenu', (e) => {
      e.preventDefault();
      stepCaptureBars(-1);
    });
    capBarsBtn.addEventListener('wheel', (e) => {
      e.preventDefault();
      stepCaptureBars(e.deltaY < 0 ? 1 : -1);
    }, { passive: false });
  }
  if (armBtn) {
    armBtn.addEventListener('click', () => {
      host.action((lastFrame.capState | 0) === 0 ? 'armCapture' : 'resetCapture', {});
    });
  }
  // Reflect the capture machine onto the header chips. Called every frame from
  // onFrame so the labels/classes track host truth without a second timer.
  function updateCaptureChips() {
    const st = lastFrame.capState | 0;
    if (capBarsBtn) {
      capBarsBtn.textContent = `${lastFrame.capBars || 8} bars`;
      capBarsBtn.classList.toggle('locked', st >= 2);
    }
    if (armBtn) {
      armBtn.textContent = st === 0 ? 'Arm' : 'Reset';
      armBtn.setAttribute('aria-label', st === 0 ? 'Arm capture' : 'Reset capture');
      armBtn.classList.toggle('on', st >= 1);
    }
    if (exportChip) exportChip.classList.toggle('capReady', st === 3);
  }

  /* --- note map modal --- */
  // GM drum names (General MIDI percussion, notes 35-81) ported verbatim from
  // plugin/source/ui/note_map_view.cpp kDrumNames so the WebUI note-map picker
  // shows the same named drums as the native note_map_view (G05 parity).
  const NM_DRUM_NAMES = {
    35: 'Kick 2', 36: 'Kick 1', 37: 'Side Stick', 38: 'Snare', 39: 'Clap',
    40: 'Elec Snare', 41: 'Floor Tom L', 42: 'Closed HH', 43: 'Floor Tom H', 44: 'Pedal HH',
    45: 'Low Tom', 46: 'Open HH', 47: 'Low-Mid Tom', 48: 'Hi-Mid Tom', 49: 'Crash 1',
    50: 'High Tom', 51: 'Ride 1', 52: 'China', 53: 'Ride Bell', 54: 'Tambourine',
    55: 'Splash', 56: 'Cowbell', 57: 'Crash 2', 58: 'Vibraslap', 59: 'Ride 2',
    60: 'Hi Bongo', 61: 'Lo Bongo', 62: 'Mute Conga H', 63: 'Open Conga H', 64: 'Low Conga',
    65: 'Hi Timbale', 66: 'Lo Timbale', 67: 'Hi Agogo', 68: 'Lo Agogo', 69: 'Cabasa',
    70: 'Maracas', 71: 'Whistle S', 72: 'Whistle L', 73: 'Guiro S', 74: 'Guiro L',
    75: 'Claves', 76: 'Woodblock H', 77: 'Woodblock L', 78: 'Cuica Mute', 79: 'Cuica Open',
    80: 'Triangle M', 81: 'Triangle O',
  };
  let noteMapModal = null;
  let noteMapDismiss = null;
  // View mode persists across modal rebuilds (refreshAll closes+reopens on every
  // state change). 'notes' = full 128-row view; 'lanes' = 8 lane source-note rows.
  let noteMapMode = 'notes';
  function nmDrumLabel(n) {
    return NM_DRUM_NAMES[n] || `N${n}`;
  }
  function buildNoteMapModal() {
    if (noteMapModal) { closeNoteMapModal(); return; }
    const nm = S.noteMap || [];
    let gridHtml;
    if (noteMapMode === 'lanes') {
      // Lane-scoped view: one row per lane (mirrors native note_map_view LANE
      // column). Source = S.lanes[li].note; destination = noteMap[source]. Edits
      // route through setNoteMap keyed on the lane's source note.
      const lanes = S.lanes || [];
      gridHtml = `<div class="notemap-grid notemap-lanes">${lanes.map((lane, li) => {
        const src = lane.note;
        const dst = nm[src] ?? src;
        return `<div class="notemap-row" data-lane="${li}" data-src="${src}">` +
          `<span class="nl">L${li + 1}</span>` +
          `<span class="nm-name">${nmDrumLabel(src)}</span>` +
          `<span class="ni">${src}</span>` +
          `<button data-lnmdec="${src}">−</button>` +
          `<span class="no">${dst}</span>` +
          `<button data-lnminc="${src}">+</button>` +
          `${src !== dst ? '<span class="nm-mod">✦</span>' : ''}` +
          `</div>`;
      }).join('')}</div>`;
    } else {
      gridHtml = `<div class="notemap-grid">${Array.from({ length: 128 }, (_, i) =>
        `<div class="notemap-row" data-note="${i}">` +
        `<span class="ni">${i}</span>` +
        `<span class="nm-name">${NM_DRUM_NAMES[i] || ''}</span>` +
        `<button data-nmdec="${i}">−</button>` +
        `<span class="no">${nm[i] ?? i}</span>` +
        `<button data-nminc="${i}">+</button>` +
        `${nm[i] !== i ? '<span class="nm-mod">✦</span>' : ''}` +
        `</div>`).join('')}</div>`;
    }
    const pop = document.createElement('div');
    pop.className = 'notemap-modal open';
    pop.innerHTML = `<div class="notemap-inner">
      <div class="notemap-header"><h4>Note Map</h4>` +
      `<button class="notemap-view" data-nmview>${noteMapMode === 'notes' ? 'Lanes' : 'Notes'}</button>` +
      `<button class="notemap-reset">Reset</button><button class="notemap-close">✕</button></div>
      ${gridHtml}</div>`;
    document.getElementById('win').appendChild(pop);
    noteMapModal = pop;

    pop.querySelector('.notemap-close').addEventListener('click', closeNoteMapModal);
    pop.querySelector('[data-nmview]').addEventListener('click', (e) => {
      e.stopPropagation();
      noteMapMode = noteMapMode === 'notes' ? 'lanes' : 'notes';
      closeNoteMapModal();
      buildNoteMapModal();
    });
    pop.querySelector('.notemap-reset').addEventListener('click', () => {
      host.action('resetNoteMap', {});
    });
    const stepNoteMap = (src, delta) => {
      const cur = S.noteMap ? (S.noteMap[src] ?? src) : src;
      const next = cur + delta;
      if (next >= 0 && next <= 127) host.action('setNoteMap', { note: src, output: next });
    };
    pop.querySelectorAll('[data-nmdec]').forEach((b) =>
      b.addEventListener('click', () => stepNoteMap(parseInt(b.dataset.nmdec), -1)));
    pop.querySelectorAll('[data-nminc]').forEach((b) =>
      b.addEventListener('click', () => stepNoteMap(parseInt(b.dataset.nminc), +1)));
    pop.querySelectorAll('[data-lnmdec]').forEach((b) =>
      b.addEventListener('click', () => stepNoteMap(parseInt(b.dataset.lnmdec), -1)));
    pop.querySelectorAll('[data-lnminc]').forEach((b) =>
      b.addEventListener('click', () => stepNoteMap(parseInt(b.dataset.lnminc), +1)));

    const dismiss = (e) => {
      if (noteMapModal && !noteMapModal.querySelector('.notemap-inner').contains(e.target) &&
          e.target !== document.getElementById('noteMapBtn')) {
        closeNoteMapModal();
      }
    };
    noteMapDismiss = dismiss;
    setTimeout(() => document.addEventListener('click', dismiss), 0);
  }
  function closeNoteMapModal() {
    if (noteMapModal) { noteMapModal.remove(); noteMapModal = null; }
    if (noteMapDismiss) { document.removeEventListener('click', noteMapDismiss); noteMapDismiss = null; }
  }
  document.getElementById('noteMapBtn').addEventListener('click', (e) => {
    e.stopPropagation();
    buildNoteMapModal();
  });

  /* --- preset dropdown --- */
  const presetTrigger = document.getElementById('presetTrigger');
  const presetBtn = document.getElementById('presetName');
  const presetMenu = document.getElementById('presetMenu');
  let presetMenuBuilt = false;
  let presetCategoryFilter = 'All';

  function buildPresetMenu() {
    if (!S || !S.presets || presetMenuBuilt) return;
    presetMenuBuilt = true;
    presetMenu.innerHTML = '';

    // Chip row: All + one chip per category (order from state.categories,
    // which mirrors the taxonomy in docs/preset-taxonomy.md).
    const cats = Array.isArray(S.categories) ? S.categories : [];
    if (cats.length > 0) {
      const chipRow = document.createElement('div');
      chipRow.className = 'preset-chips';
      chipRow.setAttribute('role', 'tablist');
      chipRow.setAttribute('aria-label', 'Filter presets by category');
      const chipLabels = ['All', ...cats];
      chipLabels.forEach((label) => {
        const chip = document.createElement('button');
        chip.className = 'preset-chip';
        chip.setAttribute('role', 'tab');
        chip.dataset.category = label;
        chip.textContent = label;
        const selected = label === presetCategoryFilter;
        chip.classList.toggle('active', selected);
        chip.tabIndex = selected ? 0 : -1;
        chip.setAttribute('aria-selected', selected ? 'true' : 'false');
        chipRow.appendChild(chip);
      });
      chipRow.addEventListener('click', (e) => {
        const chip = e.target.closest('[role="tab"]');
        if (!chip) return;
        e.stopPropagation();
        selectCategory(chip.dataset.category);
      });
      chipRow.addEventListener('keydown', (e) => {
        if (e.key !== 'ArrowLeft' && e.key !== 'ArrowRight' && e.key !== 'Home' && e.key !== 'End') return;
        e.preventDefault();
        const chips = Array.from(chipRow.querySelectorAll('[role="tab"]'));
        const currentIdx = chips.findIndex((c) => c.dataset.category === presetCategoryFilter);
        let next = currentIdx;
        if (e.key === 'ArrowLeft') next = (currentIdx - 1 + chips.length) % chips.length;
        else if (e.key === 'ArrowRight') next = (currentIdx + 1) % chips.length;
        else if (e.key === 'Home') next = 0;
        else if (e.key === 'End') next = chips.length - 1;
        selectCategory(chips[next].dataset.category);
        chips[next].focus();
      });
      presetMenu.appendChild(chipRow);
      const chipSep = document.createElement('div');
      chipSep.className = 'preset-chips-sep';
      presetMenu.appendChild(chipSep);
    }

    const init = document.createElement('button');
    init.setAttribute('role', 'option');
    init.textContent = 'Init (All Lanes)';
    init.dataset.index = '-1';
    presetMenu.appendChild(init);
    const sep = document.createElement('div');
    sep.className = 'sep';
    presetMenu.appendChild(sep);
    S.presets.forEach((p, i) => {
      const opt = document.createElement('button');
      opt.setAttribute('role', 'option');
      opt.dataset.index = String(i);
      opt.dataset.category = p.category || '';
      opt.innerHTML = `${p.name}<small>${p.description}</small>`;
      presetMenu.appendChild(opt);
    });
    presetMenu.addEventListener('click', (e) => {
      const opt = e.target.closest('[role="option"]');
      if (!opt) return;
      host.action('applyPreset', { index: parseInt(opt.dataset.index, 10) });
      closePresetMenu();
    });
    applyPresetCategoryFilter();
  }

  function selectCategory(label) {
    presetCategoryFilter = label;
    presetMenu.querySelectorAll('.preset-chip').forEach((chip) => {
      const on = chip.dataset.category === label;
      chip.classList.toggle('active', on);
      chip.setAttribute('aria-selected', on ? 'true' : 'false');
      chip.tabIndex = on ? 0 : -1;
    });
    applyPresetCategoryFilter();
  }

  function applyPresetCategoryFilter() {
    // Init always visible; only preset options with matching category are
    // shown when a specific category is selected.
    const showAll = presetCategoryFilter === 'All';
    presetMenu.querySelectorAll('[role="option"]').forEach((opt) => {
      const idx = parseInt(opt.dataset.index, 10);
      const match = showAll || idx === -1 || opt.dataset.category === presetCategoryFilter;
      opt.classList.toggle('preset-hidden', !match);
    });
  }

  function openPresetMenu() {
    buildPresetMenu();
    const r = presetBtn.getBoundingClientRect();
    presetMenu.style.left = `${r.left}px`;
    presetMenu.classList.add('open');
    presetBtn.setAttribute('aria-expanded', 'true');
    markActivePreset();
  }

  function closePresetMenu() {
    presetMenu.classList.remove('open');
    presetBtn.setAttribute('aria-expanded', 'false');
  }

  function markActivePreset() {
    presetMenu.querySelectorAll('[role="option"]').forEach((opt) => {
      const idx = parseInt(opt.dataset.index, 10);
      const name = idx === -1 ? 'Init' : (S.presets && S.presets[idx] ? S.presets[idx].name : '');
      opt.classList.toggle('active', S.preset === name);
    });
  }

  presetBtn.addEventListener('click', (e) => {
    e.stopPropagation();
    if (presetBtn.getAttribute('aria-disabled') === 'true') return;
    if (presetMenu.classList.contains('open')) closePresetMenu();
    else openPresetMenu();
  });
  document.addEventListener('click', (e) => {
    if (!presetMenu.contains(e.target) && e.target !== presetBtn) closePresetMenu();
  });
  document.getElementById('mCloth').addEventListener('click', () => setMode('cloth'));
  document.getElementById('mDesk').addEventListener('click', () => setMode('desk'));

  function setMode(m, focus = -1) {
    mode = m;
    document.getElementById('cloth').classList.toggle('on', m === 'cloth');
    document.getElementById('desk').classList.toggle('on', m === 'desk');
    document.getElementById('mCloth').classList.toggle('on', m === 'cloth');
    document.getElementById('mDesk').classList.toggle('on', m === 'desk');
    // Arm/Reset + bars picker are capture-timeline controls — Cloth mode only.
    if (capCtl) capCtl.classList.toggle('show', m === 'cloth');
    // Learn only reveals annotations inside #cloth — gate the chip to Cloth.
    document.getElementById('learnBtn').classList.toggle('mode-hidden', m !== 'cloth');
    if (m === 'cloth') sizeLoom();
    if (m === 'desk' && focus >= 0) expandStrip(focus);
  }

  /* ================= cloth ================= */
  const loom = document.getElementById('loom');
  const tags = document.getElementById('tags');
  const clothEl = document.getElementById('cloth');
  const clothAnns = ['annA', 'annB', 'annC'].map((id) => document.getElementById(id));
  const capline = clothEl ? clothEl.querySelector('.capline') : null;
  // Eighth-note ticks per bar from the host meter (4/4 -> 8). Feedback frames
  // carry tsNum/tsDen; fall back to 4/4 on legacy hosts.
  function eighthsPerBar() {
    const num = lastFrame.tsNum || 4, den = lastFrame.tsDen || 4;
    return Math.max(1, Math.round(num * (8 / den)));
  }
  loom.addEventListener('click', (e) => {
    const r = loom.getBoundingClientRect();
    setMode('desk', Math.min(S.lanes.length - 1, Math.floor(((e.clientY - r.top) / r.height) * S.lanes.length)));
  });
  addEventListener('resize', () => { if (mode === 'cloth') sizeLoom(); });
  function sizeLoom() {
    const r = loom.parentElement.getBoundingClientRect();
    loom.width = r.width * devicePixelRatio;
    loom.height = r.height * devicePixelRatio;
    drawLoom(lastFrame.t8);
  }
  function buildTags() {
    tags.innerHTML = '';
    S.lanes.forEach((l, li) => {
      const t = document.createElement('div');
      t.className = 'tag';
      t.style.top = `${((li + 0.5) / S.lanes.length) * 100}%`;
      t.innerHTML = `${l.name}<small>${cyc8(l)} picks</small>`;
      tags.appendChild(t);
    });
  }
  // M051 S08: Cloth is a capture timeline whose visual IS the export receipt.
  // In idle it keeps the decorative convergence weave; once armed it becomes a
  // bar-anchored timeline (X = bars 1..N, Y = lanes) with an L->R playhead
  // during capturing and a gold selvage marking bar N at complete.
  function drawLoom(t8) {
    const g = loom.getContext('2d');
    if (!g || !S) return;
    const W = loom.width, H = loom.height, dp = devicePixelRatio;
    g.clearRect(0, 0, W, H);
    const capState = lastFrame.capState | 0;
    const bars = Math.max(1, lastFrame.capBars || 8);
    const prog = Math.max(0, Math.min(bars, lastFrame.capProg || 0));
    updateClothChrome(capState, bars, prog);
    if (capState >= 1) drawCaptureTimeline(g, W, H, dp, bars, capState, prog);
    else drawConvergence(g, W, H, dp, t8);
    // Receipt hook: the capture progression is directly observable (the visual
    // is the receipt). Consumed by webui/tests/capture-timeline.spec.mjs.
    window.__polyClothState = {
      capState, bars,
      mode: capState >= 1 ? 'timeline' : 'convergence',
      playhead: capState === 2 ? prog / bars : capState === 3 ? 1 : 0,
    };
  }

  // M073 S05: fold the engine's per-lane emission ring (host.getLaneEmissions,
  // classified base/ghost/add/drop by the engine and drained in wasm-host.js)
  // into a step->kind map, newest emission per step winning — the same reduction
  // the desk ladder overlay (updateEmissionOverlay) applies. The cloth timeline
  // is a second consumer of that single source of truth, so probability culls,
  // mutation drops, ghosts, and macro-resolved adds render truthfully instead of
  // the drift-prone laneHitAt re-derivation. Returns null when the host exposes
  // no emission accessor or the lane's stream is empty (mock default / DAW /
  // stopped playback): callers then fall back to positional-pattern-only via
  // laneHitAt and MUST NOT draw a drop/ghost marker — never claim an
  // unobservable drop. Mirrors the overlay's empty-return degradation contract.
  function laneStepKind(li) {
    if (!host.getLaneEmissions) return null;
    const emissions = host.getLaneEmissions(li) || [];
    if (!emissions.length) return null;
    const m = new Map();
    for (let i = 0; i < emissions.length; i++) {
      const e = emissions[i];
      if (e && e.step >= 0) m.set(e.step, e.kind);
    }
    return m;
  }
  // Tick (window column) where a lane step fires, so an off-grid add emission can
  // be placed. Pattern lanes fire step s at tick s*stepLen; cells lanes fire onset
  // index s at its aksak tick. -1 when the step has no position in this window.
  function tickForStep(l, step) {
    if (l.cells) { const os = onsets(l); return step >= 0 && step < os.length ? os[step] : -1; }
    return step * l.stepLen;
  }
  // A dropped on-pattern step: the engine expected a hit here but culled it
  // (probability / mutation). Draw a hollow strike "hole" — never a solid tick —
  // so a drop reads as visibly distinct from an emitted hit.
  function drawDropMarker(g, cellX, cellW, y0, bandH, dp, woven) {
    const mw = Math.max(2 * dp, cellW * 0.6);
    const mh = bandH * 0.5;
    const mx = cellX + (cellW - mw) / 2;
    const my = y0 + (bandH - mh) / 2;
    g.globalAlpha = woven ? 0.9 : 0.18;
    g.strokeStyle = 'rgba(240,234,223,.75)';
    g.lineWidth = 1.5 * dp;
    g.strokeRect(mx, my, mw, mh);
    g.beginPath();
    g.moveTo(mx, my);
    g.lineTo(mx + mw, my + mh);
    g.stroke();
    g.globalAlpha = 1;
  }
  // Macro/mutation-resolved add hits fire off the base positional grid. Draw them
  // at the step's tick where laneHitAt has no onset, capped so they read as extra
  // off-grid hits rather than base pattern. No-op when stepKind is null (no stream).
  function drawEmissionAdds(g, l, stepKind, cellW, tickCount, y0, bandH, dp, wovenFn) {
    if (!stepKind) return;
    stepKind.forEach((kind, step) => {
      if (kind !== 'add') return;
      const e = tickForStep(l, step);
      if (e < 0 || e >= tickCount) return;
      if (laneHitAt(l, e)) return; // already drawn on the positional grid
      const woven = wovenFn(e);
      const bw = Math.max(2 * dp, cellW * 0.45);
      const bh = bandH * 0.45;
      const x = e * cellW + (cellW - bw) / 2;
      const y = y0 + (bandH - bh) / 2;
      g.globalAlpha = woven ? 0.85 : 0.18;
      g.fillStyle = l.hue;
      g.fillRect(x, y, bw, bh);
      g.fillStyle = 'rgba(240,234,223,.6)';
      g.fillRect(x, y - 2 * dp, bw, 1.5 * dp); // off-grid cap tick
      g.globalAlpha = 1;
    });
  }

  // Idle decorative weave: lanes stacked over the convergence window with the
  // step grid, per-lane cycle markers, gold selvage and a sweeping playhead.
  // M073 S05: consults the engine emission stream (laneStepKind) so drops render
  // as holes and ghosts dim even in the idle weave; empty stream => positional only.
  function drawConvergence(g, W, H, dp, t8) {
    const bandH = H / S.lanes.length, colW = W / CONV;
    S.lanes.forEach((l, li) => {
      const y0 = li * bandH;
      g.fillStyle = li % 2 ? '#222E52' : '#26335A';
      g.fillRect(0, y0, W, bandH);
      g.fillStyle = 'rgba(240,234,223,.05)';
      for (let x = 0; x < CONV; x += 2) g.fillRect(x * colW, y0, 1 * dp, bandH);
      const stepKind = laneStepKind(li);
      for (let e = 0; e < CONV; e++) {
        const hit = laneHitAt(l, e);
        if (!hit) continue;
        const kind = stepKind ? stepKind.get(hit.step) : undefined;
        // Emission-truthful drop: engine culled this on-pattern step. Draw a
        // hole, not a tick. Guarded by stepKind (a muted lane records no
        // emission, so it never reaches here and is never drawn as a drop).
        if (kind === 'drop') { drawDropMarker(g, e * colW, colW, y0, bandH, dp, true); continue; }
        const vn = hitVelocity(l, li, e, hit);
        if (vn <= 0) continue; // baseVelocity 0 mutes the lane: draw no hit (M073 S01)
        const ghost = kind === 'ghost'; // ghost emissions render dimmed vs base
        const wUnits = l.cells ? l.cells[hit.step] : l.stepLen;
        const bw = colW * wUnits * 0.86;
        const bh = bandH * Math.min(0.92, 0.3 + vn * 0.52);
        const x = e * colW + colW * wUnits * 0.07;
        const y = y0 + (bandH - bh) / 2;
        g.globalAlpha = (hit.step === 0 ? 1 : 0.86) * (ghost ? 0.4 : 1);
        g.fillStyle = l.hue;
        g.fillRect(x, y, bw, bh);
        g.globalAlpha = 0.22 * (ghost ? 0.4 : 1);
        g.fillStyle = '#0E1526';
        for (let ty = y + 3 * dp; ty < y + bh; ty += 6 * dp) g.fillRect(x, ty, bw, 1.5 * dp);
        g.globalAlpha = 1;
        if (hit.step === 0 && !ghost) {
          g.fillStyle = 'rgba(240,234,223,.85)';
          g.fillRect(x, y0 + bandH * 0.08, 2 * dp, bandH * 0.84);
        }
      }
      drawEmissionAdds(g, l, stepKind, colW, CONV, y0, bandH, dp, () => true);
      g.fillStyle = 'rgba(240,234,223,.16)';
      for (let e = 0; e < CONV; e += cyc8(l)) g.fillRect(e * colW, y0, 1.5 * dp, bandH);
      g.fillStyle = 'rgba(14,21,38,.55)';
      g.fillRect(0, y0 + bandH - 2 * dp, W, 2 * dp);
    });
    g.fillStyle = '#D9A441';
    g.fillRect(W - 4 * dp, 0, 4 * dp, H);
    if (lastFrame.playing && lastFrame.convLeft <= 4) {
      g.globalAlpha = 0.5 * (1 - (lastFrame.convLeft - 1) / 4);
      g.fillStyle = '#E8A33D';
      g.fillRect(W - 26 * dp, 0, 26 * dp, H);
      g.globalAlpha = 1;
    }
    const sx = ((t8 % CONV) / CONV) * W;
    g.fillStyle = 'rgba(240,234,223,.9)';
    g.fillRect(sx, 0, 2 * dp, H);
    g.beginPath();
    g.moveTo(sx - 9 * dp, 0);
    g.lineTo(sx + 11 * dp, 0);
    g.lineTo(sx + 1 * dp, 14 * dp);
    g.closePath();
    g.fill();
  }

  // Bar-anchored capture timeline. M073 S05: ticks render the engine emission
  // stream (host.getLaneEmissions via laneStepKind) — a dropped on-pattern step
  // draws a hole, ghosts dim, macro-resolved adds appear off the positional grid.
  // Where the stream is absent (mock default / DAW / stopped) it degrades to the
  // positional pattern via laneHitAt and draws no drop/ghost marker (never claims
  // an unobservable drop). Captured ticks are solid, not-yet-woven ticks dimmed.
  function drawCaptureTimeline(g, W, H, dp, bars, capState, prog) {
    const bandH = H / S.lanes.length;
    const barW = W / bars;
    const ticks = bars * eighthsPerBar();
    const tickW = W / ticks;
    // A tick is "woven" once the playhead has passed it (or always, at complete).
    const isWoven = (e) => capState === 3 || e / ticks <= prog / bars;
    S.lanes.forEach((l, li) => {
      const y0 = li * bandH;
      g.fillStyle = li % 2 ? '#222E52' : '#26335A';
      g.fillRect(0, y0, W, bandH);
      g.fillStyle = 'rgba(240,234,223,.05)';
      for (let e = 0; e < ticks; e++) g.fillRect(e * tickW, y0, 1 * dp, bandH);
      const stepKind = laneStepKind(li);
      for (let e = 0; e < ticks; e++) {
        const hit = laneHitAt(l, e);
        if (!hit) continue;
        // Ahead-of-playhead ticks are faint to read as pending.
        const woven = isWoven(e);
        const kind = stepKind ? stepKind.get(hit.step) : undefined;
        // Emission-truthful drop: engine culled this on-pattern step. Draw a
        // hole, not a tick. Guarded by stepKind, so a muted lane (no recorded
        // emission => empty stream => null stepKind) is never drawn as a drop.
        if (kind === 'drop') { drawDropMarker(g, e * tickW, tickW, y0, bandH, dp, woven); continue; }
        const vn = hitVelocity(l, li, e, hit);
        if (vn <= 0) continue; // baseVelocity 0 mutes the lane: draw no hit (M073 S01)
        const ghost = kind === 'ghost'; // ghost emissions render dimmed vs base
        const bw = Math.max(2 * dp, tickW * 0.7);
        const bh = bandH * Math.min(0.9, 0.32 + vn * 0.5);
        const x = e * tickW + (tickW - bw) / 2;
        const y = y0 + (bandH - bh) / 2;
        g.globalAlpha = (woven ? 1 : 0.18) * (ghost ? 0.4 : 1);
        g.fillStyle = l.hue;
        g.fillRect(x, y, bw, bh);
        g.globalAlpha = 1;
      }
      drawEmissionAdds(g, l, stepKind, tickW, ticks, y0, bandH, dp, isWoven);
      g.fillStyle = 'rgba(14,21,38,.55)';
      g.fillRect(0, y0 + bandH - 2 * dp, W, 2 * dp);
    });
    // Bar dividers + bar-number ruler (X = bars 1..N).
    g.fillStyle = 'rgba(240,234,223,.20)';
    for (let b = 1; b < bars; b++) g.fillRect(b * barW, 0, 1 * dp, H);
    g.fillStyle = 'rgba(240,234,223,.6)';
    g.font = `${11 * dp}px system-ui, -apple-system, sans-serif`;
    g.textBaseline = 'top';
    for (let b = 0; b < bars; b++) g.fillText(String(b + 1), b * barW + 3 * dp, 2 * dp);
    // Gold selvage marks bar N — brighter/thicker once the window is frozen.
    const complete = capState === 3;
    g.fillStyle = complete ? '#F0C24A' : '#D9A441';
    const selW = (complete ? 6 : 4) * dp;
    g.fillRect(W - selW, 0, selW, H);
    // Playhead sweeps L->R while capturing.
    if (capState === 2) {
      const px = (prog / bars) * W;
      g.fillStyle = 'rgba(240,234,223,.92)';
      g.fillRect(px, 0, 2 * dp, H);
      g.beginPath();
      g.moveTo(px - 9 * dp, 0);
      g.lineTo(px + 11 * dp, 0);
      g.lineTo(px + 1 * dp, 14 * dp);
      g.closePath();
      g.fill();
    }
  }

  // Cloth chrome reflects the capture state: annotations are the idle story, so
  // they hide once the cloth becomes a functional capture timeline. The capline
  // narrates the arm->capture->complete progression.
  function updateClothChrome(capState, bars, prog) {
    if (clothEl) {
      clothEl.classList.toggle('capArmed', capState === 1);
      clothEl.classList.toggle('capturing', capState === 2);
      clothEl.classList.toggle('capComplete', capState === 3);
    }
    const timeline = capState >= 1;
    clothAnns.forEach((a) => { if (a) a.style.display = timeline ? 'none' : ''; });
    if (capline) {
      if (capState === 1) capline.innerHTML = `Armed · <b>${bars} bars</b> · waiting for bar 1`;
      else if (capState === 2) capline.innerHTML = `Capturing · bar <b>${Math.min(bars, Math.floor(prog) + 1)}/${bars}</b>`;
      else if (capState === 3) capline.innerHTML = `Complete · <b>${bars} bars</b> · drag cloth to DAW`;
      else capline.innerHTML = `Capture · <b>${bars} bars</b> · arm to weave`;
    }
  }

  /* ================= desk ================= */
  const desk = document.getElementById('desk');
  function el(t, at, parent) {
    const e = document.createElementNS(NS, t);
    for (const k in at) e.setAttribute(k, at[k]);
    parent.appendChild(e);
    return e;
  }
  // G02: inline lane rename. Mirrors the native double-click-name gesture
  // (lane_edit_view.cpp beginNameEdit/commitNameEdit): prefill with the current
  // name, cap at 15 chars, commit non-empty on Enter/blur, discard on Escape.
  // The setLaneName action round-trips through the bridge; the resulting state
  // push rebuilds the strip head (buildDesk) with the committed name, so no
  // manual DOM update is needed on a successful commit.
  function beginLaneRename(li, nameEl) {
    if (nameEl.querySelector('input')) return; // already editing
    const current = (S.lanes[li] && S.lanes[li].name) || '';
    const input = document.createElement('input');
    input.className = 'nm-edit';
    input.type = 'text';
    input.maxLength = 15;
    input.value = current;
    input.setAttribute('aria-label', `Rename lane ${li + 1}`);
    nameEl.textContent = '';
    nameEl.appendChild(input);
    input.focus();
    input.select();
    let done = false;
    const finish = (commit) => {
      if (done) return;
      done = true;
      const name = input.value.trim().slice(0, 15);
      // Non-empty names commit through the bridge (empty is dropped, matching
      // the native/bridge invariant); anything else restores the prior label.
      if (commit && name) host.action('setLaneName', { lane: li, name });
      else nameEl.textContent = current;
    };
    input.addEventListener('keydown', (ev) => {
      if (ev.key === 'Enter') { ev.preventDefault(); finish(true); }
      else if (ev.key === 'Escape') { ev.preventDefault(); finish(false); }
    });
    input.addEventListener('blur', () => finish(true));
  }
  function buildDesk() {
    desk.innerHTML = '';
    strips = []; rings = []; hands = []; ladders = []; vus = []; playeds = [];
    // M045 S01 T03: ladder DOM was just recreated — invalidate the overlay
    // diff cache so the next onFrame paints classes onto the fresh buttons.
    overlayLastKind.length = 0;
    S.lanes.forEach((l, li) => {
      const s = document.createElement('div');
      s.className = 'strip';
      s.dataset.lane = li;
      s.style.setProperty('--c', l.hue);
      s.innerHTML = `
        <div class="head">
          <div class="nm"><b>${l.name}</b>${l.role}</div>
          <button class="mute${l.active ? '' : ' off'}" data-mute aria-label="Mute ${l.name}" title="Mute">●</button>
          ${CAN_EXPORT ? `<button class="lex" draggable="true" data-lex aria-label="Export ${l.name} lane" title="Export or drag just this lane to your DAW">⇱</button>` : ''}
          <button class="ex" aria-label="Expand ${l.name} strip" title="Expand">⤢</button>
        </div>
        <div class="body2">
          <div class="core">
            <svg class="ring" viewBox="0 0 64 64" aria-hidden="true"></svg>
            <div class="laddwrap">
              <div class="ladder" role="group" aria-label="${l.name} steps"></div>
              <div class="played" aria-hidden="true" title="What's actually played — dots sit at the real onset (syncopation, swing, humanize shift them off the grid)"></div>
            </div>
            <div class="vu"><i></i></div>
          </div>
          <div class="deep">
            <div class="tabs">
              <button data-tab="pattern" class="on">Pattern</button>
              <button data-tab="timing">Timing</button>
              <button data-tab="env">Envelopes</button>
              <button data-tab="expr">Expr</button>
              <button data-tab="adv">Adv</button>
            </div>
            <div class="pane on" data-pane="pattern"></div>
            <div class="pane" data-pane="timing"></div>
            <div class="pane" data-pane="env"></div>
            <div class="pane" data-pane="expr"></div>
            <div class="pane" data-pane="adv"></div>
          </div>
        </div>
        <div class="feel"></div>
        <div class="stat">${l.ch < 0 ? 'Auto' : 'CH ' + l.ch} · N${l.note}</div>`;
      desk.appendChild(s);
      strips.push(s);
      const svg = s.querySelector('svg.ring');
      el('circle', { cx: 32, cy: 32, r: 24, fill: 'none', stroke: '#2B251E', 'stroke-width': 1 }, svg);
      rings.push(el('g', {}, svg));
      hands.push(el('line', { x1: 32, y1: 32, x2: 32, y2: 9, stroke: '#F0EADF', 'stroke-width': 1.2, opacity: 0.85 }, svg));
      ladders.push(s.querySelector('.ladder'));
      playeds.push(s.querySelector('.played'));
      vus.push(s.querySelector('.vu i'));
      s.querySelector('.ex').addEventListener('click', () => expandStrip(expanded === li ? -1 : li));
      const nameEl = s.querySelector('.nm b');
      if (nameEl) {
        nameEl.title = 'Double-click to rename';
        nameEl.addEventListener('dblclick', () => beginLaneRename(li, nameEl));
      }
      s.querySelector('[data-mute]').addEventListener('click', () => {
        const active = S.lanes[li].active;
        host.edit(`lane.${li}.active`, active ? 0 : 1, 'begin');
        host.edit(`lane.${li}.active`, active ? 0 : 1, 'perform');
        host.edit(`lane.${li}.active`, active ? 0 : 1, 'end');
      });
      // M032 S02 (T03): per-lane export/drag. A click saves just this lane
      // (exportSaveAs {lane}); a drag hands just this lane's .mid to the DAW
      // (beginMidiDrag {lane}). Both carry {lane: li} so the native handler
      // forwards laneFilter into renderPatternToSMF — the global Export chip
      // keeps firing an empty payload (all lanes). Plugin-only (CAN_EXPORT).
      const lex = s.querySelector('[data-lex]');
      if (lex) {
        lex.addEventListener('click', (e) => {
          e.stopPropagation();
          host.action('exportSaveAs', { lane: li });
        });
        lex.addEventListener('dragstart', (e) => {
          e.stopPropagation();
          host.action('beginMidiDrag', { lane: li });
          lex.classList.add('dragging');
        });
        lex.addEventListener('dragend', () => lex.classList.remove('dragging'));
      }
      s.querySelectorAll('.tabs button').forEach((tb) =>
        tb.addEventListener('click', () => {
          tabState[li] = tb.dataset.tab;
          s.querySelectorAll('.tabs button').forEach((x) => x.classList.toggle('on', x === tb));
          s.querySelectorAll('.pane').forEach((p) => p.classList.toggle('on', p.dataset.pane === tb.dataset.tab));
        }));
      const saved = tabState[li];
      if (saved && saved !== 'pattern') {
        s.querySelectorAll('.tabs button').forEach((tb) => tb.classList.toggle('on', tb.dataset.tab === saved));
        s.querySelectorAll('.pane').forEach((p) => p.classList.toggle('on', p.dataset.pane === saved));
      }
      refreshStrip(li);
    });
    const m = document.createElement('div');
    m.id = 'master';
    const macros = S.macros;
    const MACRO_PARAMS = [
      { name: 'Complexity', id: 'macro.complexity', v: macros.complexity },
      { name: 'Density',    id: 'macro.density',    v: macros.density },
      { name: 'Syncopation',id: 'macro.syncopation',v: macros.syncopation },
      { name: 'Swing',      id: 'macro.swing',      v: macros.swing },
      { name: 'Tension',    id: 'macro.tension',    v: macros.tension },
      { name: 'Humanize',   id: 'macro.humanize',   v: macros.humanize },
    ];
    m.innerHTML = `<h3>Master</h3>
      ${MACRO_PARAMS.map(({ name, id, v }) =>
        `<div class="macro"><div class="t"><span>${name}</span><b>${Math.round(v * 100)}</b></div>
          <div class="slider-track" data-macro="${id}"><i style="width:${v * 100}%"></i></div></div>`
      ).join('')}
      <div id="cmeter"><i></i><span>Convergence</span></div>`;
    desk.appendChild(m);
    m.querySelectorAll('.slider-track').forEach((track) => {
      const paramId = track.dataset.macro;
      const fill = track.querySelector('i');
      const vSpan = track.closest('.macro').querySelector('b');
      const calc = (e) => {
        const r = track.getBoundingClientRect();
        return Math.max(0, Math.min(1, (e.clientX - r.left) / r.width));
      };
      const paint = (v) => { fill.style.width = `${(v * 100).toFixed(1)}%`; vSpan.textContent = Math.round(v * 100); };
      track.addEventListener('pointerdown', (e) => {
        e.preventDefault();
        let lastV = calc(e); paint(lastV);
        host.edit(paramId, lastV, 'begin');
        host.edit(paramId, lastV, 'perform');
        const mv = (ev) => { lastV = calc(ev); paint(lastV); host.edit(paramId, lastV, 'perform'); };
        const up = () => {
          window.removeEventListener('pointermove', mv);
          host.edit(paramId, lastV, 'end');
        };
        window.addEventListener('pointermove', mv);
        window.addEventListener('pointerup', up, { once: true });
      });
    });

    // M053 S07 (F1): lane add/remove. The desk previously rendered exactly the
    // lanes a preset shipped, with no affordance to grow or shrink the ensemble,
    // even though the activeLaneCount param (1..8 lanes) is wired end-to-end
    // through the bridge (bridge_params.h -> web_ui_view.cpp -> processor.cpp).
    // This footer edits that param; the host round-trips a new state with a
    // different lane count and refreshAll() rebuilds the desk.
    const laneCount = S.lanes.length;
    const mgr = document.createElement('div');
    mgr.className = 'lane-mgr';
    mgr.innerHTML =
      `<button class="lane-del" ${laneCount <= 1 ? 'disabled' : ''} aria-label="Remove last lane" title="Remove the last lane">− Lane</button>` +
      `<span class="lane-count" aria-live="polite">${laneCount} / 8 lanes</span>` +
      `<button class="lane-add" ${laneCount >= 8 ? 'disabled' : ''} aria-label="Add lane" title="Add a lane">+ Lane</button>`;
    desk.appendChild(mgr);
    const setLaneCount = (c) => {
      const norm = (c - 1) / 7;
      host.edit('activeLaneCount', norm, 'begin');
      host.edit('activeLaneCount', norm, 'perform');
      host.edit('activeLaneCount', norm, 'end');
    };
    mgr.querySelector('.lane-add').addEventListener('click', () => {
      const c = S.lanes.length;
      if (c < 8) setLaneCount(c + 1);
    });
    mgr.querySelector('.lane-del').addEventListener('click', () => {
      const c = S.lanes.length;
      if (c > 1) setLaneCount(c - 1);
    });

    desk.classList.remove('lane-expanded');
    desk.style.gridTemplateColumns = `repeat(${S.lanes.length}, 1fr) 160px`;
  }
  function expandStrip(li) {
    expanded = li;
    strips.forEach((s, i) => s.classList.toggle('expanded', i === li));
    const n = S.lanes.length;
    const wide = n >= 6 ? '2.55fr' : '1.9fr';
    const narrow = n >= 6 ? '.38fr' : '.62fr';
    const master = n >= 6 ? '148px' : '160px';
    // M031 S02 T02: the expanded strip must stay wide enough for its full
    // 5-button tab row. The buttons don't shrink (no flex) and .strip clips
    // overflow, so once the strip's .deep column falls below the row's natural
    // 320px the trailing tabs ("Adv") clip off the right edge — at 6-8 lanes
    // the bare fr share shrank the strip to ~436px (.deep 292px < 320). Floor
    // the expanded column so .deep clears the row's natural width: strip = 320
    // (tabs) + 104 (core col) + 10 (body2 gap) + 28 (strip 14px L/R padding) =
    // 462. The 320px tab-row figure was measured on macOS Chromium; the headless
    // Ubuntu runner (webui-tests CI) renders the same 5 buttons ~8px wider, so a
    // 470px floor left .deep (~326px) only ~6px of slack — which the Ubuntu
    // glyph widening consumed, clipping the "Adv" tab by 2px on CI while passing
    // locally. Floor at 484 so .deep is ~340px: comfortably above the ~334px
    // Ubuntu natural width with headroom for future font-rendering jitter.
    // minmax() keeps the fr as the max so the strip still grows on wide
    // viewports; collapsed columns keep their fr and shrink to absorb the
    // floor (T03 gives collapsed strips their own usable floor + scroll).
    const EXPANDED_FLOOR_PX = 484;
    const wideCol = `minmax(${EXPANDED_FLOOR_PX}px, ${wide})`;
    // M031 S02 T03: floor each collapsed column so a strip never crushes below
    // ring legibility. T01 measured the usable floor at 84px (64px ring + 20px
    // L/R padding); at the 8-lane worst case the bare .38fr share shrank the
    // collapsed strips to ~60px (an unreadable sliver). minmax(88px, <fr>) keeps
    // the fr as the max — so collapsed strips stay elastic and the layout is
    // unchanged whenever they fit — but pins the min at 88px (84px floor + 4px
    // margin against sub-pixel jitter). When the floored columns no longer fit
    // (6-8 lanes with a lane expanded) the desk overflows horizontally instead
    // of squeezing; the .lane-expanded class below turns on overflow-x:auto so
    // the collapsed strips scroll behind a scrollbar affordance rather than
    // silently shrinking. This only applies while a lane is expanded — the
    // collapsed no-expand layout (li < 0 branch) is untouched for 1-8 lanes.
    const COLLAPSED_FLOOR_PX = 88;
    const narrowCol = `minmax(${COLLAPSED_FLOOR_PX}px, ${narrow})`;
    desk.classList.toggle('lane-expanded', li >= 0);
    desk.style.gridTemplateColumns = li < 0
      ? `repeat(${n}, 1fr) 160px`
      : S.lanes.map((_, i) => (i === li ? wideCol : narrowCol)).join(' ') + ` ${master}`;
    if (li >= 0) buildPanes(li);
  }
  function refreshStrip(li) {
    const l = S.lanes[li];
    strips[li].classList.toggle('muted', !l.active);
    const muteBtn = strips[li].querySelector('[data-mute]');
    if (muteBtn) muteBtn.classList.toggle('off', !l.active);
    drawRing(li);
    buildLadder(li);
    strips[li].querySelector('.feel').innerHTML = `
      <div><span>VEL</span><b>${l.vel}</b></div>
      <div><span>PROB</span><b>${Math.round(l.prob * 100)}%</b></div>
      <div><span>PUSH</span><b>${l.push > 0 ? '+' : ''}${l.push}ms</b></div>
      <div><span>MODE</span><b>${l.timeline ? 'TIMELINE' : l.cells ? 'CELLS ' + l.cells.join('+') : `E(${l.pattern.filter(Boolean).length},${l.steps})`}</b></div>`;
    const tag = tags.children[li];
    if (tag) tag.innerHTML = `${l.name}<small>${cyc8(l)} picks</small>`;
    if (mode === 'cloth') drawLoom(lastFrame.t8);
  }
  function drawRing(li) {
    const l = S.lanes[li], g = rings[li];
    g.innerHTML = '';
    const put = (frac, on, big) => {
      const a = frac * Math.PI * 2 - Math.PI / 2;
      const e = document.createElementNS(NS, 'circle');
      e.setAttribute('cx', 32 + 24 * Math.cos(a));
      e.setAttribute('cy', 32 + 24 * Math.sin(a));
      e.setAttribute('r', on ? (big ? 3.4 : 2.8) : 0.9);
      e.setAttribute('fill', on ? l.hue : '#4A4238');
      g.appendChild(e);
    };
    if (l.cells) {
      const tot = cyc8(l);
      onsets(l).forEach((o, i) => put(o / tot, true, i === 0));
    } else {
      for (let i = 0; i < l.steps; i++) put(i / l.steps, !!l.pattern[i], i === 0);
    }
  }
  function buildLadder(li) {
    const l = S.lanes[li], lad = ladders[li];
    lad.innerHTML = '';
    const n = l.cells ? l.cells.length : l.steps;
    for (let i = 0; i < n; i++) {
      const b = document.createElement('button');
      b.style.flexGrow = l.cells ? String(l.cells[i]) : '1';
      b.style.flexBasis = '0';
      if (l.cells || l.pattern[i]) b.classList.add('hit');
      b.setAttribute('aria-label', `${l.name} step ${i + 1}`);
      if (l.timeline) {
        b.addEventListener('click', () => host.action('toggleStep', { lane: li, step: i }));
      } else if (!l.cells) {
        b.style.cursor = 'default';
      }
      lad.appendChild(b);
    }
  }

  /* ================= deep panes ================= */
  function buildPanes(li) {
    const l = S.lanes[li], s = strips[li];
    const pat = s.querySelector('[data-pane="pattern"]');
    const tim = s.querySelector('[data-pane="timing"]');
    const env = s.querySelector('[data-pane="env"]');

    /* PATTERN */
    let html = '';
    if (l.timeline) {
      html += `<div class="prow"><label>Timeline mode</label><span class="switch"><button class="on" data-tl aria-label="Timeline mode"><i></i></button></span></div>
        <div class="hint">Fixed pattern — immune to macros. The bell is law. Edit steps on the ladder above.</div>`;
    } else {
      html += `<div class="prow"><label>Timeline mode</label><span class="switch"><button data-tl aria-label="Timeline mode"><i></i></button></span></div>
        <div class="prow"><label>Steps</label><span class="stepper"><button data-st="-1">−</button><span class="v">${l.steps} × ${l.stepLen === 2 ? '♩' : '♪'}</span><button data-st="1">+</button></span></div>
        <div class="prow"><label>Hits</label><span class="stepper"><button data-ht="-1">−</button><span class="v">E(${l.hits},${l.steps})</span><button data-ht="1">+</button></span></div>
        <div class="prow"><label>Rotation</label><span class="stepper"><button data-rt="-1">−</button><span class="v">${l.rot}</span><button data-rt="1">+</button></span></div>
        <div class="prow"><label>Additive cells</label><span class="switch"><button ${l.cells ? 'class="on"' : ''} data-cl aria-label="Additive cells"><i></i></button></span></div>`;
      if (l.cells)
        html += `<div class="cells" data-cells>${l.cells.map((c, i) => `<button class="onc" style="flex:${c} 1 0" data-cellstep="${i}" data-i="${i}" title="drag up/down · scroll · click to set 1-16">${c}</button>`).join('')}<button data-addcell style="flex:.6 1 0">+</button></div>
          <div class="hint">aksak grouping — drag a cell up/down or scroll to set 1-16 · cycle = ${cyc8(l)}♪ (${l.cells.join('+')})</div>`;
    }
    pat.innerHTML = html;
    const tlBtn = pat.querySelector('[data-tl]');
    if (tlBtn) {
      tlBtn.addEventListener('click', () => {
        const next = l.timeline ? 0.0 : 1.0;
        host.edit(`lane.${li}.timeline`, next, 'begin');
        host.edit(`lane.${li}.timeline`, next, 'perform');
        host.edit(`lane.${li}.timeline`, next, 'end');
      });
    }
    if (!l.timeline) {
      pat.querySelectorAll('[data-st]').forEach((b) =>
        b.addEventListener('click', () => host.action('setEuclid', { lane: li, steps: l.steps + +b.dataset.st })));
      pat.querySelectorAll('[data-ht]').forEach((b) =>
        b.addEventListener('click', () => host.action('setEuclid', { lane: li, hits: l.hits + +b.dataset.ht })));
      pat.querySelectorAll('[data-rt]').forEach((b) =>
        b.addEventListener('click', () => host.action('setEuclid', { lane: li, rotation: l.rot + +b.dataset.rt })));
      const clBtn = pat.querySelector('[data-cl]');
      if (clBtn)
        clBtn.addEventListener('click', () =>
          host.action('setCells', { lane: li, cells: l.cells ? null : [2, 2, 3] }));
      const cellsEl = pat.querySelector('[data-cells]');
      if (cellsEl) {
        // Additive cells are editable across the full 1–16 range (native parity),
        // replacing the old three-value click-cycle. Each cell supports vertical drag
        // (bottom = 1, top = 16), scroll-wheel stepping, and a plain click that
        // steps +1 (wrapping 16→1). Reads/writes go through the live global state
        // (S.lanes[li]) rather than the render-closure `l`, so a mid-drag state
        // re-render (which rebuilds this row) can't leave us acting on stale data.
        const setCell = (i, val) => {
          const cur = S.lanes[li] && S.lanes[li].cells;
          if (!cur) return;
          const v = Math.max(1, Math.min(16, val));
          if (cur[i] === v) return;
          const next = cur.slice();
          next[i] = v;
          host.action('setCells', { lane: li, cells: next });
        };
        cellsEl.querySelectorAll('button[data-cellstep]').forEach((b) => {
          const i = +b.dataset.cellstep;
          const dragSet = (e) => {
            const live = document.querySelector(`[data-cells] button[data-cellstep="${i}"]`) || b;
            const r = live.getBoundingClientRect();
            if (!r.height) return;
            const f = Math.max(0, Math.min(1, (r.bottom - e.clientY) / r.height));
            setCell(i, Math.round(1 + f * 15));
          };
          b.addEventListener('pointerdown', (e) => {
            b.setPointerCapture(e.pointerId);
            const startY = e.clientY;
            let moved = false;
            const mv = (ev) => {
              if (!moved && Math.abs(ev.clientY - startY) < 4) return;
              moved = true;
              dragSet(ev);
            };
            b.addEventListener('pointermove', mv);
            b.addEventListener('pointerup', () => {
              b.removeEventListener('pointermove', mv);
              const cur = S.lanes[li] && S.lanes[li].cells;
              if (!moved && cur) setCell(i, cur[i] >= 16 ? 1 : cur[i] + 1);
            }, { once: true });
          });
          b.addEventListener('wheel', (e) => {
            e.preventDefault();
            const cur = S.lanes[li] && S.lanes[li].cells;
            if (cur) setCell(i, cur[i] + (e.deltaY < 0 ? 1 : -1));
          }, { passive: false });
        });
        cellsEl.querySelector('[data-addcell]').addEventListener('click', () => {
          const next = l.cells.length < 6 ? l.cells.concat([2]) : l.cells.slice(0, 2);
          host.action('setCells', { lane: li, cells: next });
        });
      }
    }

    /* ACCENTS */
    const accentSteps = l.cells ? l.cells.length : l.steps;
    const accentHtml = `<div class="section-label">Accents</div>
      <div class="accent-row" data-accents>${Array.from({ length: accentSteps }, (_, i) =>
        `<button class="accent-btn${l.accents[i] > 0 ? ' on' : ''}" data-acc="${i}" aria-label="Accent step ${i + 1}"></button>`).join('')}</div>
      <div class="hint">Toggle accent emphasis per step</div>`;
    pat.insertAdjacentHTML('beforeend', accentHtml);
    pat.querySelectorAll('[data-acc]').forEach((b) =>
      b.addEventListener('click', () => {
        const si = parseInt(b.dataset.acc);
        const cur = l.accents[si] > 0 ? 0 : 1;
        host.action('setAccent', { lane: li, step: si, value: cur });
      }));

    /* TIMING */
    const items = l.cells ? l.cells.length : l.steps;
    tim.innerHTML = `<div class="prow"><label>Push / pull (lane)</label><span class="v">${l.push > 0 ? '+' : ''}${l.push} ms</span></div>
      <div class="mtwrap"><div class="prow"><label>Per-step micro-timing</label><span class="v" data-mtv>±20 ms</span></div>
      <div class="mtbars" data-mt>${Array.from({ length: items }, () => '<div class="mb"><i></i></div>').join('')}</div>
      <div class="hint">drag a bar — up = late, down = early — audio follows</div></div>`;
    const bars = tim.querySelectorAll('.mb');
    const paint = (i) => {
      const v = l.mt[i] || 0, elx = bars[i].querySelector('i');
      const h = (Math.abs(v) / 20) * 50;
      elx.style.height = `${h}%`;
      elx.style.top = v >= 0 ? `${50 - h}%` : '50%';
    };
    for (let i = 0; i < items; i++) paint(i);
    const mtv = tim.querySelector('[data-mtv]');
    bars.forEach((b, i) => {
      const set = (e) => {
        const r = b.getBoundingClientRect();
        const f = Math.max(-1, Math.min(1, (r.top + r.height / 2 - e.clientY) / (r.height / 2)));
        const ms = Math.round(f * 20);
        host.action('setMicroTiming', { lane: li, step: i, ms });
        paint(i);
        mtv.textContent = `step ${i + 1}: ${ms > 0 ? '+' : ''}${ms} ms`;
      };
      b.addEventListener('pointerdown', (e) => {
        b.setPointerCapture(e.pointerId);
        set(e);
        const mv = (ev) => set(ev);
        b.addEventListener('pointermove', mv);
        b.addEventListener('pointerup', () => b.removeEventListener('pointermove', mv), { once: true });
      });
      // G08: double-click resets this step's micro-timing to zero (matches native
      // micro_timing_editor_view's one-gesture reset) via the same setMicroTiming path.
      b.addEventListener('dblclick', () => {
        host.action('setMicroTiming', { lane: li, step: i, ms: 0 });
        paint(i);
        mtv.textContent = `step ${i + 1}: 0 ms`;
      });
    });

    /* ENVELOPES */
    // The preview box shows a FIXED WINDOW of PERIOD_MAX bars of timeline, so
    // one cycle's WAVELENGTH is directly proportional to the period: a longer
    // period draws a physically WIDER wave (fewer humps across the box), a
    // shorter period a tighter one (more humps). At period == PERIOD_MAX exactly
    // one full cycle fills the width; at period 1 there are PERIOD_MAX cycles.
    // This keeps frequency intuitive — more bars visibly means a slower, wider
    // wave — matching the playhead (data-envph), which also sweeps the box once
    // per PERIOD_MAX bars so a longer-period cycle takes proportionally longer.
    const PERIOD_MAX = 16;
    const envPath = (depth, period) => {
      // cyclesInBox = PERIOD_MAX / period → wavelength ∝ period.
      const cycles = PERIOD_MAX / Math.max(1, period || 1);
      let d = 'M0 15';
      for (let x = 0; x <= 74; x += 1)
        d += ` L${x} ${(15 - Math.sin((x / 74) * Math.PI * 2 * cycles) * 11 * depth).toFixed(1)}`;
      return d;
    };
    // Period is edited as an integer bar count in [1, PERIOD_MAX]; the clamp
    // (Math.max(1, …)) guarantees the emitted setEnvelope never carries period<=0.
    const periodToNorm = (p) => Math.max(0, Math.min(1, (Math.max(1, p || 1) - 1) / (PERIOD_MAX - 1)));
    const normToPeriod = (n) => Math.max(1, Math.round(1 + Math.max(0, Math.min(1, n)) * (PERIOD_MAX - 1)));
    // Display the stored period as a clamped integer bar count in [1, PERIOD_MAX].
    // (normToPeriod maps a 0..1 norm, not a bar count, so a bar count must not be
    // passed to it for display — that would misread e.g. period 4 as "16 bars".)
    const clampPeriod = (p) => Math.max(1, Math.min(PERIOD_MAX, Math.round(p || 1)));
    const curve = (e, id) =>
      `<svg class="envcurve" data-envdepth="${id}" viewBox="0 0 74 30" role="slider" tabindex="0" aria-label="Envelope ${id + 1} depth (drag up/down, right-click to reset)" aria-valuemin="0" aria-valuemax="100" aria-valuenow="${Math.round(e.depth * 100)}"><path d="${envPath(e.depth, e.period)}" fill="none" stroke="${l.hue}" stroke-width="1.4" opacity="${e.on ? 0.95 : 0.3}"/><line data-envph="${id}" x1="0" y1="2" x2="0" y2="28" stroke="#F0EADF" stroke-width="1" opacity="${e.on ? 0.7 : 0}"/></svg>`;
    env.innerHTML =
      l.envs.map((e, i) => `
        <div class="envrow">
          <div class="t">${e.target} · <span style="color:var(--dim)" data-envmeta="${i}">${clampPeriod(e.period)} bars · sine</span></div>
          ${curve(e, i)}
          <div class="m">depth <span data-envdepthval="${i}">${Math.round(e.depth * 100)}%</span> <button data-envon="${i}" class="chip ${e.on ? 'on' : ''}" style="padding:2px 8px">${e.on ? 'ON' : 'OFF'}</button></div>
          <div class="param-slider envctl"><label>Period</label><div class="slider-track" data-envperiod="${i}" role="slider" tabindex="0" aria-label="Envelope ${i + 1} period in bars" aria-valuemin="1" aria-valuemax="${PERIOD_MAX}" aria-valuenow="${clampPeriod(e.period)}"><i style="width:${(periodToNorm(e.period) * 100).toFixed(1)}%"></i></div><span class="v" data-envperiodval="${i}">${clampPeriod(e.period)} bars</span></div>
          <div class="param-slider envctl"><label>Depth</label><div class="slider-track" data-envdepthslider="${i}" role="slider" tabindex="0" aria-label="Envelope ${i + 1} depth" aria-valuemin="0" aria-valuemax="100" aria-valuenow="${Math.round(e.depth * 100)}"><i style="width:${(e.depth * 100).toFixed(1)}%"></i></div><span class="v" data-envdepthsliderval="${i}">${Math.round(e.depth * 100)}%</span></div>
        </div>`).join('') +
      `<button class="addenv">+ add envelope</button>
       <div class="hint">drag a curve up/down to set depth · right-click resets to 100% · edit period and depth with the sliders · envelopes superimpose</div>`;
    env.querySelectorAll('[data-envon]').forEach((b) =>
      b.addEventListener('click', () => {
        const i = +b.dataset.envon;
        const e = Object.assign({}, l.envs[i], { on: !l.envs[i].on });
        host.action('setEnvelope', { lane: li, index: i, envelope: e });
      }));
    // Continuous depth editing: vertical drag scrubs depth (up = deeper),
    // right-click resets to 1.0 — mirrors native envelope_curve_view (setEnvelope bridge).
    env.querySelectorAll('[data-envdepth]').forEach((svg) => {
      const i = +svg.dataset.envdepth;
      const path = svg.querySelector('path');
      const valSpan = env.querySelector(`[data-envdepthval="${i}"]`);
      const slider = env.querySelector(`[data-envdepthslider="${i}"]`);
      const sliderFill = slider ? slider.querySelector('i') : null;
      const sliderVal = env.querySelector(`[data-envdepthsliderval="${i}"]`);
      const emit = (depth) => {
        const e = Object.assign({}, l.envs[i], { depth });
        l.envs[i] = e;
        host.action('setEnvelope', { lane: li, index: i, envelope: e });
      };
      const repaint = (depth) => {
        const pct = Math.round(depth * 100);
        path.setAttribute('d', envPath(depth, l.envs[i].period));
        if (valSpan) valSpan.textContent = `${pct}%`;
        svg.setAttribute('aria-valuenow', String(pct));
        // Keep the explicit depth slider in lock-step with the curve drag.
        if (sliderFill) sliderFill.style.width = `${(depth * 100).toFixed(1)}%`;
        if (sliderVal) sliderVal.textContent = `${pct}%`;
        if (slider) slider.setAttribute('aria-valuenow', String(pct));
      };
      svg.addEventListener('contextmenu', (ev) => {
        ev.preventDefault();
        repaint(1);
        emit(1);
      });
      svg.addEventListener('pointerdown', (ev) => {
        if (ev.button !== 0) return;
        ev.preventDefault();
        svg.setPointerCapture(ev.pointerId);
        const startY = ev.clientY;
        const startDepth = l.envs[i].depth;
        const h = svg.getBoundingClientRect().height || 30;
        const move = (e2) => {
          const depth = Math.max(0, Math.min(1, startDepth + (startY - e2.clientY) / h));
          repaint(depth);
          emit(depth);
        };
        svg.addEventListener('pointermove', move);
        svg.addEventListener('pointerup', () => svg.removeEventListener('pointermove', move), { once: true });
      });
    });
    env.querySelector('.addenv').addEventListener('click', () =>
      host.action('setEnvelope', {
        lane: li,
        index: l.envs.length,
        envelope: { target: 'Velocity', period: [1, 4, 7, 16][l.envs.length % 4], depth: 0.3, on: true },
      }));
    // Explicit, discoverable period control: horizontal drag picks an integer
    // bar count (1..PERIOD_MAX), rescales the curve horizontally, and emits the
    // same setEnvelope bridge action carrying envelope.period (MEM040 — no new action).
    env.querySelectorAll('[data-envperiod]').forEach((track) => {
      const i = +track.dataset.envperiod;
      const fill = track.querySelector('i');
      const valSpan = env.querySelector(`[data-envperiodval="${i}"]`);
      const meta = env.querySelector(`[data-envmeta="${i}"]`);
      const svg = env.querySelector(`[data-envdepth="${i}"]`);
      const path = svg ? svg.querySelector('path') : null;
      // Rect is passed in (captured once at pointerdown) rather than re-measured
      // per move: emitting setEnvelope re-renders and detaches this track, after
      // which getBoundingClientRect() reads 0 and the norm collapses to 1.
      const calc = (e, r) => Math.max(0, Math.min(1, (e.clientX - r.left) / r.width));
      const apply = (norm) => {
        const period = normToPeriod(norm); // clamped >= 1, never <= 0
        fill.style.width = `${(periodToNorm(period) * 100).toFixed(1)}%`;
        if (valSpan) valSpan.textContent = `${period} bars`;
        track.setAttribute('aria-valuenow', String(period));
        if (meta) meta.textContent = `${period} bars · sine`;
        if (path) path.setAttribute('d', envPath(l.envs[i].depth, period));
        const e = Object.assign({}, l.envs[i], { period });
        l.envs[i] = e;
        host.action('setEnvelope', { lane: li, index: i, envelope: e });
      };
      // Window-level pointermove/up (matching the macro/expression sliders) so a
      // drag keeps tracking even when the pointer leaves the 14px-tall track. The
      // track geometry is fixed for the gesture, so the pointerdown rect stays
      // valid even after the setEnvelope re-render detaches the original element.
      track.addEventListener('pointerdown', (e) => {
        e.preventDefault();
        const r = track.getBoundingClientRect();
        apply(calc(e, r));
        const mv = (ev) => apply(calc(ev, r));
        const up = () => window.removeEventListener('pointermove', mv);
        window.addEventListener('pointermove', mv);
        window.addEventListener('pointerup', up, { once: true });
      });
    });
    // Explicit, discoverable depth slider mirrors the curve-drag depth edit
    // through the same setEnvelope bridge, giving an accessible labeled control.
    env.querySelectorAll('[data-envdepthslider]').forEach((track) => {
      const i = +track.dataset.envdepthslider;
      const fill = track.querySelector('i');
      const valSpan = env.querySelector(`[data-envdepthsliderval="${i}"]`);
      const svg = env.querySelector(`[data-envdepth="${i}"]`);
      const path = svg ? svg.querySelector('path') : null;
      const curveVal = env.querySelector(`[data-envdepthval="${i}"]`);
      // Rect is passed in (captured once at pointerdown); see the period slider
      // above — the setEnvelope re-render detaches the track mid-drag.
      const calc = (e, r) => Math.max(0, Math.min(1, (e.clientX - r.left) / r.width));
      const apply = (depth) => {
        const pct = Math.round(depth * 100);
        fill.style.width = `${(depth * 100).toFixed(1)}%`;
        if (valSpan) valSpan.textContent = `${pct}%`;
        track.setAttribute('aria-valuenow', String(pct));
        // Keep the curve preview + its depth readout in sync with the slider.
        if (path) path.setAttribute('d', envPath(depth, l.envs[i].period));
        if (curveVal) curveVal.textContent = `${pct}%`;
        if (svg) svg.setAttribute('aria-valuenow', String(pct));
        const e = Object.assign({}, l.envs[i], { depth });
        l.envs[i] = e;
        host.action('setEnvelope', { lane: li, index: i, envelope: e });
      };
      // Window-level pointermove/up (matching the macro/expression sliders) so a
      // drag keeps tracking even when the pointer leaves the 14px-tall track. The
      // track geometry is fixed for the gesture, so the pointerdown rect stays
      // valid even after the setEnvelope re-render detaches the original element.
      track.addEventListener('pointerdown', (e) => {
        e.preventDefault();
        const r = track.getBoundingClientRect();
        apply(calc(e, r));
        const mv = (ev) => apply(calc(ev, r));
        const up = () => window.removeEventListener('pointermove', mv);
        window.addEventListener('pointermove', mv);
        window.addEventListener('pointerup', up, { once: true });
      });
    });

    /* EXPRESSION */
    const expr = s.querySelector('[data-pane="expr"]');
    const PARAMS = [
      { field: 'velocity', label: 'Velocity', norm: l.vel / 127, fmt: (v) => String(Math.round(v * 127)) },
      { field: 'probability', label: 'Probability', norm: l.prob, fmt: (v) => Math.round(v * 100) + '%' },
      { field: 'ghostFloor', label: 'Ghost', norm: l.ghost / 127, fmt: (v) => String(Math.round(v * 127)) },
      { field: 'spread', label: 'Spread', norm: l.spread, fmt: (v) => Math.round(v * 100) + '%' },
      { field: 'swing', label: 'Swing', norm: l.swing, fmt: (v) => Math.round(v * 100) + '%' },
      { field: 'humanize', label: 'Humanize', norm: l.humanize / 50, fmt: (v) => Math.round(v * 50) + 'ms' },
      { field: 'duration', label: 'Duration', norm: l.duration / 4, fmt: (v) => (v * 4).toFixed(1) },
      { field: 'note', label: 'Note', norm: l.note / 127, fmt: (v) => 'N' + Math.round(v * 127) },
      { field: 'channel', label: 'Channel', norm: (l.ch + 1) / 16, fmt: (v) => { const c = Math.round(v * 16) - 1; return c < 0 ? 'Auto' : 'CH ' + c; } },
    ];
    expr.innerHTML = PARAMS.map((p) =>
      `<div class="param-slider"><label>${p.label}</label>` +
      `<div class="slider-track" data-field="${p.field}"><i style="width:${(p.norm * 100).toFixed(1)}%"></i></div>` +
      `<span class="v">${p.fmt(p.norm)}</span></div>`
    ).join('');
    expr.querySelectorAll('.slider-track').forEach((track) => {
      const field = track.dataset.field;
      const p = PARAMS.find((x) => x.field === field);
      const paramId = `lane.${li}.${field}`;
      const fill = track.querySelector('i');
      const vSpan = track.nextElementSibling;
      const calc = (e) => {
        const r = track.getBoundingClientRect();
        return Math.max(0, Math.min(1, (e.clientX - r.left) / r.width));
      };
      const paint = (v) => { fill.style.width = `${(v * 100).toFixed(1)}%`; vSpan.textContent = p.fmt(v); };
      track.addEventListener('pointerdown', (e) => {
        e.preventDefault();
        let lastV = calc(e); paint(lastV);
        host.edit(paramId, lastV, 'begin');
        host.edit(paramId, lastV, 'perform');
        const mv = (ev) => { lastV = calc(ev); paint(lastV); host.edit(paramId, lastV, 'perform'); };
        const up = () => {
          window.removeEventListener('pointermove', mv);
          host.edit(paramId, lastV, 'end');
        };
        window.addEventListener('pointermove', mv);
        window.addEventListener('pointerup', up, { once: true });
      });
    });

    /* ADVANCED */
    const adv = s.querySelector('[data-pane="adv"]');
    const PHRASE = [
      { field: 'phraseLength', label: 'Length', norm: l.phraseLength / 64,
        fmt: (v) => { const b = Math.round(v * 64); return b === 0 ? 'Off' : b + ' bars'; } },
      { field: 'phraseGap', label: 'Gap', norm: l.phraseGap / 64,
        fmt: (v) => { const b = Math.round(v * 64); return b === 0 ? 'Off' : b + ' bars'; } },
      { field: 'phraseOffset', label: 'Offset', norm: l.phraseOffset / 64,
        fmt: (v) => Math.round(v * 64) + ' bars' },
    ];
    // Gap/Offset are inert while Length is Off (the lane never rests), so
    // disable them to match native phrase_edit_view. Derived purely from
    // l.phraseLength — the fmt reads Off when Math.round(l.phraseLength)===0.
    const phraseOff = Math.round(l.phraseLength) === 0;
    PHRASE[1].disabled = phraseOff; // Gap
    PHRASE[2].disabled = phraseOff; // Offset
    const MUTATION = [
      { field: 'mutationRate', label: 'Mutation', norm: l.mutationRate,
        fmt: (v) => Math.round(v * 100) + '%' },
      { field: 'driftRate', label: 'Drift', norm: (l.driftRate + 4) / 8,
        fmt: (v) => { const d = v * 8 - 4; return (d > 0 ? '+' : '') + d.toFixed(1); } },
    ];
    const MORE = [
      { field: 'emphasisProb', label: 'Emphasis', norm: l.emphasisProb,
        fmt: (v) => Math.round(v * 100) + '%' },
      { field: 'timingOffset', label: 'T. Offset', norm: (l.timingOffset + 20) / 40,
        fmt: (v) => { const ms = v * 40 - 20; return (ms > 0 ? '+' : '') + Math.round(ms) + 'ms'; } },
    ];
    const FILL = [
      { field: 'fillEveryN', label: 'Every', norm: (l.fillEveryN || 0) / 64,
        fmt: (v) => { const n = Math.round(v * 64); return n === 0 ? 'Off' : n + ' bars'; } },
    ];
    const ALL_ADV = [...PHRASE, ...MUTATION, ...MORE, ...FILL];
    const SUBS = [1, 2, 4, 8, 16];
    const sliderHtml = (arr) => arr.map((p) =>
      `<div class="param-slider${p.disabled ? ' phrase-disabled' : ''}"><label>${p.label}</label>` +
      `<div class="slider-track" data-field="${p.field}"><i style="width:${(p.norm * 100).toFixed(1)}%"></i></div>` +
      `<span class="v">${p.fmt(p.norm)}</span></div>`
    ).join('');

    adv.innerHTML =
      `<div class="section-label">Phrase</div>` + sliderHtml(PHRASE) +
      `<div class="hint">Lane plays for Length bars, rests for Gap. Offset shifts the start.</div>` +
      `<div class="section-label">Mutation</div>` + sliderHtml(MUTATION) +
      `<div class="hint">Mutation randomly flips steps; Drift shifts pattern phase over time.</div>` +
      `<div class="section-label">More</div>` +
      `<div class="prow"><label>Subdivision</label><div class="chip-row">${SUBS.map((sv) =>
        `<button class="chip${l.subdivision === sv ? ' on' : ''}" data-sub="${sv}">${sv}</button>`).join('')}</div></div>` +
      sliderHtml(MORE) +
      `<div class="prow"><label>Kotekan</label><div class="chip-row">` +
        `<button class="chip${l.kotekanSource === -1 ? ' on' : ''}" data-kot="-1">None</button>` +
        S.lanes.map((sl, si) => si !== li
          ? `<button class="chip${l.kotekanSource === si ? ' on' : ''}" data-kot="${si}">${sl.name}</button>`
          : '').join('') +
      `</div></div>` +
      `<div class="section-label">Fill</div>` + sliderHtml(FILL) +
      `<div class="prow"><label>Manual</label><div class="chip-row">` +
        `<button class="chip" data-manualfill>Fill Now</button></div></div>` +
      `<div class="hint">Fill plays an off-pattern burst every N bars. Fill Now triggers one fill pass immediately.</div>` +
      `<div class="section-label">Seed</div>` +
      `<div class="prow"><label>Lock</label><div class="chip-row">` +
        `<button class="chip${l.seedLocked ? ' on' : ''}" data-seedlock aria-pressed="${l.seedLocked ? 'true' : 'false'}">${l.seedLocked ? 'Locked' : 'Lock Seed'}</button></div></div>` +
      `<div class="hint">Locking pins this lane's pattern to the current seed, so rolling the global seed leaves it unchanged.</div>`;

    adv.querySelector('[data-manualfill]').addEventListener('click', () =>
      host.action('manualFill', {}));

    // Per-lane seed lock: a boolean edit that flips the lane's current lock
    // state. The native processor captures the current global seed into the
    // lane's laneSeed on the false->true edge (M034 S03 T01), so a subsequent
    // global reroll leaves this lane byte-identical.
    adv.querySelector('[data-seedlock]').addEventListener('click', () => {
      const next = l.seedLocked ? 0 : 1;
      host.edit(`lane.${li}.seedLock`, next, 'begin');
      host.edit(`lane.${li}.seedLock`, next, 'perform');
      host.edit(`lane.${li}.seedLock`, next, 'end');
    });

    adv.querySelectorAll('.slider-track').forEach((track) => {
      const field = track.dataset.field;
      const p = ALL_ADV.find((x) => x.field === field);
      const paramId = `lane.${li}.${field}`;
      const fill = track.querySelector('i');
      const vSpan = track.nextElementSibling;
      const calc = (e) => {
        const r = track.getBoundingClientRect();
        return Math.max(0, Math.min(1, (e.clientX - r.left) / r.width));
      };
      const paint = (v) => { fill.style.width = `${(v * 100).toFixed(1)}%`; vSpan.textContent = p.fmt(v); };
      track.addEventListener('pointerdown', (e) => {
        // Gap/Offset are disabled while phrase Length is Off — ignore the drag.
        if (track.closest('.param-slider').classList.contains('phrase-disabled')) return;
        e.preventDefault();
        let lastV = calc(e); paint(lastV);
        host.edit(paramId, lastV, 'begin');
        host.edit(paramId, lastV, 'perform');
        const mv = (ev) => { lastV = calc(ev); paint(lastV); host.edit(paramId, lastV, 'perform'); };
        const up = () => {
          window.removeEventListener('pointermove', mv);
          host.edit(paramId, lastV, 'end');
        };
        window.addEventListener('pointermove', mv);
        window.addEventListener('pointerup', up, { once: true });
      });
    });
    adv.querySelectorAll('[data-sub]').forEach((b) =>
      b.addEventListener('click', () => {
        const norm = SUBS.indexOf(parseInt(b.dataset.sub)) / 4;
        host.edit(`lane.${li}.subdivision`, norm, 'begin');
        host.edit(`lane.${li}.subdivision`, norm, 'perform');
        host.edit(`lane.${li}.subdivision`, norm, 'end');
      }));
    adv.querySelectorAll('[data-kot]').forEach((b) =>
      b.addEventListener('click', () => {
        const kv = parseInt(b.dataset.kot);
        host.edit(`lane.${li}.kotekanSource`, (kv + 1) / 8, 'begin');
        host.edit(`lane.${li}.kotekanSource`, (kv + 1) / 8, 'perform');
        host.edit(`lane.${li}.kotekanSource`, (kv + 1) / 8, 'end');
      }));
  }

  /* ================= state + frame wiring ================= */
  function renderChrome() {
    document.getElementById('presetName').textContent = S.preset;
    document.getElementById('seedVal').textContent = S.seed;
    document.getElementById('tempoVal').textContent = S.tempo.toFixed(1);
    document.getElementById('scA').classList.toggle('on', S.scene === 'A');
    document.getElementById('scB').classList.toggle('on', S.scene === 'B');
    document.getElementById('scM').classList.toggle('on', S.scene === 'Morph');
    // In Morph the output is a blend of A and B, so applying a preset is
    // meaningless. Hide the whole preset group (label + dropdown + seed) rather
    // than just disabling it: that frees the horizontal space the morph slider
    // needs, so the seed no longer overlaps the Cloth chip in the nowrap chrome.
    const morphing = S.scene === 'Morph';
    presetTrigger.classList.toggle('preset-hidden', morphing);
    presetBtn.setAttribute('aria-disabled', morphing ? 'true' : 'false');
    if (morphing) closePresetMenu();
    morphSlider.style.display = S.scene === 'Morph' ? 'flex' : 'none';
    morphFill.style.width = `${((S.morph || 0) * 100).toFixed(1)}%`;
    const chain = S.chain || { enabled: false };
    chainBtn.classList.toggle('on', chain.enabled);
    if (chainPopover) buildChainPopover();
    if (!embedded) {
      const ready = !!S.samplesReady;
      playBtn.classList.toggle('loading', !ready);
      playBtn.style.opacity = ready ? '' : '0.35';
      playBtn.style.cursor = ready ? '' : 'progress';
      playBtn.setAttribute('aria-busy', ready ? 'false' : 'true');
      playBtn.title = ready ? '' : 'Loading samples…';
    }
  }
  function refreshAll() {
    renderChrome();
    buildTags();
    buildDesk();
    if (expanded >= 0) expandStrip(Math.min(expanded, S.lanes.length - 1));
    drawLoom(lastFrame.t8);
    if (noteMapModal) {
      closeNoteMapModal();
      buildNoteMapModal();
    }
  }
  function boot(state) {
    S = state;
    _prevStateStr = JSON.stringify(state);
    recordSeed(state.seed);
    refreshAll();
    sizeLoom();
  }

  let _prevStateStr = null;
  host.onState((state) => {
    const first = !S;
    S = state;
    const sig = JSON.stringify(state);
    if (!first && sig === _prevStateStr) return;
    _prevStateStr = sig;
    if (first) { boot(state); return; }
    recordSeed(state.seed);
    refreshAll();
  });
  if (host.getState()) boot(host.getState());

  /* M045 S01 T03: desk emission overlay. Reads the ordered per-lane emission
     ring exposed by the host and paints em-add / em-drop / em-ghost classes on
     the ladder step buttons. Latest emission per step wins — when the playhead
     next crosses that step and fires a base hit, the mark clears. On Stop the
     host resets its rings, this loop paints all-clear on the next frame, and
     the ladder reverts to base pattern rendering.
     Empty return from getLaneEmissions() (mock host, non-WASM surfaces, or
     stopped playback) hits the fast-path and clears any stale overlay. */
  const OVERLAY_CLASSES = ['em-add', 'em-drop', 'em-ghost'];
  function updateEmissionOverlay(li) {
    if (!host.getLaneEmissions) return;
    const lad = ladders[li];
    if (!lad) return;
    const emissions = host.getLaneEmissions(li) || [];
    const buttons = lad.children;
    if (!buttons.length) return;
    let laneCache = overlayLastKind[li];
    if (!laneCache || laneCache.length !== buttons.length) {
      laneCache = new Array(buttons.length).fill(null);
      overlayLastKind[li] = laneCache;
    }
    // Newest emission per step wins — walk oldest → newest so later entries
    // overwrite earlier ones. Empty entries leave step at null (clears
    // whatever overlay was there).
    const stepKind = new Array(buttons.length).fill(null);
    for (let i = 0; i < emissions.length; i++) {
      const e = emissions[i];
      if (e && e.step >= 0 && e.step < buttons.length) stepKind[e.step] = e.kind;
    }
    for (let i = 0; i < buttons.length; i++) {
      const kind = stepKind[i];
      if (kind === laneCache[i]) continue;
      laneCache[i] = kind;
      const btn = buttons[i];
      const cls = btn.classList;
      if (kind === 'add') { cls.add('em-add'); cls.remove('em-drop', 'em-ghost'); }
      else if (kind === 'drop') { cls.add('em-drop'); cls.remove('em-add', 'em-ghost'); }
      else if (kind === 'ghost') { cls.add('em-ghost'); cls.remove('em-add', 'em-drop'); }
      else cls.remove(...OVERLAY_CLASSES);
    }
  }

  /* Desk "played" timeline (expanded strip only). A second vertical column
     beside the ladder that shows WHAT IS ACTUALLY PLAYED, as a rolling window
     scrolling with the transport. The ladder shows pattern intent (fixed grid
     rows); this column reads the engine emission stream (host.getLaneEmissions →
     {ppq, shiftedPpq, step, kind}) and places each hit by how recently it fired
     relative to the playhead, so it is genuinely time-positioned, NOT quantized
     to the grid: a swung / syncopated / humanized / offset hit enters the window
     at its real shifted onset, off its grid row — tying what the user sees to
     what they hear.

     Rolling window (the fix for "dots accumulate and fill the column"): each dot
     is positioned by its AGE in eighth-ticks behind the current playhead,
     ageT8 = frame.t8 − onsetT8 (onsetT8 = onset ppq × 2, since t8 = 2·ppq). The
     freshest hit (age 0) sits at the bottom by the playhead; as the transport
     advances the dot scrolls UP and, once its age exceeds the window, it
     drops off entirely — so the column shows only the last window of hits and
     self-clears instead of filling up. Orientation matches the ladder
     (column-reverse), so both vertical timelines move the same direction and
     read as complementary: bottom = (age/window)·100%, freshest at bottom.

     Kind coloring mirrors the ladder overlay vocabulary: base = lane hue,
     ghost = dimmed, add = accent (off-grid mutation/macro add), drop = hollow.
     Drops never schedule a note, so they use the grid ppq (no shifted onset);
     fired hits use the shifted onset (fall back to grid ppq for legacy 4-field
     builds without shiftedPpq).

     Empty stream (mock default, DAW, or stopped playback) clears the column and
     draws nothing — never claims an onset it can't observe, mirroring the
     overlay's empty-return degradation contract. */
  // Per-kind base opacity (matches the old ladder-overlay reading: ghost + drop
  // sit dimmer than a base/add hit). Multiplied by the age fade in updatePlayed.
  const PLAYED_KIND_OPACITY = { base: 1, add: 1, ghost: 0.42, drop: 0.55 };
  function updatePlayed(li, t8) {
    const col = playeds[li];
    if (!col) return;
    // Only meaningful in the expanded strip; collapsed strips keep it hidden via
    // CSS, so skip the work entirely to avoid per-frame churn on every lane.
    if (expanded !== li) { if (col.childElementCount) col.replaceChildren(); return; }
    const l = S.lanes[li];
    if (!host.getLaneEmissions || !l || !l.active) { if (col.childElementCount) col.replaceChildren(); return; }
    const emissions = host.getLaneEmissions(li) || [];
    if (!emissions.length) { if (col.childElementCount) col.replaceChildren(); return; }
    // Rolling window = one cycle of the lane in eighth-ticks (cyc8), so exactly
    // one lap of the pattern is visible at a time. cyc8 is already in eighths
    // (t8 units), matching frame.t8. Guard against a degenerate zero-length lane.
    const win = cyc8(l);
    if (!(win > 0)) { if (col.childElementCount) col.replaceChildren(); return; }
    const now = typeof t8 === 'number' ? t8 : 0;
    // Newest emission per (step,kind) wins so a re-fired step replaces its prior
    // mark rather than stacking dots as the ring wraps. Keyed by step+kind but
    // positioned by absolute age, so the SAME step re-fired a cycle later moves.
    const byKey = new Map();
    for (let i = 0; i < emissions.length; i++) {
      const e = emissions[i];
      if (!e) continue;
      const isDrop = e.kind === 'drop';
      const ppq = isDrop ? e.ppq : (typeof e.shiftedPpq === 'number' ? e.shiftedPpq : e.ppq);
      // onset in eighth-ticks; age behind the playhead. Fresh hits (age→0) at the
      // bottom, older ones scroll up; past the window they drop off (self-clear).
      const ageT8 = now - ppq * 2;
      if (!(ageT8 >= 0 && ageT8 <= win)) continue;
      byKey.set(e.step + ':' + e.kind, { age: ageT8, kind: e.kind });
    }
    const frag = document.createDocumentFragment();
    byKey.forEach((d) => {
      const dot = document.createElement('i');
      dot.className = 'pdot p-' + d.kind;
      // Freshest (age 0) at bottom (0%), oldest (age=win) at top (100%) — dots
      // scroll UPWARD, matching the column-reverse ladder beside it so both
      // vertical timelines read as complementary (moving the same direction).
      const frac = d.age / win;
      dot.style.bottom = (frac * 100).toFixed(2) + '%';
      // Opacity = age fade × the kind's base opacity (ghost/drop read dimmer than
      // base/add even when fresh). Inline so CSS never fights the fade.
      const fade = Math.max(0.15, 1 - d.age / win);
      dot.style.opacity = (fade * (PLAYED_KIND_OPACITY[d.kind] || 1)).toFixed(2);
      frag.appendChild(dot);
    });
    col.replaceChildren(frag);
  }

  /* ================= first-run MIDI-routing diagnostic banner =================
     Poly is a MIDI-only instrument: it makes no sound until its output is routed
     to a drum instrument in the DAW, and the plugin has no audio-feedback path
     to prove that routing exists. The only viable fix for the "plugin makes no
     sound; is it broken?" support case is in-plugin guidance shown exactly in
     the failure window — transport playing while at least one active lane is
     emitting hits, yet no audio is guaranteed. The banner fires once after a
     wall-clock dwell (Date.now(), not a frame count, so it is robust to frame
     rate), is dismissible, and persists the dismissal to localStorage so it
     never re-nags. It NEVER appears in non-embedded (mock/standalone) mode —
     there the browser Web Audio host makes real sound, so the diagnostic is
     meaningless. Copy mirrors the routing wording in
     site/src/content/docs/guide-using-poly.mdx and docs/cubase-workflow.md. */
  const ROUTING_BANNER_KEY = 'poly.routingBannerDismissed';
  // Wall-clock dwell before the banner appears. Overridable via
  // window.__POLY_ROUTING_BANNER_MS so Playwright can shrink the production
  // window (a few seconds) to a few frames without a slow real-time wait.
  const ROUTING_BANNER_MS = (typeof window.__POLY_ROUTING_BANNER_MS === 'number')
    ? window.__POLY_ROUTING_BANNER_MS : 4000;
  let routingEmitSince = 0;      // Date.now() when play+emit began; 0 = not counting
  let routingBanner = null;      // banner DOM node once shown
  let routingBannerDone = false; // shown or dismissed this session — never re-trigger

  function routingDismissed() {
    // localStorage can throw (private mode / disabled storage) — treat any
    // failure as "not dismissed" so the diagnostic still helps the user.
    try { return localStorage.getItem(ROUTING_BANNER_KEY) === '1'; } catch (_) { return false; }
  }
  // A lane "emits hits" when it is active and its pattern actually produces
  // onsets — mirrors the VU `active` gate (cells lanes always emit; euclid /
  // timeline lanes emit only with >=1 hit). A muted or empty lane makes no MIDI.
  function laneEmitsHits(l) {
    if (!l || !l.active) return false;
    if (l.cells) return true;
    if (l.timeline) return Array.isArray(l.fixed) && l.fixed.some(Boolean);
    return Array.isArray(l.pattern) && l.pattern.some(Boolean);
  }
  function showRoutingBanner() {
    if (routingBanner) return;
    const b = document.createElement('div');
    b.id = 'routingBanner';
    b.className = 'routing-banner';
    b.setAttribute('role', 'status');
    b.setAttribute('aria-live', 'polite');
    b.innerHTML =
      `<div class="routing-banner-body">` +
      `<strong>No sound? Route Poly's MIDI to a drum instrument.</strong>` +
      `<span>Poly generates MIDI only — it produces no audio on its own. ` +
      `Route its output to a drum sound source (Groove Agent, Battery, Kontakt, ` +
      `Drum Rack, or any GM-compatible drum plugin) to hear the groove. ` +
      `See the Poly guide for your DAW's MIDI routing steps.</span>` +
      `</div>` +
      `<button type="button" class="routing-banner-dismiss" ` +
      `aria-label="Dismiss routing help and don't show again">Don't show again</button>`;
    document.getElementById('win').appendChild(b);
    routingBanner = b;
    b.querySelector('.routing-banner-dismiss').addEventListener('click', dismissRoutingBanner);
  }
  function dismissRoutingBanner() {
    routingBannerDone = true;
    try { localStorage.setItem(ROUTING_BANNER_KEY, '1'); } catch (_) { /* storage disabled — best effort */ }
    if (routingBanner) { routingBanner.remove(); routingBanner = null; }
  }
  function updateRoutingBanner(frame) {
    if (!embedded || routingBannerDone) return;
    if (routingDismissed()) { routingBannerDone = true; return; }
    const emitting = !!(frame && frame.playing) && !!(S && S.lanes && S.lanes.some(laneEmitsHits));
    if (!emitting) { routingEmitSince = 0; return; }
    const now = Date.now();
    if (routingEmitSince === 0) { routingEmitSince = now; return; }
    if (now - routingEmitSince >= ROUTING_BANNER_MS) {
      routingBannerDone = true; // latch before showing so it fires exactly once
      showRoutingBanner();
    }
  }

  host.onFrame((frame) => {
    if (!S) return;
    if (REDUCED) frame = Object.assign({}, frame, { t8: Math.floor(frame.t8) });
    lastFrame = frame;
    updateRoutingBanner(frame);
    updateCaptureChips();
    document.getElementById('picon').setAttribute('d',
      frame.playing ? 'M2.5 2 H5.5 V12 H2.5 Z M8.5 2 H11.5 V12 H8.5 Z' : 'M3 2 L12 7 L3 12 Z');
    document.querySelector('#conv b').textContent = Math.ceil(frame.convLeft / 12);
    // M051 S02: reflect host time signature next to BPM. tsNum/tsDen absent
    // on legacy hosts or older plugin builds — fall back to 4/4 silently.
    if (typeof frame.tsNum === 'number' && typeof frame.tsDen === 'number') {
      const el = document.getElementById('timeSigVal');
      if (el) el.textContent = `${frame.tsNum}/${frame.tsDen}`;
    }
    if (mode === 'desk') {
      S.lanes.forEach((l, li) => {
        const fl = frame.lanes[li];
        if (!fl || !hands[li]) return;
        // Freeze needle + step highlight when the lane is muted so disabled
        // reads as no motion, not just dimmer motion.
        const laneOn = !!l.active;
        hands[li].setAttribute('transform', `rotate(${laneOn ? fl.ph * 360 : 0} 32 32)`);
        // M053 S12 T03: rotate the ring <g> per frame by the engine's drift offset
        // so the circular display tracks the audible drifted cycle step
        // (engine.cpp computeDriftedCycleStep, mirrored by PolyGrooveMath.driftOffset
        // and carried on fl.driftOffset by both host pumps). Freeze at 0 when muted so
        // a disabled lane reads as no motion. drawRing() only rewrites innerHTML, so
        // this transform survives ring rebuilds.
        //
        // D008: the ring is geometrically INVARIANT under subdivision (stepLen). Dot
        // angles are fractions of the cycle (i/steps or onset/cyc8), which a stepLen
        // change does not alter, so subdivision never rotates or reshapes the ring —
        // only drift does. Subdivision motion is shown on the convergence timeline /
        // ladder instead. This divergence is intentional, not a missing cue.
        const ringSteps = l.cells ? l.cells.length : l.steps;
        const ringDrift = laneOn && ringSteps ? ((fl.driftOffset || 0) / ringSteps) * 360 : 0;
        rings[li].setAttribute('transform', `rotate(${ringDrift} 32 32)`);
        ladders[li].querySelectorAll('button').forEach((b, i) =>
          b.classList.toggle('now', laneOn && frame.playing && i === fl.step));
        updateEmissionOverlay(li);
        updatePlayed(li, frame.t8);
        const active = laneOn && frame.playing && (l.cells ? true : !!l.pattern[fl.step]) && frame.t8 % 1 < 0.5;
        vus[li].style.width = active ? `${(l.vel / 127) * 100}%` : '4%';
        if (expanded === li) {
          strips[li].querySelectorAll('[data-envph]').forEach((ln, i) => {
            const e = l.envs[i];
            if (!e) return;
            // The box is a fixed PERIOD_MAX-bar window (see envPath), so the
            // playhead sweeps it once per PERIOD_MAX bars regardless of the
            // envelope's own period. A longer-period wave then visibly takes
            // more of the sweep to complete one cycle — the intuitive slower/
            // wider reading. (PERIOD_MAX=16, mirrored from the curve builder.)
            const x = (((frame.t8 / 12) / 16) % 1) * 74;
            ln.setAttribute('x1', x.toFixed(1));
            ln.setAttribute('x2', x.toFixed(1));
          });
        }
      });
      const cm = document.querySelector('#cmeter i');
      if (cm) cm.style.height = `${(1 - frame.convLeft / CONV) * 100}%`;
    } else {
      drawLoom(frame.t8);
    }
  });
})();
