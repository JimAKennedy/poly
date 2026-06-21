#pragma once

#include "poly/types.h"

namespace poly {

struct PresetInfo {
    const char* name;
    const char* description;
};

static constexpr int kFactoryPresetCount = 5;

inline GrooveState makeFourOnTheFloor() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 1;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {4, 4};
    kick.hitCount = 4;
    kick.baseVelocity = 110;
    kick.probability = 1.0f;
    kick.noteDuration = 0.25f;

    auto& snare = s.lanes[1];
    snare.id = 1;
    snare.role = Role::Backbeat;
    snare.midiNote = 38;
    snare.cycle = {4, 4};
    snare.hitCount = 2;
    snare.baseVelocity = 100;
    snare.probability = 1.0f;
    snare.noteDuration = 0.25f;

    auto& hh = s.lanes[2];
    hh.id = 2;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {8, 8};
    hh.hitCount = 8;
    hh.baseVelocity = 80;
    hh.probability = 0.95f;
    hh.velocitySpread = 0.08f;
    hh.noteDuration = 0.1f;

    auto& perc = s.lanes[3];
    perc.id = 3;
    perc.role = Role::Ghost;
    perc.midiNote = 46;
    perc.cycle = {7, 8};
    perc.hitCount = 4;
    perc.baseVelocity = 60;
    perc.probability = 0.7f;
    perc.ghostFloor = 25;
    perc.velocitySpread = 0.12f;

    s.macros.density = 0.5f;
    s.macros.complexity = 0.3f;
    return s;
}

inline GrooveState makePolymetricDrift() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 7;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {3, 4};
    kick.hitCount = 2;
    kick.baseVelocity = 105;
    kick.probability = 1.0f;
    kick.noteDuration = 0.3f;

    auto& rim = s.lanes[1];
    rim.id = 1;
    rim.role = Role::Accent;
    rim.midiNote = 37;
    rim.cycle = {5, 16};
    rim.hitCount = 3;
    rim.baseVelocity = 85;
    rim.probability = 0.9f;

    auto& tom = s.lanes[2];
    tom.id = 2;
    tom.role = Role::Ghost;
    tom.midiNote = 45;
    tom.cycle = {7, 16};
    tom.hitCount = 4;
    tom.baseVelocity = 70;
    tom.probability = 0.8f;
    tom.ghostFloor = 30;
    tom.velocitySpread = 0.1f;

    auto& shimmer = s.lanes[3];
    shimmer.id = 3;
    shimmer.role = Role::Shimmer;
    shimmer.midiNote = 42;
    shimmer.cycle = {11, 16};
    shimmer.hitCount = 7;
    shimmer.baseVelocity = 65;
    shimmer.probability = 0.85f;
    shimmer.velocitySpread = 0.15f;

    s.macros.complexity = 0.7f;
    s.macros.density = 0.4f;
    s.macros.tension = 0.3f;
    return s;
}

inline GrooveState makeSparsePulse() {
    GrooveState s{};
    s.activeLaneCount = 3;
    s.seed = 23;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {2, 4};
    kick.hitCount = 1;
    kick.baseVelocity = 100;
    kick.probability = 1.0f;
    kick.noteDuration = 0.5f;

    auto& rim = s.lanes[1];
    rim.id = 1;
    rim.role = Role::Ornament;
    rim.midiNote = 37;
    rim.cycle = {3, 8};
    rim.hitCount = 2;
    rim.baseVelocity = 70;
    rim.probability = 0.8f;
    rim.humanizeMs = 3.0f;

    auto& ghost = s.lanes[2];
    ghost.id = 2;
    ghost.role = Role::Ghost;
    ghost.midiNote = 39;
    ghost.cycle = {5, 16};
    ghost.hitCount = 2;
    ghost.baseVelocity = 45;
    ghost.probability = 0.6f;
    ghost.ghostFloor = 20;
    ghost.velocitySpread = 0.2f;

    s.macros.density = 0.25f;
    s.macros.humanize = 0.3f;
    return s;
}

