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
//   CC 23 -> File > SAVE          (value >= 64 triggers)            M004 S01
//   CC 24 -> Poly's EDITOR        (absolute: >= 64 open, < 64 close) M004 S04
//   CC 25 -> Poly's automation W  (absolute: >= 64 on, < 64 off)     M004 S07
//   CC 26 -> Poly's automation R  (absolute: >= 64 on, < 64 off)     M004 S07
//   CC 27 -> Poly PARAM_INDEX     (absolute: value / 127)            M004 S07
//   CC 28 -> File > EXPORT AUDIO MIXDOWN (value >= 64 opens the dialog) M004 S06
//   CC 117 OUT -> param echo  (value: the parameter's process value * 126, read
//                              back after every CC 27 -- proof the set landed;
//                              127 means it could not be resolved)
//   CC 118 IN  -> ready POLL  (driver asks "are you live?")
//   CC 119 OUT -> ready ping  (value 127; script's reply to a poll)
//
// "Poly" in CC 24-27 is the FIRST instrument channel in the MixConsole, which
// every fixture puts on track 1 (tests/cubase/fixtures/README.md). It is
// addressed by mixer position rather than through mTrackSelection because the
// fixtures save with the probe track selected, and a binding that followed the
// selection would drive the probe.

var midiremote_api = require('midiremote_api_v1')

// --- Protocol constants (must match play_scenario.py) ---
var CHANNEL = 0 // API channel index 0 == MIDI channel 1
var CC_START = 20
var CC_STOP = 21
var CC_LOCATE = 22
var CC_SAVE = 23
var CC_EDITOR = 24
var CC_AUTO_WRITE = 25
var CC_AUTO_READ = 26
var CC_PARAM = 27
var CC_EXPORT = 28
var CC_PARAM_ECHO = 117 // undefined CC in GM — echo of CC_PARAM's result (OUT)
// The parameter CC_PARAM drives: lane 0's "Active", Poly's parameter index 8
// (ParamIDs::laneParam(0, kActive) in plugin/source/plugids.h). Why this
// parameter is recorded in tests/cubase/e2e/lib/automation-contract.ts.
var PARAM_INDEX = 8
var PARAM_TITLE = 'Active'
// Echo values: the read-back process value scaled to 0..126, or 127 when the
// parameter could not be resolved -- distinct from every success value.
var PARAM_ECHO_FAILED = 127
var CC_POLL = 118 // undefined CC in GM — driver's "are you live?" poll (IN)
var CC_READY = 119 // undefined CC in GM — safe sentinel for the ready ping (OUT)
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

// --- Surface: four momentary buttons — three transport, one ready-poll ---
// Buttons are off-screen coordinates; this surface is never shown, it only
// exists to carry the MIDI bindings.
var surface = driver.mSurface
var startButton = surface.makeButton(0, 0, 1, 1)
var stopButton = surface.makeButton(1, 0, 1, 1)
var locateButton = surface.makeButton(2, 0, 1, 1)
var pollButton = surface.makeButton(3, 0, 1, 1)

startButton.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_START)
stopButton.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_STOP)
locateButton.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_LOCATE)
pollButton.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_POLL)

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

// --- M004: save, editor, automation switches and one parameter on Poly ---
var saveButton = surface.makeButton(4, 0, 1, 1)
var editorKnob = surface.makeKnob(5, 0, 1, 1)
var autoWriteKnob = surface.makeKnob(6, 0, 1, 1)
var autoReadKnob = surface.makeKnob(7, 0, 1, 1)
var paramKnob = surface.makeKnob(8, 0, 1, 1)
var exportButton = surface.makeButton(9, 0, 1, 1)

saveButton.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_SAVE)
editorKnob.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_EDITOR)
autoWriteKnob.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_AUTO_WRITE)
autoReadKnob.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_AUTO_READ)
paramKnob.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_PARAM)
exportButton.mSurfaceValue.mMidiBinding.setInputPort(midiInput).bindToControlChange(CHANNEL, CC_EXPORT)

