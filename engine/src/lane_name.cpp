// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include "poly/lane_name.h"

#include <cstring>

namespace poly {

// GM percussion mapping — matches the taxonomy used by the site sample manifest
// and preset-patterns.ts. Anything not in this table falls through to "perc".
// This is the single source of truth; emit_presets.cpp consumes it via the
// public noteLabel() below rather than keeping its own copy.
const char* noteLabel(int note) {
    switch (note) {
    case 35:
    case 36:
        return "kick";
    case 37:
        return "rim";
    case 38:
    case 40:
        return "snare";
    case 39:
        return "clap";
    case 41:
    case 43:
    case 45:
    case 47:
    case 48:
    case 50:
        return "tom";
    case 42:
    case 44:
        return "hat";
    case 46:
        return "openhat";
    case 49:
    case 51:
    case 52:
    case 53:
    case 55:
    case 57:
    case 59:
        return "cymbal";
    case 56:
        return "cowbell";
    case 60:
    case 61:
        return "bongo";
    case 62:
    case 63:
    case 64:
        return "conga";
    case 65:
    case 66:
        return "timbale";
    case 67:
    case 68:
        return "agogo";
    case 69:
    case 70:
    case 71:
    case 72:
        return "shaker";
    case 73:
    case 74:
        return "guiro";
    case 75:
        return "clave";
    case 76:
    case 77:
        return "woodblock";
    case 78:
    case 79:
        return "cuica";
    case 80:
    case 81:
        return "triangle";
    default:
        return "perc";
    }
}

// Title-case / friendly presentation of each taxonomy label. Keyed off the
// single-sourced noteLabel() table so a new GM note only has to be added in one
// place. Only labels with a non-trivial spelling need an entry; everything else
// is capitalized from the taxonomy label itself.
const char* laneName(int note) {
    const char* label = noteLabel(note);
    if (std::strcmp(label, "kick") == 0)
        return "Kick";
    if (std::strcmp(label, "rim") == 0)
        return "Rim";
    if (std::strcmp(label, "snare") == 0)
        return "Snare";
    if (std::strcmp(label, "clap") == 0)
        return "Clap";
    if (std::strcmp(label, "tom") == 0)
        return "Tom";
    if (std::strcmp(label, "hat") == 0)
        return "Hi-Hat";
    if (std::strcmp(label, "openhat") == 0)
        return "Open Hat";
    if (std::strcmp(label, "cymbal") == 0)
        return "Cymbal";
    if (std::strcmp(label, "cowbell") == 0)
        return "Cowbell";
    if (std::strcmp(label, "bongo") == 0)
        return "Bongo";
    if (std::strcmp(label, "conga") == 0)
        return "Conga";
    if (std::strcmp(label, "timbale") == 0)
        return "Timbale";
    if (std::strcmp(label, "agogo") == 0)
        return "Agogo";
    if (std::strcmp(label, "shaker") == 0)
        return "Shaker";
    if (std::strcmp(label, "guiro") == 0)
        return "Guiro";
    if (std::strcmp(label, "clave") == 0)
        return "Clave";
    if (std::strcmp(label, "woodblock") == 0)
        return "Woodblock";
    if (std::strcmp(label, "cuica") == 0)
        return "Cuica";
    if (std::strcmp(label, "triangle") == 0)
        return "Triangle";
    return "Perc";
}

} // namespace poly
