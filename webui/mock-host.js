'use strict';
/**
 * Mock host: browser-standalone PolyHost implementation.
 * Owns the groove model and a WebAudio preview engine so the UI can be
 * developed, demoed, and CI-tested without the plugin. Provides all 43
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
    { name: 'Sub-Saharan: Agbekor', description: 'Ewe-inspired polymetric ensemble — gankogui bell timeline, kidi, sogo and lead drum' },
    { name: 'Gamelan: Colotomic', description: 'Javanese colotomic nesting — ketuk, kempul, kenong and gong ageng at 4:8:16:32 ratios' },
    { name: 'Polymetric Foundation', description: 'Two-lane polymetric demonstration — 12-step bell against 7-step counter pattern' },
    { name: 'Ewe Polymetric Ensemble', description: 'Gankogui bell timeline with support, responding, and interlocking lead drums' },
    { name: 'Manding Djembe', description: 'Three-lane djembe ensemble — dunun, sangban, and lead sharing an 8-step cycle' },
    { name: 'Cuban Son Montuno', description: 'Clave matrix with cascara, tumbao bass, conga, and shaker layers' },
    { name: 'Afrobeat Lagos', description: 'Extended Afrobeat groove — bell timeline with staggered phrase gating' },
    { name: 'Balinese Kotekan', description: 'Polos and sangsih interlocking with jegogan bass and reyong accents' },
    { name: 'Javanese Colotomic', description: 'Nested colotomic hierarchy — ketuk, kempul, kenong, and gong at 4:8:16:32' },
    { name: 'Tintal Groove', description: 'Hindustani 16-beat cycle with sam, theka, dugun, and tigun layakari layers' },
    { name: 'Rupak Tal', description: '7-beat Hindustani cycle — sam marker, theka, and counter rhythm' },
    { name: 'Rachenitsa 7/8', description: 'Bulgarian rachenitsa — tupan, kaval, and gadulka in seven-beat cycle' },
    { name: 'Kopanitsa 11/8', description: 'Fast Bulgarian kopanitsa — kick, snare, hat, and bell in eleven-beat cycle' },
    { name: 'Reich Phase Process', description: 'Gradual phase-shifting — two identical 12-step patterns drifting apart' },
    { name: 'Riley Layered Entry', description: 'Staggered voice entries with phrase gating — foundation plus four gated layers' },
    { name: 'Nancarrow Tempi', description: 'Independent tempo multipliers — anchor, double-time, half-time, and hemiola voices' },
    { name: 'Minimal Techno', description: 'Four-on-the-floor kick with polymetric 7-step ghost percussion and sparse clap' },
    { name: 'Deep House', description: 'Swung house groove — open hat, shaker, and rim with moderate swing and humanize' },
    { name: 'Samba Batucada', description: 'Full batucada ensemble — surdo, tamborim, agogo, repinique, and caixa' },
    { name: 'Bossa Nova Trio', description: 'Intimate bossa — E(5,16) bass, ride cymbal, and ghost brush' },
    { name: 'Classic Funk', description: 'JB-style pocket — kick pushes late, snare pulls early, dense ghost layer' },
    { name: 'Neo-Soul Pocket', description: 'Loose, warm groove with heavy humanize and polymetric 12-step rim click' },
    { name: 'Jazz Bop Ride', description: 'Bop timekeeping — ride and hi-hat foot with mutating kick and snare comp' },
    { name: 'Elvin Jones Cascade', description: 'Polyrhythmic cascade — 4, 3, 5, and 7-step cycles with swing and mutation' },
    { name: 'Jungle Break', description: 'Syncopated chopped break — dense ghost layer with rolling undertow' },
    { name: 'Liquid Drum and Bass', description: 'Clean two-step kick with steady hat and warm ride cymbal wash' },
    { name: 'Afro-Electronic Fusion', description: 'Cuban clave meets techno kick and gamelan-inspired kotekan shimmer' },
    { name: 'Balkan Funk', description: '7/8 aksak with funk ghost notes and micro-timing on the hi-hat' },
    { name: 'Compositional Arc', description: 'Six-lane layered build — three continuous lanes plus three gated ornamental voices' },
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
    ['Bell', 'Kidi', 'Sogo', 'Lead'],  // 14: Agbekor
    ['Ketuk', 'Kempul', 'Kenong', 'Gong'],  // 15: Gamelan Colotomic
    ['Bell', 'Counter'],  // 16: Polymetric Foundation
    ['Bell', 'Support', 'Responding', 'Lead'],  // 17: Ewe Ensemble
    ['Dunun', 'Sangban', 'Djembe'],  // 18: Manding
    ['Clave', 'Cascara', 'Tumbao', 'Conga', 'Shaker'],  // 19: Cuban Son
    ['Bell', 'Hi-Hat', 'Kick', 'Snare', 'Shaker', 'Conga'],  // 20: Afrobeat Lagos
    ['Polos', 'Sangsih', 'Jegogan', 'Reyong'],  // 21: Balinese Kotekan
    ['Ketuk', 'Kempul', 'Kenong', 'Gong'],  // 22: Javanese Colotomic
    ['Sam', 'Theka', 'Dugun', 'Tigun'],  // 23: Tintal
    ['Sam', 'Theka', 'Counter'],  // 24: Rupak Tal
    ['Tupan Bass', 'Tupan Rim', 'Kaval', 'Gadulka'],  // 25: Rachenitsa
    ['Kick', 'Snare', 'Hi-Hat', 'Bell'],  // 26: Kopanitsa
    ['Fixed', 'Drifting', 'Anchor'],  // 27: Reich Process
    ['Foundation', 'Voice A', 'Voice B', 'Voice C', 'Voice D'],  // 28: Riley
    ['Anchor', 'Double', 'Half', 'Hemiola'],  // 29: Nancarrow
    ['Kick', 'Hi-Hat', 'Ghost', 'Clap'],  // 30: Minimal Techno
    ['Kick', 'Open Hat', 'Shaker', 'Rim'],  // 31: Deep House
    ['Surdo', 'Tamborim', 'Agogo', 'Repinique', 'Caixa'],  // 32: Batucada
    ['Bass', 'Ride', 'Brush'],  // 33: Bossa Trio
    ['Kick', 'Snare Accent', 'Snare Ghost', 'Hi-Hat'],  // 34: Classic Funk
    ['Kick', 'Snare', 'Hi-Hat', 'Rim Click'],  // 35: Neo-Soul
    ['Ride', 'HH Foot', 'Kick', 'Snare Comp'],  // 36: Jazz Bop
    ['Ride', 'Snare', 'Tom', 'Bass Drum'],  // 37: Elvin
    ['Kick', 'Snare', 'Hi-Hat', 'Ghost'],  // 38: Jungle
    ['Kick', 'Snare', 'Hi-Hat', 'Ride'],  // 39: Liquid DnB
    ['Clave', 'Kick', 'Polos', 'Sangsih', 'Shaker'],  // 40: Afro-Electronic
    ['Kick', 'Snare', 'Hi-Hat', 'Rim'],  // 41: Balkan Funk
    ['Kick', 'Bell', 'Hi-Hat', 'Conga', 'Rim', 'Lead'],  // 42: Comp Arc
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
      case 14: return [ // Sub-Saharan: Agbekor — 4 lanes
        mkLane('Bell', 'Anchor pulse', 56, 1, 12, 1, 7, 95, { timeline: true, fixed: [1,0,1,0,1,1,0,1,0,1,0,1] }),
        mkLane('Kidi', 'Accent', 63, 2, 5, 1, 5, 85, { prob: 0.95, spread: 0.08, humanize: 1.5 }),
        mkLane('Sogo', 'Ghost', 43, 3, 3, 1, 3, 75, { prob: 0.9, ghost: 30, spread: 0.1, humanize: 2 }),
        mkLane('Lead', 'Ornament', 38, 4, 7, 1, 4, 70, { prob: 0.8, spread: 0.12, humanize: 2.5 }),
      ];
      case 15: return [ // Gamelan: Colotomic — 4 lanes
        mkLane('Ketuk', 'Shimmer', 76, 1, 4, 2, 1, 70, { duration: 0.15 }),
        mkLane('Kempul', 'Accent', 67, 2, 8, 2, 1, 80, { duration: 0.3 }),
        mkLane('Kenong', 'Backbeat', 56, 3, 16, 2, 1, 90, { duration: 0.5 }),
        mkLane('Gong', 'Anchor pulse', 36, 4, 32, 2, 1, 110, { duration: 0.8 }),
      ];
      case 16: return [ // Polymetric Foundation — 2 lanes
        mkLane('Bell', 'Anchor pulse', 56, 1, 12, 1, 7, 90, { duration: 0.1 }),
        mkLane('Counter', 'Accent', 42, 2, 7, 1, 5, 80),
      ];
      case 17: return [ // Ewe Polymetric Ensemble — 4 lanes
        mkLane('Bell', 'Anchor pulse', 56, 1, 12, 1, 7, 110, { duration: 0.1 }),
        mkLane('Support', 'Accent', 43, 2, 12, 1, 3, 90, { ghost: 40 }),
        mkLane('Responding', 'Ghost', 45, 3, 5, 1, 3, 85, { prob: 0.9, ghost: 50 }),
        mkLane('Lead', 'Ornament', 47, 4, 7, 1, 5, 75, { prob: 0.85, ghost: 60, rot: 2, kotekanSource: 0 }),
      ];
      case 18: return [ // Manding Djembe — 3 lanes
        mkLane('Dunun', 'Anchor pulse', 36, 1, 8, 1, 3, 100),
        mkLane('Sangban', 'Accent', 43, 2, 8, 1, 5, 85, { prob: 0.95, ghost: 45, rot: 1 }),
        mkLane('Djembe', 'Shimmer', 50, 3, 8, 1, 7, 95, { prob: 0.9, ghost: 55 }),
      ];
      case 19: return [ // Cuban Son Montuno — 5 lanes
        mkLane('Clave', 'Anchor pulse', 75, 1, 16, 0.5, 5, 100, { swing: 0.25 }),
        mkLane('Cascara', 'Accent', 37, 2, 8, 1, 5, 80, { prob: 0.95, ghost: 45, rot: 1, swing: 0.25 }),
        mkLane('Tumbao', 'Backbeat', 36, 3, 8, 1, 3, 95, { swing: 0.2 }),
        mkLane('Conga', 'Ghost', 63, 4, 16, 0.5, 7, 85, { prob: 0.9, ghost: 55, rot: 2, swing: 0.3 }),
        mkLane('Shaker', 'Shimmer', 70, 5, 8, 1, 7, 50, { prob: 0.85, ghost: 30, swing: 0.2 }),
      ];
      case 20: return [ // Afrobeat Lagos — 6 lanes
        mkLane('Bell', 'Anchor pulse', 56, 1, 12, 1, 7, 105, { timeline: true, fixed: [1,0,1,0,1,1,0,1,0,1,0,1], duration: 0.1 }),
        mkLane('Hi-Hat', 'Shimmer', 42, 2, 16, 0.5, 13, 70, { prob: 0.9, ghost: 50, mutationRate: 0.05 }),
        mkLane('Kick', 'Backbeat', 36, 3, 16, 0.5, 3, 110, { duration: 0.25 }),
        mkLane('Snare', 'Accent', 38, 4, 16, 0.5, 5, 90, { prob: 0.9, ghost: 40, rot: 3, phraseLength: 8, phraseGap: 4, mutationRate: 0.2 }),
        mkLane('Shaker', 'Ornament', 70, 5, 12, 1, 9, 55, { prob: 0.85, ghost: 35, rot: 2, phraseLength: 12, phraseGap: 4, phraseOffset: 4, mutationRate: 0.1 }),
        mkLane('Conga', 'Ghost', 63, 6, 8, 1, 3, 80, { prob: 0.8, ghost: 50, rot: 1, phraseLength: 6, phraseGap: 6, phraseOffset: 8, mutationRate: 0.25 }),
      ];
      case 21: return [ // Balinese Kotekan — 4 lanes
        mkLane('Polos', 'Anchor pulse', 72, 1, 8, 0.5, 5, 90, { ghost: 40, duration: 0.12 }),
        mkLane('Sangsih', 'Accent', 74, 2, 8, 0.5, 5, 85, { prob: 0.95, ghost: 40, kotekanSource: 0, duration: 0.12 }),
        mkLane('Jegogan', 'Backbeat', 48, 3, 8, 2, 2, 100, { duration: 0.5 }),
        mkLane('Reyong', 'Ghost', 67, 4, 16, 0.5, 5, 75, { prob: 0.85, ghost: 50, rot: 3 }),
      ];
      case 22: return [ // Javanese Colotomic — 4 lanes
        mkLane('Ketuk', 'Shimmer', 76, 1, 4, 2, 1, 70, { duration: 0.15 }),
        mkLane('Kempul', 'Accent', 60, 2, 8, 2, 1, 85, { duration: 0.3 }),
        mkLane('Kenong', 'Backbeat', 55, 3, 16, 2, 1, 95, { duration: 0.5 }),
        mkLane('Gong', 'Anchor pulse', 48, 4, 32, 2, 1, 110, { duration: 0.8 }),
      ];
      case 23: return [ // Tintal Groove — 4 lanes
        mkLane('Sam', 'Anchor pulse', 36, 1, 16, 2, 4, 110, { duration: 0.3 }),
        mkLane('Theka', 'Accent', 38, 2, 16, 1, 7, 90, { prob: 0.95 }),
        mkLane('Dugun', 'Shimmer', 42, 3, 16, 1, 9, 70, { prob: 0.9, rot: 2 }),
        mkLane('Tigun', 'Ghost', 46, 4, 16, 0.5, 5, 55, { prob: 0.8, rot: 4 }),
      ];
      case 24: return [ // Rupak Tal — 3 lanes
        mkLane('Sam', 'Anchor pulse', 36, 1, 7, 2, 3, 120, { ghost: 50 }),
        mkLane('Theka', 'Accent', 38, 2, 7, 1, 4, 85, { prob: 0.9, ghost: 40, rot: 1 }),
        mkLane('Counter', 'Shimmer', 42, 3, 7, 1, 5, 70, { prob: 0.85, ghost: 35, rot: 3 }),
      ];
      case 25: return [ // Rachenitsa 7/8 — 4 lanes
        mkLane('Tupan Bass', 'Anchor pulse', 36, 1, 7, 1, 3, 110, { duration: 0.2 }),
        mkLane('Tupan Rim', 'Backbeat', 37, 2, 7, 1, 4, 85, { rot: 2 }),
        mkLane('Kaval', 'Ornament', 76, 3, 7, 1, 2, 75, { prob: 0.9, rot: 1 }),
        mkLane('Gadulka', 'Shimmer', 42, 4, 7, 0.5, 5, 65, { prob: 0.85 }),
      ];
      case 26: return [ // Kopanitsa 11/8 — 4 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 11, 1, 4, 115, { humanize: 2 }),
        mkLane('Snare', 'Backbeat', 38, 2, 11, 1, 5, 90, { prob: 0.95, rot: 3, humanize: 3 }),
        mkLane('Hi-Hat', 'Shimmer', 42, 3, 11, 0.5, 7, 70, { prob: 0.9, humanize: 2 }),
        mkLane('Bell', 'Ornament', 56, 4, 11, 1, 3, 80, { prob: 0.9, rot: 2, humanize: 2 }),
      ];
      case 27: return [ // Reich Phase Process — 3 lanes
        mkLane('Fixed', 'Anchor pulse', 76, 1, 12, 1, 5, 90, { duration: 0.15 }),
        mkLane('Drifting', 'Anchor pulse', 76, 2, 12, 1, 5, 85, { duration: 0.15, driftRate: 0.25 }),
        mkLane('Anchor', 'Shimmer', 42, 3, 4, 2, 4, 70),
      ];
      case 28: return [ // Riley Layered Entry — 5 lanes
        mkLane('Foundation', 'Anchor pulse', 36, 1, 8, 1, 5, 95),
        mkLane('Voice A', 'Accent', 42, 2, 12, 1, 7, 80, { prob: 0.9, phraseLength: 16, phraseGap: 8, phraseOffset: 4 }),
        mkLane('Voice B', 'Shimmer', 45, 3, 10, 1, 6, 75, { prob: 0.85, phraseLength: 12, phraseGap: 12, phraseOffset: 12 }),
        mkLane('Voice C', 'Ghost', 47, 4, 7, 1, 4, 70, { prob: 0.8, phraseLength: 8, phraseGap: 16, phraseOffset: 20 }),
        mkLane('Voice D', 'Ornament', 50, 5, 9, 2, 3, 65, { prob: 0.75, phraseLength: 24, phraseGap: 8, phraseOffset: 32 }),
      ];
      case 29: return [ // Nancarrow Tempi — 4 lanes
        mkLane('Anchor', 'Anchor pulse', 36, 1, 4, 2, 4, 100, { tempoMultiplier: 1.0 }),
        mkLane('Double', 'Shimmer', 42, 2, 8, 1, 5, 70, { prob: 0.9, tempoMultiplier: 2.0 }),
        mkLane('Half', 'Accent', 56, 3, 3, 2, 2, 85, { tempoMultiplier: 0.5 }),
        mkLane('Hemiola', 'Ghost', 45, 4, 6, 1, 4, 75, { prob: 0.85, tempoMultiplier: 1.5 }),
      ];
      case 30: return [ // Minimal Techno — 4 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 4, 2, 4, 120, { duration: 0.2 }),
        mkLane('Hi-Hat', 'Shimmer', 42, 2, 16, 0.5, 8, 75, { prob: 0.95 }),
        mkLane('Ghost', 'Ghost', 39, 3, 7, 1, 5, 55, { prob: 0.8, rot: 2 }),
        mkLane('Clap', 'Backbeat', 38, 4, 16, 0.5, 2, 100, { rot: 4 }),
      ];
      case 31: return [ // Deep House — 4 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 4, 2, 4, 115, { duration: 0.25, humanize: 2 }),
        mkLane('Open Hat', 'Shimmer', 46, 2, 16, 0.5, 6, 70, { prob: 0.9, rot: 3, swing: 0.4, humanize: 4 }),
        mkLane('Shaker', 'Ghost', 70, 3, 16, 0.5, 10, 50, { prob: 0.85, swing: 0.45, humanize: 5 }),
        mkLane('Rim', 'Accent', 37, 4, 8, 1, 3, 90, { prob: 0.9, rot: 1, swing: 0.35, humanize: 4 }),
      ];
      case 32: return [ // Samba Batucada — 5 lanes
        mkLane('Surdo', 'Anchor pulse', 36, 1, 4, 2, 2, 110, { rot: 1, duration: 0.4, swing: 0.2 }),
        mkLane('Tamborim', 'Accent', 50, 2, 16, 0.5, 7, 90, { prob: 0.95, ghost: 50, rot: 2, swing: 0.25 }),
        mkLane('Agogo', 'Ornament', 56, 3, 16, 0.5, 5, 85, { prob: 0.9, ghost: 40, swing: 0.2 }),
        mkLane('Repinique', 'Backbeat', 47, 4, 8, 1, 3, 100, { ghost: 30, rot: 1, swing: 0.15 }),
        mkLane('Caixa', 'Shimmer', 38, 5, 16, 0.5, 13, 70, { prob: 0.9, ghost: 55, swing: 0.25 }),
      ];
      case 33: return [ // Bossa Nova Trio — 3 lanes
        mkLane('Bass', 'Anchor pulse', 36, 1, 16, 0.5, 5, 95, { duration: 0.3, swing: 0.15 }),
        mkLane('Ride', 'Shimmer', 51, 2, 8, 1, 4, 65, { prob: 0.9, ghost: 30, swing: 0.1 }),
        mkLane('Brush', 'Ghost', 38, 3, 16, 0.5, 9, 40, { prob: 0.8, ghost: 25, rot: 3, swing: 0.2 }),
      ];
      case 34: return [ // Classic Funk — 4 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 16, 0.5, 3, 110, { duration: 0.2, spread: 0.2, timingOffset: 3 }),
        mkLane('Snare Accent', 'Backbeat', 38, 2, 8, 1, 2, 105, { spread: 0.15, rot: 4, timingOffset: -2 }),
        mkLane('Snare Ghost', 'Ghost', 38, 3, 16, 0.5, 11, 40, { prob: 0.85, ghost: 30, spread: 0.65, timingOffset: 1 }),
        mkLane('Hi-Hat', 'Shimmer', 42, 4, 16, 0.5, 14, 75, { prob: 0.9, ghost: 45, spread: 0.4 }),
      ];
      case 35: return [ // Neo-Soul Pocket — 4 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 16, 0.5, 4, 95, { spread: 0.3, timingOffset: 4 }),
        mkLane('Snare', 'Backbeat', 38, 2, 8, 1, 2, 90, { prob: 0.95, ghost: 35, spread: 0.45, rot: 4, timingOffset: -3 }),
        mkLane('Hi-Hat', 'Shimmer', 42, 3, 16, 0.5, 10, 60, { prob: 0.85, ghost: 40, spread: 0.55, timingOffset: 1 }),
        mkLane('Rim Click', 'Ornament', 37, 4, 12, 1, 5, 55, { prob: 0.8, ghost: 30, spread: 0.5, rot: 2, timingOffset: -1 }),
      ];
      case 36: return [ // Jazz Bop Ride — 4 lanes
        mkLane('Ride', 'Anchor pulse', 51, 1, 4, 2, 4, 85, { swing: 0.45 }),
        mkLane('HH Foot', 'Backbeat', 44, 2, 4, 2, 2, 70, { rot: 1, swing: 0.4 }),
        mkLane('Kick', 'Ghost', 36, 3, 8, 1, 3, 80, { prob: 0.9, ghost: 30, swing: 0.35, mutationRate: 0.15 }),
        mkLane('Snare Comp', 'Ornament', 38, 4, 16, 0.5, 5, 65, { prob: 0.8, ghost: 40, rot: 2, swing: 0.3, mutationRate: 0.25 }),
      ];
      case 37: return [ // Elvin Jones Cascade — 4 lanes
        mkLane('Ride', 'Anchor pulse', 51, 1, 4, 2, 3, 90, { swing: 0.4, mutationRate: 0.1 }),
        mkLane('Snare', 'Accent', 38, 2, 3, 2, 2, 75, { prob: 0.9, ghost: 45, swing: 0.35, mutationRate: 0.2 }),
        mkLane('Tom', 'Ghost', 45, 3, 5, 2, 3, 70, { prob: 0.85, ghost: 40, rot: 1, swing: 0.3, mutationRate: 0.2 }),
        mkLane('Bass Drum', 'Ornament', 36, 4, 7, 2, 4, 85, { prob: 0.9, ghost: 30, rot: 2, swing: 0.35, mutationRate: 0.15 }),
      ];
      case 38: return [ // Jungle Break — 4 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 16, 0.5, 5, 110, { duration: 0.15, spread: 0.25, rot: 3, mutationRate: 0.15 }),
        mkLane('Snare', 'Backbeat', 38, 2, 4, 2, 2, 105, { spread: 0.15, rot: 1, mutationRate: 0.1 }),
        mkLane('Hi-Hat', 'Shimmer', 42, 3, 16, 0.5, 7, 80, { prob: 0.9, ghost: 45, spread: 0.4, mutationRate: 0.15 }),
        mkLane('Ghost', 'Ghost', 38, 4, 16, 0.5, 11, 35, { prob: 0.75, ghost: 25, spread: 0.6, rot: 2, mutationRate: 0.2 }),
      ];
      case 39: return [ // Liquid Drum and Bass — 4 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 8, 1, 2, 100, { duration: 0.25, spread: 0.15, mutationRate: 0.05 }),
        mkLane('Snare', 'Backbeat', 38, 2, 4, 2, 2, 95, { spread: 0.1, rot: 1, mutationRate: 0.05 }),
        mkLane('Hi-Hat', 'Shimmer', 42, 3, 16, 0.5, 9, 65, { prob: 0.9, ghost: 35, spread: 0.3, mutationRate: 0.1 }),
        mkLane('Ride', 'Ghost', 51, 4, 8, 1, 5, 55, { prob: 0.85, ghost: 30, spread: 0.25, rot: 2, mutationRate: 0.1 }),
      ];
      case 40: return [ // Afro-Electronic Fusion — 5 lanes
        mkLane('Clave', 'Anchor pulse', 75, 1, 8, 1, 3, 100, { swing: 0.2 }),
        mkLane('Kick', 'Backbeat', 36, 2, 4, 2, 4, 110, { duration: 0.2 }),
        mkLane('Polos', 'Shimmer', 62, 3, 12, 1, 7, 70, { prob: 0.9, ghost: 45, swing: 0.1 }),
        mkLane('Sangsih', 'Accent', 64, 4, 12, 1, 5, 65, { prob: 0.85, ghost: 40, rot: 2, kotekanSource: 0, swing: 0.1 }),
        mkLane('Shaker', 'Ghost', 70, 5, 16, 0.5, 9, 50, { prob: 0.8, ghost: 30, rot: 3, swing: 0.15 }),
      ];
      case 41: return [ // Balkan Funk — 4 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 7, 1, 3, 105, { duration: 0.2 }),
        mkLane('Snare', 'Backbeat', 38, 2, 7, 1, 5, 85, { prob: 0.9, ghost: 60, spread: 0.35, rot: 1 }),
        mkLane('Hi-Hat', 'Shimmer', 42, 3, 14, 0.5, 9, 55, { prob: 0.85, ghost: 40, spread: 0.2, swing: 0.15 }),
        mkLane('Rim', 'Accent', 37, 4, 7, 1, 2, 90, { rot: 3 }),
      ];
      case 42: return [ // Compositional Arc — 6 lanes
        mkLane('Kick', 'Anchor pulse', 36, 1, 4, 2, 4, 110, { duration: 0.25 }),
        mkLane('Bell', 'Accent', 56, 2, 12, 1, 7, 90, { duration: 0.1 }),
        mkLane('Hi-Hat', 'Shimmer', 42, 3, 16, 0.5, 11, 55, { prob: 0.85, ghost: 50, spread: 0.25, rot: 2 }),
        mkLane('Conga', 'Ghost', 63, 4, 8, 1, 5, 75, { prob: 0.85, ghost: 40, spread: 0.15, rot: 1, phraseLength: 12, phraseGap: 4 }),
        mkLane('Rim', 'Ornament', 37, 5, 7, 1, 3, 85, { prob: 0.8, phraseLength: 8, phraseGap: 8 }),
        mkLane('Lead', 'Ghost', 47, 6, 5, 1, 3, 80, { prob: 0.75, ghost: 55, spread: 0.3, rot: 2, phraseLength: 6, phraseGap: 6 }),
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
    { complexity: 0.4, density: 0.5, syncopation: 0, swing: 0, tension: 0, humanize: 0.15 },  // 14: Agbekor
    { complexity: 0.2, density: 0.3, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 15: Gamelan Colotomic
    { complexity: 0, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 16: Foundation
    { complexity: 0, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0.15 },  // 17: Ewe
    { complexity: 0, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0.15 },  // 18: Manding
    { complexity: 0, density: 0, syncopation: 0.4, swing: 0.25, tension: 0, humanize: 0 },  // 19: Cuban Son
    { complexity: 0, density: 0.5, syncopation: 0, swing: 0, tension: 0, humanize: 0.15 },  // 20: Afrobeat Lagos
    { complexity: 0.3, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 21: Bali Kotekan
    { complexity: 0.2, density: 0.3, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 22: Java Colotomic
    { complexity: 0.4, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 23: Tintal
    { complexity: 0.4, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 24: Rupak
    { complexity: 0.4, density: 0.45, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 25: Rachenitsa
    { complexity: 0.4, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 26: Kopanitsa
    { complexity: 0.2, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 27: Reich
    { complexity: 0.3, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 28: Riley
    { complexity: 0.5, density: 0, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 29: Nancarrow
    { complexity: 0, density: 0.4, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 30: Techno
    { complexity: 0, density: 0, syncopation: 0, swing: 0.35, tension: 0, humanize: 0.1 },  // 31: House
    { complexity: 0, density: 0, syncopation: 0, swing: 0.2, tension: 0, humanize: 0.25 },  // 32: Batucada
    { complexity: 0, density: 0, syncopation: 0, swing: 0.15, tension: 0, humanize: 0.15 },  // 33: Bossa Trio
    { complexity: 0, density: 0, syncopation: 0.3, swing: 0, tension: 0.6, humanize: 0 },  // 34: Funk
    { complexity: 0, density: 0, syncopation: 0, swing: 0, tension: 0.4, humanize: 0.4 },  // 35: Neo-Soul
    { complexity: 0, density: 0, syncopation: 0, swing: 0.4, tension: 0, humanize: 0.2 },  // 36: Jazz Bop
    { complexity: 0.6, density: 0, syncopation: 0, swing: 0.35, tension: 0, humanize: 0.2 },  // 37: Elvin
    { complexity: 0, density: 0, syncopation: 0.5, swing: 0, tension: 0.6, humanize: 0 },  // 38: Jungle
    { complexity: 0, density: 0.35, syncopation: 0, swing: 0, tension: 0, humanize: 0.1 },  // 39: Liquid
    { complexity: 0.5, density: 0, syncopation: 0, swing: 0.15, tension: 0, humanize: 0 },  // 40: Afro-Electronic
    { complexity: 0, density: 0, syncopation: 0.3, swing: 0, tension: 0.4, humanize: 0 },  // 41: Balkan Funk
    { complexity: 0.4, density: 0.5, syncopation: 0, swing: 0, tension: 0, humanize: 0 },  // 42: Comp Arc
  ];

  const PRESET_SEEDS = [1, 7, 23, 42, 13, 31, 47, 55, 71, 88, 33, 17, 44, 99, 77, 63, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127];

  const TEMPO = 126;
  const EIGHTH = 60 / TEMPO / 2;
  const CONV = 120;

  function identityNoteMap() {
    const m = new Array(128);
    for (let i = 0; i < 128; i++) m[i] = i;
    return m;
  }

  function cloneLanes(lanes) {
    return lanes.map(l => ({
      ...l,
      pattern: l.pattern.slice(),
      mt: l.mt.slice(),
      accents: l.accents.slice(),
      envs: l.envs.map(e => ({ ...e })),
      fixed: l.fixed ? l.fixed.slice() : null,
      cells: l.cells ? l.cells.slice() : null,
    }));
  }

  function makeSceneData(presetIndex) {
    return {
      preset: PRESETS[presetIndex] ? PRESETS[presetIndex].name : 'Init',
      seed: PRESET_SEEDS[presetIndex] ?? 0,
      macros: { ...PRESET_MACROS[presetIndex] },
      lanes: makePresetLanes(presetIndex),
    };
  }

  const sceneStore = {
    A: makeSceneData(9),
    B: makeSceneData(9),
  };

  const state = {
    preset: 'Afrobeat 12/8', seed: 88, tempo: TEMPO,
    scene: 'A', morph: 0,
    chain: { enabled: false, mode: 0, entryCount: 0, entries: [] },
    macros: { complexity: 0.45, density: 0.5, syncopation: 0.3, swing: 0.1, tension: 0.25, humanize: 0.15 },
    lanes: makePresetLanes(9),
    presets: PRESETS,
    noteMap: identityNoteMap(),
  };

  function saveCurrentScene() {
    const key = state.scene === 'B' ? 'B' : 'A';
    sceneStore[key] = {
      preset: state.preset,
      seed: state.seed,
      macros: { ...state.macros },
      lanes: cloneLanes(state.lanes),
    };
  }

  function loadScene(key) {
    const data = sceneStore[key];
    state.preset = data.preset;
    state.seed = data.seed;
    state.macros = { ...data.macros };
    state.lanes = cloneLanes(data.lanes);
  }

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
      const initData = { preset: 'Init', seed: 0, macros: { ...state.macros }, lanes: cloneLanes(state.lanes) };
      sceneStore.A = initData;
      sceneStore.B = { preset: 'Init', seed: 0, macros: { ...state.macros }, lanes: cloneLanes(state.lanes) };
      return;
    }
    if (index < 0 || index >= PRESETS.length) return;
    state.preset = PRESETS[index].name;
    state.seed = PRESET_SEEDS[index] ?? 0;
    state.macros = { ...PRESET_MACROS[index] };
    state.lanes = makePresetLanes(index);
    saveCurrentScene();
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
        if (!l.active) return;
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
      case 'selectScene': {
        const target = payload.scene === 'Morph' ? 'Morph' : (payload.scene === 'B' ? 'B' : 'A');
        if (target !== 'Morph' && state.scene !== 'Morph') saveCurrentScene();
        state.scene = target;
        if (target !== 'Morph') loadScene(target);
        break;
      }
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
    capabilities: { canExport: false },
    getState: () => state,
    onState: (cb) => stateSubs.push(cb),
    edit: (paramId, value, gesture) => {
      if (gesture === 'begin') return;

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
