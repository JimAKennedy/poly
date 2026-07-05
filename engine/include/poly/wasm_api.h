#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define POLY_EXPORT EMSCRIPTEN_KEEPALIVE
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

POLY_EXPORT void poly_action_set_euclid(PolyContext ctx, int lane, int steps, int hits, int rotation);
POLY_EXPORT void poly_action_toggle_step(PolyContext ctx, int lane, int step);
POLY_EXPORT void poly_action_apply_preset(PolyContext ctx, int index);

POLY_EXPORT uint64_t poly_seed(PolyContext ctx);
POLY_EXPORT void poly_set_seed(PolyContext ctx, uint64_t seed);

#ifdef __cplusplus
}
#endif
