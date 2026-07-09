'use strict';
/**
 * Plain-JS port of site/src/audio/dump-mode.ts. Same semantics: dump is
 * enabled when the current URL contains ?dump=1 or ?diag=1. Used by the WASM
 * Try It path in wasm-host.js.
 */
(function () {
  function isDumpModeEnabled(searchOverride) {
    let search = searchOverride;
    if (typeof search !== 'string') {
      search =
        typeof window !== 'undefined' && window.location
          ? window.location.search
          : '';
    }
    if (!search) return false;
    const params = new URLSearchParams(search);
    return params.get('dump') === '1' || params.get('diag') === '1';
  }

  function slugifyPresetName(name) {
    const s = String(name || '')
      .toLowerCase()
      .replace(/[^a-z0-9]+/g, '-')
      .replace(/(^-+)|(-+$)/g, '');
    return s || 'preset';
  }

  function downloadJson(payload, filename) {
    if (typeof document === 'undefined' || typeof URL === 'undefined') return;
    const blob = new Blob([JSON.stringify(payload, null, 2)], {
      type: 'application/json',
    });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  }

  function dumpBeatsOverride(searchOverride) {
    let search = searchOverride;
    if (typeof search !== 'string') {
      search =
        typeof window !== 'undefined' && window.location
          ? window.location.search
          : '';
    }
    if (!search) return undefined;
    const raw = new URLSearchParams(search).get('dumpBeats');
    if (!raw) return undefined;
    const n = Number(raw);
    return Number.isFinite(n) && n > 0 ? n : undefined;
  }

  window.PolyDumpMode = {
    isDumpModeEnabled,
    slugifyPresetName,
    downloadJson,
    dumpBeatsOverride,
  };
})();