inline GrooveState makeBreakbeat() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 42;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {5, 16};
    kick.hitCount = 3;
    kick.baseVelocity = 115;
    kick.probability = 1.0f;
    kick.noteDuration = 0.2f;

    auto& snare = s.lanes[1];
    snare.id = 1;
    snare.role = Role::Backbeat;
    snare.midiNote = 38;
    snare.cycle = {4, 4};
    snare.hitCount = 2;
    snare.baseVelocity = 105;
    snare.probability = 1.0f;
    snare.accents.steps[1] = true;
    snare.accents.steps[3] = true;
    snare.emphasisProb = 0.8f;

    auto& hh = s.lanes[2];
    hh.id = 2;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {16, 16};
    hh.hitCount = 12;
    hh.baseVelocity = 75;
    hh.probability = 0.9f;
    hh.velocitySpread = 0.12f;
    hh.swingAmount = 0.15f;

    auto& ghost = s.lanes[3];
    ghost.id = 3;
    ghost.role = Role::Ghost;
    ghost.midiNote = 45;
    ghost.cycle = {7, 16};
    ghost.hitCount = 3;
    ghost.baseVelocity = 55;
    ghost.probability = 0.65f;
    ghost.ghostFloor = 25;

    s.macros.syncopation = 0.5f;
    s.macros.tension = 0.4f;
    s.macros.complexity = 0.6f;
    return s;
}

inline GrooveState makeLatinFeel() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 13;

    auto& clave = s.lanes[0];
    clave.id = 0;
    clave.role = Role::AnchorPulse;
    clave.midiNote = 75;
    clave.cycle = {5, 16};
    clave.hitCount = 3;
    clave.baseVelocity = 95;
    clave.probability = 1.0f;

    auto& conga = s.lanes[1];
    conga.id = 1;
    conga.role = Role::Accent;
    conga.midiNote = 63;
    conga.cycle = {7, 8};
    conga.hitCount = 4;
    conga.baseVelocity = 85;
    conga.probability = 0.9f;
    conga.velocitySpread = 0.1f;
    conga.humanizeMs = 2.0f;

    auto& shaker = s.lanes[2];
    shaker.id = 2;
    shaker.role = Role::Shimmer;
    shaker.midiNote = 70;
    shaker.cycle = {16, 16};
    shaker.hitCount = 12;
    shaker.baseVelocity = 55;
    shaker.probability = 0.85f;
    shaker.velocitySpread = 0.15f;

    auto& cowbell = s.lanes[3];
    cowbell.id = 3;
    cowbell.role = Role::Ornament;
    cowbell.midiNote = 56;
    cowbell.cycle = {4, 4};
    cowbell.hitCount = 3;
    cowbell.baseVelocity = 75;
    cowbell.probability = 0.8f;

    s.macros.swing = 0.3f;
    s.macros.humanize = 0.2f;
    s.macros.complexity = 0.4f;
    return s;
}

inline GrooveState makeFactoryPreset(int index) {
    switch (index) {
    case 0:
        return makeFourOnTheFloor();
    case 1:
        return makePolymetricDrift();
    case 2:
        return makeSparsePulse();
    case 3:
        return makeBreakbeat();
    case 4:
        return makeLatinFeel();
    default:
        return GrooveState{};
    }
}

inline PresetInfo getFactoryPresetInfo(int index) {
    static constexpr PresetInfo kInfos[kFactoryPresetCount] = {
        {"Four on the Floor", "Classic club groove with straight 8th hats and polymetric open hat"},
        {"Polymetric Drift", "Prime-number cycles (3, 5, 7, 11) creating evolving phase patterns"},
        {"Sparse Pulse", "Minimal, spacious groove with wide spacing and gentle ghost notes"},
        {"Breakbeat", "Syncopated kick with punchy snare, fast hats and ghost toms"},
        {"Latin Feel", "Clave-inspired pattern with conga, shaker and cowbell ornaments"},
    };
    if (index >= 0 && index < kFactoryPresetCount)
        return kInfos[index];
    return {"", ""};
}

} // namespace poly
