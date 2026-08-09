// Poly test transport — Cubase MIDI Remote script.
//
// Drives Cubase's transport from a virtual MIDI port so the mido driver
// (tests/cubase/driver/play_scenario.py) can start/stop playback headlessly,
// and emits a "ready" ping on activation so wait-for-ready.ps1 knows the
// remote surface is live (a stronger signal than S07's window-present check).
//
// This is a driver script, not a hardware controller script: the "device" is
// the loopMIDI virtual port pair named "poly-test". Cubase auto-detects driver
// scripts by a STRICT filename+folder convention derived from makeDeviceDriver:
//   <Documents>/Steinberg/Cubase/MIDI Remote/Driver Scripts/Local/<vendor>/<device>/<vendor>_<device>.js
// so the file MUST be named JkDigital_PolyTest.js and live under
//   .../Local/JkDigital/PolyTest/JkDigital_PolyTest.js
// A mismatched filename (e.g. poly-transport.js) is silently ignored — the
// script never loads and no surface connects. The vendor/device are single
// tokens (no spaces) so the path/filename derivation is unambiguous.
// (see README.md for the exact path and the CC map the driver must match).
//
// Protocol (channel 1 == API channel index 0). These constants are the
// contract shared with tests/cubase/driver/play_scenario.py — keep them in
// sync on both sides.
//   CC 20 -> transport START   (value >= 64 triggers)
//   CC 21 -> transport STOP     (value >= 64 triggers)
//   CC 22 -> LOCATE to zero      (value >= 64 triggers; "To Left Locator")
//   ready ping OUT: CC 119 value 127 on channel 1, sent on activation.

var midiremote_api = require('midiremote_api_v1')

// --- Protocol constants (must match play_scenario.py) ---
var CHANNEL = 0 // API channel index 0 == MIDI channel 1
var CC_START = 20
var CC_STOP = 21
var CC_LOCATE = 22
var CC_READY = 119 // undefined CC in GM — safe sentinel for the ready ping
var READY_VALUE = 127
// loopMIDI virtual port pair name. loopMIDI appends an instance suffix that
// cannot be removed (e.g. the OS-enumerated name is "poly-test 1"), so this is
// matched as a SUBSTRING, not an exact name — see the detection unit below. The
// mido driver (play_scenario.py find_port) already matches by substring; both
// sides agree the port name CONTAINS "poly-test".
var PORT_NAME = 'poly-test'

// --- Device driver + virtual port pair ---
// Vendor/device are single tokens (no spaces) so Cubase's filename+folder
// derivation is unambiguous: file JkDigital_PolyTest.js under Local/JkDigital/PolyTest/.
var driver = midiremote_api.makeDeviceDriver('JkDigital', 'PolyTest', 'Jim Kennedy')

var midiInput = driver.mPorts.makeMidiInput()
var midiOutput = driver.mPorts.makeMidiOutput()

// Substring detection: loopMIDI enumerates the port as "poly-test 1" (the
// instance suffix is not removable), so expectNameEquals("poly-test") would
// never bind and the script would never load / never ping ready. Contains
// tolerates the suffix and matches the driver's substring semantics.
driver.makeDetectionUnit()
    .detectPortPair(midiInput, midiOutput)
    .expectInputNameContains(PORT_NAME)
    .expectOutputNameContains(PORT_NAME)

// --- Surface: three momentary buttons, one per transport command ---
// Buttons are off-screen coordinates; this surface is never shown, it only
// exists to carry the MIDI bindings.
var surface = driver.mSurface
var startButton = surface.makeButton(0, 0, 1, 1)
var stopButton = surface.makeButton(1, 0, 1, 1)
var locateButton = surface.makeButton(2, 0, 1, 1)

startButton.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_START)
stopButton.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_STOP)
locateButton.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_LOCATE)

// --- Host mapping: buttons -> transport ---
var page = driver.mMapping.makePage('Poly Test Transport')

// Start / Stop bind to the transport value directly. setTypeToggle() is NOT
// used: the driver sends discrete start and stop commands, so each button is a
// one-shot trigger rather than a toggle of a single button.
page.makeValueBinding(startButton.mSurfaceValue, page.mHostAccess.mTransport.mValue.mStart)
page.makeValueBinding(stopButton.mSurfaceValue, page.mHostAccess.mTransport.mValue.mStop)

// Locate-to-zero uses the "To Left Locator" transport command; the fixture's
// left locator is at bar 1 (see tests/cubase/fixtures/README.md), so this
// returns the cursor to the scenario start before a run.
page.makeCommandBinding(locateButton.mSurfaceValue, 'Transport', 'To Left Locator')

// --- Ready ping on activation ---
// Emit the ready sentinel so the mido driver's wait_for_ready can proceed.
//
// The ping MUST fire from driver.mOnActivate, NOT page.mOnActivate. The device
// driver's mOnActivate fires when the surface CONNECTS (the port pair is
// detected and bound) — which on the headless runner happens automatically.
// page.mOnActivate, by contrast, only fires when Cubase makes that MAPPING PAGE
// the active page, which requires the MIDI Remote surface to be focused/selected
// in the UI. On an unattended nightly the page is never activated, so a
// page-bound ping never sends and the driver times out (observed on every armed
// run through 2026-08-09). The driver hook receives an activeDevice handle,
// which is exactly what sendMidi needs.
driver.mOnActivate = function (activeDevice) {
    midiOutput.sendMidi(activeDevice, [0xB0 + CHANNEL, CC_READY, READY_VALUE])
}