page.makeCommandBinding(saveButton.mSurfaceValue, 'File', 'Save')
page.makeCommandBinding(exportButton.mSurfaceValue, 'File', 'Export Audio Mixdown')

var polyBank = page.mHostAccess.mMixConsole.makeMixerBankZone('Poly').includeInstrumentChannels()
var polySlot = polyBank.makeMixerBankChannel().mInstrumentPluginSlot
page.makeValueBinding(editorKnob.mSurfaceValue, polySlot.mEdit)
page.makeValueBinding(autoWriteKnob.mSurfaceValue, polySlot.mAutomationWrite)
page.makeValueBinding(autoReadKnob.mSurfaceValue, polySlot.mAutomationRead)

// The parameter goes through direct access, which addresses a VST3 ParamID by
// tag. The parameter bank zone would also reach it, but by bank position, and
// that position is Cubase's ordering rather than Poly's.
var polyAccess = page.mHostAccess.makeDirectAccess(polySlot)
var activeMapping = null
page.mOnActivate = function (activeDevice, mapping) {
    activeMapping = mapping
}
paramKnob.mSurfaceValue.mOnProcessValueChange = function (activeDevice, value) {
    var echo = PARAM_ECHO_FAILED
    if (activeMapping) {
        // The instrument slot's own object carries the slot's parameters
        // (Freeze, Activate Output, Extract Sound); the plugin is its one child.
        var slotID = polyAccess.getBaseObjectID(activeMapping)
        var pluginID = polyAccess.getChildObjectID(activeMapping, slotID, 0)
        // Direct-access tags are Cubase's numbering, not Poly's ParamIDs (lane
        // 0's Active is tag 4209 on Cubase 14), so resolve the tag from the
        // index, which does follow Poly's registration order, and refuse to
        // drive a parameter whose title is not the one expected.
        var tag = polyAccess.getParameterTagByIndex(activeMapping, pluginID, PARAM_INDEX)
        if (polyAccess.getParameterTitle(activeMapping, pluginID, tag, 32) === PARAM_TITLE) {
            polyAccess.setParameterProcessValue(activeMapping, pluginID, tag, value)
            echo = Math.round(polyAccess.getParameterProcessValue(activeMapping, pluginID, tag) * 126)
        }
    }
    midiOutput.sendMidi(activeDevice, [0xB0 + CHANNEL, CC_PARAM_ECHO, echo])
}

// --- Ready handshake: driver polls, script replies ---
// Readiness is driver-initiated instead of relying on the one-shot
// driver.mOnActivate. mOnActivate fires exactly once, at surface-connect time,
// which on an unattended nightly happens DURING Cubase load — before
// play_scenario.py has opened its input port. mido only buffers messages after
// the port is open, so that single fire-and-forget ping was already gone by the
// time the driver started listening, and the driver timed out on every armed
// run through 2026-08-09 despite the ping being emitted correctly (confirmed by
// hand: with the listener opened first, the CC119=127 ping arrives on schedule).
//
// The poll/reply handshake removes the ordering dependency entirely: the driver
// (which controls exactly when it starts listening) sends CC_POLL every loop
// iteration while waiting, and this handler replies with the ready ping each
// time it sees one. Whenever the poll lands after the surface is connected, the
// reply arrives within one poll interval — robust to launch-then-listen,
// listen-then-launch, and a slow cold Cubase load alike.
//
// mOnProcessValueChange fires on every CC_POLL the pollButton receives; the
// callback's second arg is the live activeDevice handle sendMidi needs. Guard on
// a rising value so we reply once per poll, not on the button's zero-reset.
pollButton.mSurfaceValue.mOnProcessValueChange = function (activeDevice, value) {
    if (value <= 0) return
    midiOutput.sendMidi(activeDevice, [0xB0 + CHANNEL, CC_READY, READY_VALUE])
}
