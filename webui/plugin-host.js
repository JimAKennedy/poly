'use strict';
/**
 * Plugin host: PolyHost implementation bridging to the native plugin over
 * the webview message channel. The C++ side owns all truth; this file only
 * transports. See bridge-schema.md for the wire format.
 *
 * Native side contract (choc::ui::WebView):
 *   - C++ binds `polyHostCall(jsonString)` for JS → C++.
 *   - C++ pushes via `window.polyHostPush(jsonString)` for C++ → JS.
 *   - The webview defines `window.__POLY_EMBEDDED__ = true` before load.
 */
(function () {
  const stateSubs = [];
  const frameSubs = [];
  let lastState = null;

  function send(msg) {
    msg.v = window.POLY_SCHEMA_VERSION;
    if (typeof window.polyHostCall === 'function') {
      window.polyHostCall(JSON.stringify(msg));
    } else {
      console.error('[plugin-host] native binding polyHostCall missing', msg);
    }
  }

  let _fc = 0, _sc = 0;
  let _dbg = null;
  function _updateDbg(extra) {
    if (!_dbg) {
      _dbg = document.createElement('div');
      _dbg.id = '_dbg';
      _dbg.style.cssText = 'position:fixed;bottom:2px;right:4px;font:10px/1.3 monospace;color:#8f8;background:rgba(0,0,0,.7);padding:2px 6px;z-index:9999;pointer-events:none;border-radius:3px';
      document.body.appendChild(_dbg);
    }
    _dbg.textContent = extra;
  }

  // C++ → JS entry point.
  window.polyHostPush = function (json) {
    let msg;
    try {
      msg = typeof json === 'string' ? JSON.parse(json) : json;
    } catch (e) {
      console.error('[plugin-host] bad push payload', e);
      return;
    }
    if (msg.type === 'state') {
      _sc++;
      lastState = msg.state;
      stateSubs.forEach((cb) => cb(lastState));
      _updateDbg(`SC:${_sc} FC:${_fc} lanes:${msg.state?.lanes?.length ?? '?'}`);
    } else if (msg.type === 'frame') {
      _fc++;
      frameSubs.forEach((cb) => cb(msg.frame));
      if (_fc % 30 === 0) _updateDbg(`SC:${_sc} FC:${_fc} t8:${msg.frame?.t8?.toFixed(1) ?? '?'} P:${msg.frame?.playing ? 'Y' : 'N'}`);
    }
  };

  window.PolyPluginHost = {
    schemaVersion: window.POLY_SCHEMA_VERSION,
    capabilities: { canExport: true },
    getState: () => lastState, // null until the first native push arrives
    onState: (cb) => {
      stateSubs.push(cb);
      if (lastState) cb(lastState);
    },
    edit: (paramId, value, gesture) => send({ type: 'edit', paramId, value, gesture }),
    action: (name, payload) => send({ type: 'action', name, payload }),
    onFrame: (cb) => frameSubs.push(cb),
  };

  // Announce readiness so the native side pushes the initial state.
  if (window.__POLY_EMBEDDED__) send({ type: 'ready' });
})();
