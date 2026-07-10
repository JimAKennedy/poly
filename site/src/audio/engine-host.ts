// Lazy loader for the WASM poly engine, shared across all preview cards on a
// page. `initEngine()` returns a Promise that resolves to the loaded Module and
// a shared engineCtx; both are cached so multiple Play clicks reuse the same
// WASM instance. Real playback allocates a fresh preset on the shared ctx;
// dumps use a per-call scratch ctx so they never perturb playback.

export interface EngineModule {
  _poly_create(): number;
  _poly_destroy(ctx: number): void;
  _poly_preset_count(): number;
  _poly_preset_name(index: number): number;
  _poly_preset_description(index: number): number;
  _poly_load_preset(ctx: number, index: number): void;
  _poly_render(
    ctx: number,
    ppqStart: number,
    ppqEnd: number,
    tempo: number,
    sampleRate: number,
    blockSize: number,
    playing: number,
    looping: number,
    loopStartPpq: number,
    loopEndPpq: number,
    jumped: number,
  ): number;
  _poly_event_count(ctx: number): number;
  _poly_event_buffer(ctx: number): number;
  _poly_active_lane_count(ctx: number): number;
  _poly_lane_int(ctx: number, lane: number, field: number): number;
  _poly_lane_float(ctx: number, lane: number, field: number): number;
  _poly_edit_lane_int(ctx: number, lane: number, field: number, value: number): void;
  _poly_macro_value(ctx: number, index: number): number;
  _poly_seed(ctx: number): bigint | number;
  HEAPF64: Float64Array;
  UTF8ToString(ptr: number): string;
}

interface ModuleFactoryOptions {
  locateFile?: (path: string, prefix: string) => string;
}
type ModuleFactory = (opts?: ModuleFactoryOptions) => Promise<EngineModule>;

let modulePromise: Promise<EngineModule> | null = null;
let sharedCtx: number | null = null;

export function resolveEngineJsUrl(baseUrl: string): string {
  const trimmed = baseUrl.endsWith('/') ? baseUrl : `${baseUrl}/`;
  const rel = `${trimmed}webui/poly_engine.js`;
  if (typeof location === 'undefined') return rel;
  // Emscripten's locateFile calls `new URL(path, engineJsUrl)`, which requires
  // an absolute base. Normalize against the current origin so the .wasm sibling
  // resolves correctly.
  return new URL(rel, location.origin).href;
}

// Import an ES module from a URL that Vite refuses to serve as a module
// (poly_engine.js lives under /public and is copied as a static asset). Fetch
// the source, wrap it in a Blob, and import that. The Blob URL bypasses Vite's
// resolver entirely — the browser sees a plain ESM import.
async function importEngineFactory(url: string): Promise<ModuleFactory> {
  const res = await fetch(url);
  if (!res.ok) throw new Error(`engine-host: fetch ${url} → ${res.status}`);
  const text = await res.text();
  const blob = new Blob([text], { type: 'text/javascript' });
  const blobUrl = URL.createObjectURL(blob);
  try {
    const mod = (await import(/* @vite-ignore */ blobUrl)) as {
      default: ModuleFactory;
    };
    return mod.default;
  } finally {
    // Safe to revoke; the import has already been resolved and cached by the
    // module system.
    URL.revokeObjectURL(blobUrl);
  }
}

export function loadEngineModule(baseUrl: string): Promise<EngineModule> {
  if (modulePromise) return modulePromise;
  const url = resolveEngineJsUrl(baseUrl);
  modulePromise = (async () => {
    const factory = await importEngineFactory(url);
    // Emscripten resolves poly_engine.wasm via import.meta.url of the JS
    // module. Loading through a Blob would derive a bogus blob: sibling URL,
    // so we override locateFile to point back at the original static asset
    // path.
    return factory({
      locateFile: (path: string) => new URL(path, url).href,
    });
  })();
  return modulePromise;
}

export async function getSharedEngine(
  baseUrl: string,
): Promise<{ Module: EngineModule; ctx: number }> {
  const Module = await loadEngineModule(baseUrl);
  if (sharedCtx === null) sharedCtx = Module._poly_create();
  return { Module, ctx: sharedCtx };
}

// _poly_event_buffer returns a pointer to 6 doubles per event:
//   (ppqPosition, midiPitch, velocity, duration, channel, laneIndex)
export const EVENT_STRIDE = 6;

export interface EngineEvent {
  ppq: number;
  note: number;
  velocity: number;
  duration: number;
  channel: number;
  laneIndex: number;
}

export function readEngineEvents(
  Module: EngineModule,
  ctx: number,
  count: number,
): EngineEvent[] {
  const bufPtr = Module._poly_event_buffer(ctx);
  const base = bufPtr >> 3; // HEAPF64 index for doubles
  const out: EngineEvent[] = new Array(count);
  for (let i = 0; i < count; i++) {
    const off = base + i * EVENT_STRIDE;
    out[i] = {
      ppq: Module.HEAPF64[off + 0],
      note: Module.HEAPF64[off + 1],
      velocity: Module.HEAPF64[off + 2],
      duration: Module.HEAPF64[off + 3],
      channel: Module.HEAPF64[off + 4],
      laneIndex: Module.HEAPF64[off + 5],
    };
  }
  return out;
}

// Test-only reset so the module cache is dropped between suites. Never called
// from card runtime.
export function __resetEngineHostForTests(): void {
  modulePromise = null;
  sharedCtx = null;
}
