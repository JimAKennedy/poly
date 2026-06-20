---
id: T01
parent: S06
milestone: M002
key_files:
  - plugin/resource/poly.uidesc
  - plugin/source/controller.cpp
  - plugin/CMakeLists.txt
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T22:19:19.899Z
blocker_discovered: false
---

# T01: Set up VSTGUI editor scaffold with VST3Editor, uidesc, and vstgui_support

**Set up VSTGUI editor scaffold with VST3Editor, uidesc, and vstgui_support**

## What Happened

Created poly.uidesc resource file, wired createView() in controller to return VST3Editor with uidesc, linked vstgui_support library, bundled uidesc via smtg_target_add_plugin_resources. Editor opens and closes cleanly.

## Verification

Build succeeds, uidesc bundled in VST3 plugin resources

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cd build && cmake --build . && ctest --output-on-failure` | 0 | pass | 5000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/resource/poly.uidesc`
- `plugin/source/controller.cpp`
- `plugin/CMakeLists.txt`
