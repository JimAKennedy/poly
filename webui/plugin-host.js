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
      lastState = msg.state;
      stateSubs.forEach((cb) => cb(lastState));
    } else if (msg.type === 'frame') {
      frameSubs.forEach((cb) => cb(msg.frame));
    }
  };

  window.PolyPluginHost = {
    schemaVersion: window.POLY_SCHEMA_VERSION,
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
