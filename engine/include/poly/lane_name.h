// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

namespace poly {

// Single source of truth for the GM percussion note -> taxonomy label mapping.
// Returns a lowercase taxonomy label (e.g. "kick", "snare", "hat", "openhat").
// Anything not in the table falls through to "perc". Matches the taxonomy used
// by the site sample manifest and preset-patterns.ts. Consumed by the preset
// emitter (engine/tools/emit_presets.cpp) and by laneName() below, so both share
// exactly one table.
const char* noteLabel(int note);

// Human-readable lane name for a GM percussion note, used for SMF Format-1
// track names (e.g. 36 -> "Kick", 38 -> "Snare", 42 -> "Hi-Hat"). Derived from
// noteLabel() so the note->name mapping stays single-sourced; only the
// presentation (title case, friendly spelling) differs. Never returns nullptr.
const char* laneName(int note);

} // namespace poly
