#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __EMSCRIPTEN__
#define POLY_EXPORT __attribute__((used))
#else
#define POLY_EXPORT
#endif

typedef void* PolyContext;

POLY_EXPORT PolyContext poly_create();
POLY_EXPORT void poly_destroy(PolyContext ctx);

POLY_EXPORT int poly_preset_count();
POLY_EXPORT const char* poly_preset_name(int index);
POLY_EXPORT const char* poly_preset_description(int index);
POLY_EXPORT void poly_load_preset(PolyContext ctx, int index);

POLY_EXPORT int poly_render(PolyContext ctx, double ppqStart, double ppqEnd, double tempo, double sampleRate,
                            int blockSize, int playing, int looping, double loopStartPpq, double loopEndPpq,
                            int jumped);

POLY_EXPORT int poly_event_count(PolyContext ctx);
POLY_EXPORT double* poly_event_buffer(PolyContext ctx);

POLY_EXPORT int poly_active_lane_count(PolyContext ctx);
POLY_EXPORT double poly_macro_value(PolyContext ctx, int index);
POLY_EXPORT void poly_set_macro(PolyContext ctx, int index, double value);

POLY_EXPORT void poly_edit_lane_int(PolyContext ctx, int lane, int field, int value);
POLY_EXPORT void poly_edit_lane_float(PolyContext ctx, int lane, int field, double value);
POLY_EXPORT int poly_lane_int(PolyContext ctx, int lane, int field);
POLY_EXPORT double poly_lane_float(PolyContext ctx, int lane, int field);

POLY_EXPORT int poly_lane_pattern(PolyContext ctx, int lane, int* out, int maxLen);
POLY_EXPORT float* poly_lane_micro_timing_ptr(PolyContext ctx, int lane);
POLY_EXPORT float* poly_lane_accents_ptr(PolyContext ctx, int lane);
POLY_EXPORT int* poly_lane_cells_ptr(PolyContext ctx, int lane);
POLY_EXPORT int poly_lane_fixed_pattern(PolyContext ctx, int lane, int* out, int maxLen);
POLY_EXPORT int poly_lane_envelope_count(PolyContext ctx, int lane);
POLY_EXPORT int poly_lane_envelope_active(PolyContext ctx, int lane, int index);
POLY_EXPORT int poly_lane_envelope_target(PolyContext ctx, int lane, int index);
POLY_EXPORT float poly_lane_envelope_period(PolyContext ctx, int lane, int index);
POLY_EXPORT float poly_lane_envelope_depth(PolyContext ctx, int lane, int index);

POLY_EXPORT void poly_action_set_euclid(PolyContext ctx, int lane, int steps, int hits, int rotation);
POLY_EXPORT void poly_action_toggle_step(PolyContext ctx, int lane, int step);
POLY_EXPORT void poly_action_apply_preset(PolyContext ctx, int index);
// M043 S14 T01: scene-aware preset apply. `targetScene` is 0 = A, 1 = B; any
// other value falls back to the current selection. Morph selection routes to
// A because Morph is a render-time blend, not a writable slot.
POLY_EXPORT void poly_action_apply_preset_to_scene(PolyContext ctx, int index, int targetScene);
POLY_EXPORT void poly_action_select_scene(PolyContext ctx, int sceneSelect);
POLY_EXPORT int poly_scene_select(PolyContext ctx);
// M043 S14 T02: morph amount push/pull and scene-state snapshot for
// audible Morph playback. See wasm_api.cpp for rationale.
POLY_EXPORT void poly_set_morph(PolyContext ctx, double amount);
POLY_EXPORT double poly_morph_amount(PolyContext ctx);
POLY_EXPORT void poly_copy_scenes(PolyContext dst, PolyContext src);
POLY_EXPORT void poly_action_set_fixed_step(PolyContext ctx, int lane, int step, int on);
POLY_EXPORT void poly_action_set_micro_timing(PolyContext ctx, int lane, int step, float ms);
POLY_EXPORT void poly_action_set_accent(PolyContext ctx, int lane, int step, float value);
POLY_EXPORT void poly_action_set_cells(PolyContext ctx, int lane, const int* cells, int count);
POLY_EXPORT void poly_action_clear_cells(PolyContext ctx, int lane);
POLY_EXPORT void poly_action_set_envelope(PolyContext ctx, int lane, int index, int target, float period, float depth,
                                          int active);
POLY_EXPORT void poly_action_remove_envelope(PolyContext ctx, int lane, int index);

POLY_EXPORT uint64_t poly_seed(PolyContext ctx);
POLY_EXPORT void poly_set_seed(PolyContext ctx, uint64_t seed);

#ifdef __cplusplus
}
#endif
