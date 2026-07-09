// S12 dump-mode gate. Enabled by ?dump=1 or ?diag=1 in the page URL. When on,
// the site card scheduler (T02) and the WASM Try It host (T03) each capture
// the first 8 bars of playback into an SMF blob and auto-download it. Debug
// diagnostic only — normal playback callers see no behavioral change.
//
// The Play↔Try It equivalence spec relies on a second companion download per
// dump: a `.params.json` snapshot capturing what each surface *thinks* the
// preset says. The Play surface dumps its site-view of the JSON preset (the
// fields the JS euclidean scheduler consumes); the Try It surface dumps its
// post-applyPreset engine state. The spec diffs the intersection to catch
// silent mis-interpretation of the shared presets.json.

export function isDumpModeEnabled(searchOverride?: string): boolean {
  const search =
    searchOverride ??
    (typeof window !== 'undefined' && window.location
      ? window.location.search
      : '');
  if (!search) return false;
  const params = new URLSearchParams(search);
  return params.get('dump') === '1' || params.get('diag') === '1';
}

export function slugifyPresetName(name: string): string {
  return name
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, '-')
    .replace(/(^-+)|(-+$)/g, '') || 'preset';
}

// Optional `?dumpBeats=N` URL override. Returns undefined when the param is
// absent or unparseable; the equivalence spec uses this to constrain both
// surfaces to the same short synchronous render window instead of waiting for
// 8 real-time loops (which is unusable for presets with large loopBeats).
export function dumpBeatsOverride(searchOverride?: string): number | undefined {
  const search =
    searchOverride ??
    (typeof window !== 'undefined' && window.location
      ? window.location.search
      : '');
  if (!search) return undefined;
  const raw = new URLSearchParams(search).get('dumpBeats');
  if (!raw) return undefined;
  const n = Number(raw);
  return Number.isFinite(n) && n > 0 ? n : undefined;
}

export function downloadJson(payload: unknown, filename: string): void {
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
