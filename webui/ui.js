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
    if (e.key === 'Escape') expandStrip(-1);
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
            </div>
            <div class="pane on" data-pane="pattern"></div>
            <div class="pane" data-pane="timing"></div>
            <div class="pane" data-pane="env"></div>
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
      s.querySelectorAll('.tabs button').forEach((tb) =>
        tb.addEventListener('click', () => {
          s.querySelectorAll('.tabs button').forEach((x) => x.classList.toggle('on', x === tb));
          s.querySelectorAll('.pane').forEach((p) => p.classList.toggle('on', p.dataset.pane === tb.dataset.tab));
        }));
      refreshStrip(li);
    });
    const m = document.createElement('div');
    m.id = 'master';
    const macros = S.macros;
    m.innerHTML = `<h3>Master</h3>
      ${Object.entries({ Complexity: macros.complexity, Density: macros.density, Syncopation: macros.syncopation,
                         Swing: macros.swing, Tension: macros.tension, Humanize: macros.humanize })
        .map(([n, v]) => `<div class="macro"><div class="t"><span>${n}</span><b>${Math.round(v * 100)}</b></div>
          <div class="track"><i style="width:${v * 100}%"></i></div></div>`).join('')}
      <div id="cmeter"><i></i><span>Convergence</span></div>`;
    desk.appendChild(m);
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
  }

  /* ================= state + frame wiring ================= */
  function renderChrome() {
    document.getElementById('presetName').textContent = S.preset;
    document.getElementById('seedVal').textContent = S.seed;
    document.getElementById('tempoVal').textContent = S.tempo.toFixed(1);
    document.getElementById('scA').classList.toggle('on', S.scene !== 'B');
    document.getElementById('scB').classList.toggle('on', S.scene === 'B');
  }
  function refreshAll() {
    renderChrome();
    buildTags();
    buildDesk();
    if (expanded >= 0) expandStrip(Math.min(expanded, S.lanes.length - 1));
    drawLoom(lastFrame.t8);
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
