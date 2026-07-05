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

  let S = null; // latest state snapshot
  let lastFrame = { t8: 0, playing: false, convLeft: CONV, lanes: [] };
  let mode = 'desk';
  let expanded = -1;
  let strips = [], rings = [], hands = [], ladders = [], vus = [];
  const tabState = {};

  /* ================= chrome ================= */
  const embedded = !!window.__POLY_EMBEDDED__;
  document.getElementById('play').addEventListener('click', () => { if (!embedded) host.action('togglePlay', {}); });
  if (embedded) {
    const pb = document.getElementById('play');
    pb.style.opacity = '0.35';
    pb.style.cursor = 'default';
    pb.title = 'Transport controlled by host DAW';
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
      const v = calc(e); paint(v);
      host.edit('scene.morph', v, 'begin');
      host.edit('scene.morph', v, 'perform');
      const mv = (ev) => { const nv = calc(ev); paint(nv); host.edit('scene.morph', nv, 'perform'); };
      const up = (ev) => {
        window.removeEventListener('pointermove', mv);
        const nv = calc(ev); paint(nv); host.edit('scene.morph', nv, 'end');
      };
      window.addEventListener('pointermove', mv);
      window.addEventListener('pointerup', up, { once: true });
    });
  })();

  /* --- chain popover --- */
  const chainBtn = document.getElementById('chainBtn');
  let chainPopover = null;
  function buildChainPopover() {
    if (chainPopover) { chainPopover.remove(); chainPopover = null; }
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
      host.edit('chain.enabled', chain.enabled ? 0 : 1, 'begin');
      host.edit('chain.enabled', chain.enabled ? 0 : 1, 'perform');
      host.edit('chain.enabled', chain.enabled ? 0 : 1, 'end');
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
        document.removeEventListener('click', dismiss);
      }
    };
    setTimeout(() => document.addEventListener('click', dismiss), 0);
  }
  function closeChainPopover() {
    if (chainPopover) { chainPopover.remove(); chainPopover = null; }
  }
  chainBtn.addEventListener('click', (e) => {
    e.stopPropagation();
    if (chainPopover) closeChainPopover();
    else buildChainPopover();
  });

  /* --- export button --- */
  document.getElementById('exportBtn').addEventListener('click', () => {
    host.action('exportRequest', {});
    const btn = document.getElementById('exportBtn');
    btn.classList.add('on');
    setTimeout(() => btn.classList.remove('on'), 600);
  });

  /* --- note map modal --- */
  let noteMapModal = null;
  function buildNoteMapModal() {
    if (noteMapModal) { closeNoteMapModal(); return; }
    const nm = S.noteMap || [];
    const pop = document.createElement('div');
    pop.className = 'notemap-modal open';
    pop.innerHTML = `<div class="notemap-inner">
      <div class="notemap-header"><h4>Note Map</h4><button class="notemap-reset">Reset</button><button class="notemap-close">✕</button></div>
      <div class="notemap-grid">${Array.from({ length: 128 }, (_, i) =>
        `<div class="notemap-row" data-note="${i}">` +
        `<span class="ni">${i}</span>` +
        `<button data-nmdec="${i}">−</button>` +
        `<span class="no">${nm[i] ?? i}</span>` +
        `<button data-nminc="${i}">+</button>` +
        `${nm[i] !== i ? '<span class="nm-mod">✦</span>' : ''}` +
        `</div>`).join('')}
      </div></div>`;
    document.getElementById('win').appendChild(pop);
    noteMapModal = pop;

    pop.querySelector('.notemap-close').addEventListener('click', closeNoteMapModal);
    pop.querySelector('.notemap-reset').addEventListener('click', () => {
      host.action('resetNoteMap', {});
    });
    pop.querySelectorAll('[data-nmdec]').forEach((b) =>
      b.addEventListener('click', () => {
        const ni = parseInt(b.dataset.nmdec);
        const cur = S.noteMap ? S.noteMap[ni] : ni;
        if (cur > 0) host.action('setNoteMap', { note: ni, output: cur - 1 });
      }));
    pop.querySelectorAll('[data-nminc]').forEach((b) =>
      b.addEventListener('click', () => {
        const ni = parseInt(b.dataset.nminc);
        const cur = S.noteMap ? S.noteMap[ni] : ni;
        if (cur < 127) host.action('setNoteMap', { note: ni, output: cur + 1 });
      }));

    const dismiss = (e) => {
      if (noteMapModal && !noteMapModal.querySelector('.notemap-inner').contains(e.target) &&
          e.target !== document.getElementById('noteMapBtn')) {
        closeNoteMapModal();
        document.removeEventListener('click', dismiss);
      }
    };
    setTimeout(() => document.addEventListener('click', dismiss), 0);
  }
  function closeNoteMapModal() {
    if (noteMapModal) { noteMapModal.remove(); noteMapModal = null; }
  }
  document.getElementById('noteMapBtn').addEventListener('click', (e) => {
    e.stopPropagation();
    buildNoteMapModal();
  });

  /* --- preset dropdown --- */
  const presetBtn = document.getElementById('presetName');
  const presetMenu = document.getElementById('presetMenu');
  let presetMenuBuilt = false;

  function buildPresetMenu() {
    if (!S || !S.presets || presetMenuBuilt) return;
    presetMenuBuilt = true;
    presetMenu.innerHTML = '';
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
      opt.innerHTML = `${p.name}<small>${p.description}</small>`;
      presetMenu.appendChild(opt);
    });
    presetMenu.addEventListener('click', (e) => {
      const opt = e.target.closest('[role="option"]');
      if (!opt) return;
      host.action('applyPreset', { index: parseInt(opt.dataset.index, 10) });
      closePresetMenu();
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
    if (m === 'cloth') sizeLoom();
    if (m === 'desk' && focus >= 0) expandStrip(focus);
  }

  /* ================= cloth ================= */
  const loom = document.getElementById('loom');
  const tags = document.getElementById('tags');
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
  function drawLoom(t8) {
    const g = loom.getContext('2d');
    if (!g || !S) return;
    const W = loom.width, H = loom.height, dp = devicePixelRatio;
    g.clearRect(0, 0, W, H);
    const bandH = H / S.lanes.length, colW = W / CONV;
    S.lanes.forEach((l, li) => {
      const y0 = li * bandH;
      g.fillStyle = li % 2 ? '#222E52' : '#26335A';
      g.fillRect(0, y0, W, bandH);
      g.fillStyle = 'rgba(240,234,223,.05)';
      for (let x = 0; x < CONV; x += 2) g.fillRect(x * colW, y0, 1 * dp, bandH);
      for (let e = 0; e < CONV; e++) {
        const hit = laneHitAt(l, e);
        if (!hit) continue;
        const vn = hitVelocity(l, li, e, hit);
        const wUnits = l.cells ? l.cells[hit.step] : l.stepLen;
        const bw = colW * wUnits * 0.86;
        const bh = bandH * Math.min(0.92, 0.3 + vn * 0.52);
        const x = e * colW + colW * wUnits * 0.07;
        const y = y0 + (bandH - bh) / 2;
        g.globalAlpha = hit.step === 0 ? 1 : 0.86;
        g.fillStyle = l.hue;
        g.fillRect(x, y, bw, bh);
        g.globalAlpha = 0.22;
        g.fillStyle = '#0E1526';
        for (let ty = y + 3 * dp; ty < y + bh; ty += 6 * dp) g.fillRect(x, ty, bw, 1.5 * dp);
        g.globalAlpha = 1;
        if (hit.step === 0) {
          g.fillStyle = 'rgba(240,234,223,.85)';
          g.fillRect(x, y0 + bandH * 0.08, 2 * dp, bandH * 0.84);
        }
      }
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

  /* ================= desk ================= */
  const desk = document.getElementById('desk');
  function el(t, at, parent) {
    const e = document.createElementNS(NS, t);
    for (const k in at) e.setAttribute(k, at[k]);
    parent.appendChild(e);
    return e;
  }
  function buildDesk() {
    desk.innerHTML = '';
    strips = []; rings = []; hands = []; ladders = []; vus = [];
    S.lanes.forEach((l, li) => {
      const s = document.createElement('div');
      s.className = 'strip';
      s.dataset.lane = li;
      s.style.setProperty('--c', l.hue);
      s.innerHTML = `
        <div class="head">
          <div class="nm"><b>${l.name}</b>${l.role}</div>
          <button class="mute${l.active ? '' : ' off'}" data-mute aria-label="Mute ${l.name}" title="Mute">●</button>
          <button class="ex" aria-label="Expand ${l.name} strip" title="Expand">⤢</button>
        </div>
        <div class="body2">
          <div class="core">
            <svg class="ring" viewBox="0 0 64 64" aria-hidden="true"></svg>
            <div class="ladder" role="group" aria-label="${l.name} steps"></div>
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
        <div class="stat">CH ${l.ch} · N${l.note}</div>`;
      desk.appendChild(s);
      strips.push(s);
      const svg = s.querySelector('svg.ring');
      el('circle', { cx: 32, cy: 32, r: 24, fill: 'none', stroke: '#2B251E', 'stroke-width': 1 }, svg);
      rings.push(el('g', {}, svg));
      hands.push(el('line', { x1: 32, y1: 32, x2: 32, y2: 9, stroke: '#F0EADF', 'stroke-width': 1.2, opacity: 0.85 }, svg));
      ladders.push(s.querySelector('.ladder'));
      vus.push(s.querySelector('.vu i'));
      s.querySelector('.ex').addEventListener('click', () => expandStrip(expanded === li ? -1 : li));
      s.querySelector('[data-mute]').addEventListener('click', () => {
        const active = S.lanes[li].active;
        host.edit(`lane.${li}.active`, active ? 0 : 1, 'begin');
        host.edit(`lane.${li}.active`, active ? 0 : 1, 'perform');
        host.edit(`lane.${li}.active`, active ? 0 : 1, 'end');
      });
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
        const v = calc(e); paint(v);
        host.edit(paramId, v, 'begin');
        host.edit(paramId, v, 'perform');
        const mv = (ev) => { const nv = calc(ev); paint(nv); host.edit(paramId, nv, 'perform'); };
        const up = (ev) => {
          window.removeEventListener('pointermove', mv);
          const nv = calc(ev); paint(nv); host.edit(paramId, nv, 'end');
        };
        window.addEventListener('pointermove', mv);
        window.addEventListener('pointerup', up, { once: true });
      });
    });
    desk.style.gridTemplateColumns = `repeat(${S.lanes.length}, 1fr) 196px`;
  }
  function expandStrip(li) {
    expanded = li;
    strips.forEach((s, i) => s.classList.toggle('expanded', i === li));
    const n = S.lanes.length;
    const wide = n >= 6 ? '4fr' : '2.9fr';
    const narrow = n >= 6 ? '.38fr' : '.62fr';
    const master = n >= 6 ? '160px' : '176px';
    desk.style.gridTemplateColumns = li < 0
      ? `repeat(${n}, 1fr) 196px`
      : S.lanes.map((_, i) => (i === li ? wide : narrow)).join(' ') + ` ${master}`;
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
        <div class="hint">Fixed pattern — immune to macros. The bell is law.</div>
        <div class="hrow" data-fixed>${l.fixed.map((h, i) => `<button class="${h ? 'hit' : ''}" data-i="${i}" aria-label="pulse ${i + 1}"></button>`).join('')}</div>`;
    } else {
      html += `<div class="prow"><label>Steps</label><span class="stepper"><button data-st="-1">−</button><span class="v">${l.steps} × ${l.stepLen === 2 ? '♩' : '♪'}</span><button data-st="1">+</button></span></div>
        <div class="prow"><label>Hits</label><span class="stepper"><button data-ht="-1">−</button><span class="v">E(${l.hits},${l.steps})</span><button data-ht="1">+</button></span></div>
        <div class="prow"><label>Rotation</label><span class="stepper"><button data-rt="-1">−</button><span class="v">${l.rot}</span><button data-rt="1">+</button></span></div>
        <div class="prow"><label>Additive cells</label><span class="switch"><button ${l.cells ? 'class="on"' : ''} data-cl aria-label="Additive cells"><i></i></button></span></div>`;
      if (l.cells)
        html += `<div class="cells" data-cells>${l.cells.map((c, i) => `<button class="onc" style="flex:${c} 1 0" data-i="${i}">${c}</button>`).join('')}<button data-addcell style="flex:.6 1 0">+</button></div>
          <div class="hint">aksak grouping — click a cell to cycle 2·3·4 · cycle = ${cyc8(l)}♪ (${l.cells.join('+')})</div>`;
    }
    pat.innerHTML = html;
    if (l.timeline) {
      pat.querySelectorAll('[data-fixed] button').forEach((b) =>
        b.addEventListener('click', () => {
          const i = +b.dataset.i;
          host.action('setFixedStep', { lane: li, step: i, on: !l.fixed[i] });
        }));
    } else {
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
        cellsEl.querySelectorAll('button[data-i]').forEach((b) =>
          b.addEventListener('click', () => {
            const i = +b.dataset.i;
            const next = l.cells.slice();
            next[i] = next[i] >= 4 ? 2 : next[i] + 1;
            host.action('setCells', { lane: li, cells: next });
          }));
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
    });

    /* ENVELOPES */
    const curve = (e, id) => {
      let d = 'M0 15';
      for (let x = 0; x <= 74; x += 2)
        d += ` L${x} ${(15 - Math.sin((x / 74) * Math.PI * 2) * 11 * e.depth).toFixed(1)}`;
      return `<svg viewBox="0 0 74 30" aria-hidden="true"><path d="${d}" fill="none" stroke="${l.hue}" stroke-width="1.4" opacity="${e.on ? 0.95 : 0.3}"/><line data-envph="${id}" x1="0" y1="2" x2="0" y2="28" stroke="#F0EADF" stroke-width="1" opacity="${e.on ? 0.7 : 0}"/></svg>`;
    };
    env.innerHTML =
      l.envs.map((e, i) => `
        <div class="envrow">
          <div class="t">${e.target} · <span style="color:var(--dim)">${e.period} bars · sine</span></div>
          ${curve(e, i)}
          <div class="m">depth ${Math.round(e.depth * 100)}% <button data-envon="${i}" class="chip ${e.on ? 'on' : ''}" style="padding:2px 8px">${e.on ? 'ON' : 'OFF'}</button></div>
        </div>`).join('') +
      `<button class="addenv">+ add envelope</button>
       <div class="hint">envelopes superimpose — different periods create multiphase motion</div>`;
    env.querySelectorAll('[data-envon]').forEach((b) =>
      b.addEventListener('click', () => {
        const i = +b.dataset.envon;
        const e = Object.assign({}, l.envs[i], { on: !l.envs[i].on });
        host.action('setEnvelope', { lane: li, index: i, envelope: e });
      }));
    env.querySelector('.addenv').addEventListener('click', () =>
      host.action('setEnvelope', {
        lane: li,
        index: l.envs.length,
        envelope: { target: 'Velocity', period: [1, 4, 7, 16][l.envs.length % 4], depth: 0.3, on: true },
      }));

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
      { field: 'channel', label: 'Channel', norm: (l.ch - 1) / 15, fmt: (v) => 'CH ' + (Math.round(v * 15) + 1) },
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
        const v = calc(e); paint(v);
        host.edit(paramId, v, 'begin');
        host.edit(paramId, v, 'perform');
        const mv = (ev) => { const nv = calc(ev); paint(nv); host.edit(paramId, nv, 'perform'); };
        const up = (ev) => {
          window.removeEventListener('pointermove', mv);
          const nv = calc(ev); paint(nv); host.edit(paramId, nv, 'end');
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
    const ALL_ADV = [...PHRASE, ...MUTATION, ...MORE];
    const SUBS = [1, 2, 4, 8, 16];
    const sliderHtml = (arr) => arr.map((p) =>
      `<div class="param-slider"><label>${p.label}</label>` +
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
      `</div></div>`;

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
        e.preventDefault();
        const v = calc(e); paint(v);
        host.edit(paramId, v, 'begin');
        host.edit(paramId, v, 'perform');
        const mv = (ev) => { const nv = calc(ev); paint(nv); host.edit(paramId, nv, 'perform'); };
        const up = (ev) => {
          window.removeEventListener('pointermove', mv);
          const nv = calc(ev); paint(nv); host.edit(paramId, nv, 'end');
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
    morphSlider.style.display = S.scene === 'Morph' ? 'flex' : 'none';
    morphFill.style.width = `${((S.morph || 0) * 100).toFixed(1)}%`;
    const chain = S.chain || { enabled: false };
    chainBtn.classList.toggle('on', chain.enabled);
    if (chainPopover) buildChainPopover();
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
    if (first) boot(state);
    else refreshAll();
  });
  if (host.getState()) boot(host.getState());

  host.onFrame((frame) => {
    if (!S) return;
    if (REDUCED) frame = Object.assign({}, frame, { t8: Math.floor(frame.t8) });
    lastFrame = frame;
    document.getElementById('picon').setAttribute('d',
      frame.playing ? 'M2.5 2 H5.5 V12 H2.5 Z M8.5 2 H11.5 V12 H8.5 Z' : 'M3 2 L12 7 L3 12 Z');
    document.querySelector('#conv b').textContent = Math.ceil(frame.convLeft / 12);
    if (mode === 'desk') {
      S.lanes.forEach((l, li) => {
        const fl = frame.lanes[li];
        if (!fl || !hands[li]) return;
        hands[li].setAttribute('transform', `rotate(${fl.ph * 360} 32 32)`);
        ladders[li].querySelectorAll('button').forEach((b, i) =>
          b.classList.toggle('now', frame.playing && i === fl.step));
        const active = frame.playing && (l.cells ? true : !!l.pattern[fl.step]) && frame.t8 % 1 < 0.5;
        vus[li].style.width = active ? `${(l.vel / 127) * 100}%` : '4%';
        if (expanded === li) {
          strips[li].querySelectorAll('[data-envph]').forEach((ln, i) => {
            const e = l.envs[i];
            if (!e) return;
            const x = (((frame.t8 / 12) / e.period) % 1) * 74;
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
