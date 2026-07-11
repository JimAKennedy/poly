#include "poly/presets.h"

namespace poly {

GrooveState makeFourOnTheFloor() {
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
    s.macros.complexity = 0.5f;
    return s;
}

GrooveState makePolymetricDrift() {
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

GrooveState makeSparsePulse() {
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

GrooveState makeBreakbeat() {
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
    snare.accents.steps[1] = 1.0f;
    snare.accents.steps[3] = 1.0f;
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

GrooveState makeLatinFeel() {
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

GrooveState makeAfroHousePhrases() {
    GrooveState s{};
    s.activeLaneCount = 5;
    s.seed = 31;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {4, 4};
    kick.hitCount = 3;
    kick.baseVelocity = 110;
    kick.probability = 1.0f;
    kick.noteDuration = 0.25f;
    kick.phraseLength = 16.0f;
    kick.phraseGap = 0.0f;

    auto& shaker = s.lanes[1];
    shaker.id = 1;
    shaker.role = Role::Shimmer;
    shaker.midiNote = 70;
    shaker.cycle = {16, 16};
    shaker.hitCount = 12;
    shaker.baseVelocity = 60;
    shaker.probability = 0.9f;
    shaker.velocitySpread = 0.12f;

    auto& conga = s.lanes[2];
    conga.id = 2;
    conga.role = Role::Accent;
    conga.midiNote = 63;
    conga.cycle = {7, 8};
    conga.hitCount = 4;
    conga.baseVelocity = 85;
    conga.probability = 0.9f;
    conga.phraseLength = 8.0f;
    conga.phraseGap = 2.0f;
    conga.phraseOffset = 0.0f;
    conga.humanizeMs = 2.0f;

    auto& djembe = s.lanes[3];
    djembe.id = 3;
    djembe.role = Role::Ghost;
    djembe.midiNote = 43;
    djembe.cycle = {5, 8};
    djembe.hitCount = 3;
    djembe.baseVelocity = 70;
    djembe.probability = 0.85f;
    djembe.phraseLength = 12.0f;
    djembe.phraseGap = 4.0f;
    djembe.phraseOffset = 4.0f;
    djembe.ghostFloor = 25;
    djembe.velocitySpread = 0.1f;

    auto& perc = s.lanes[4];
    perc.id = 4;
    perc.role = Role::Ornament;
    perc.midiNote = 56;
    perc.cycle = {3, 4};
    perc.hitCount = 2;
    perc.baseVelocity = 65;
    perc.probability = 0.75f;
    perc.phraseLength = 4.0f;
    perc.phraseGap = 4.0f;
    perc.phraseOffset = 8.0f;

    s.macros.swing = 0.15f;
    s.macros.humanize = 0.15f;
    s.macros.density = 0.45f;
    return s;
}

GrooveState makeReichPhasing() {
    GrooveState s{};
    s.activeLaneCount = 3;
    s.seed = 47;

    auto& fixed = s.lanes[0];
    fixed.id = 0;
    fixed.role = Role::AnchorPulse;
    fixed.midiNote = 76;
    fixed.cycle = {5, 12};
    fixed.hitCount = 3;
    fixed.baseVelocity = 90;
    fixed.probability = 1.0f;
    fixed.noteDuration = 0.15f;

    auto& drifting = s.lanes[1];
    drifting.id = 1;
    drifting.role = Role::AnchorPulse;
    drifting.midiNote = 76;
    drifting.cycle = {5, 12};
    drifting.hitCount = 3;
    drifting.baseVelocity = 90;
    drifting.probability = 1.0f;
    drifting.noteDuration = 0.15f;
    drifting.driftRate = 0.25f;

    auto& pulse = s.lanes[2];
    pulse.id = 2;
    pulse.role = Role::Shimmer;
    pulse.midiNote = 42;
    pulse.cycle = {4, 4};
    pulse.hitCount = 4;
    pulse.baseVelocity = 45;
    pulse.probability = 0.8f;
    pulse.velocitySpread = 0.05f;

    s.macros.complexity = 0.2f;
    s.macros.density = 0.3f;
    return s;
}

GrooveState makeKotekanInterlock() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 55;

    auto& polos = s.lanes[0];
    polos.id = 0;
    polos.role = Role::AnchorPulse;
    polos.midiNote = 76;
    polos.cycle = {3, 8};
    polos.hitCount = 3;
    polos.baseVelocity = 95;
    polos.probability = 1.0f;
    polos.noteDuration = 0.12f;

    auto& sangsih = s.lanes[1];
    sangsih.id = 1;
    sangsih.role = Role::Accent;
    sangsih.midiNote = 77;
    sangsih.cycle = {3, 8};
    sangsih.hitCount = 3;
    sangsih.baseVelocity = 85;
    sangsih.probability = 0.95f;
    sangsih.noteDuration = 0.12f;
    sangsih.kotekanSourceLane = 0;

    auto& gong = s.lanes[2];
    gong.id = 2;
    gong.role = Role::Backbeat;
    gong.midiNote = 36;
    gong.cycle = {4, 4};
    gong.hitCount = 1;
    gong.baseVelocity = 100;
    gong.probability = 1.0f;
    gong.noteDuration = 0.5f;

    auto& shimmer = s.lanes[3];
    shimmer.id = 3;
    shimmer.role = Role::Ghost;
    shimmer.midiNote = 42;
    shimmer.cycle = {7, 16};
    shimmer.hitCount = 4;
    shimmer.baseVelocity = 50;
    shimmer.probability = 0.7f;
    shimmer.ghostFloor = 20;
    shimmer.velocitySpread = 0.15f;

    s.macros.complexity = 0.3f;
    s.macros.density = 0.4f;
    return s;
}

GrooveState makePocketGroove() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 71;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {4, 16};
    kick.hitCount = 3;
    kick.baseVelocity = 110;
    kick.probability = 1.0f;
    kick.noteDuration = 0.3f;
    kick.timingOffsetMs = 3.0f;

    auto& snare = s.lanes[1];
    snare.id = 1;
    snare.role = Role::Backbeat;
    snare.midiNote = 38;
    snare.cycle = {4, 4};
    snare.hitCount = 2;
    snare.baseVelocity = 100;
    snare.probability = 1.0f;
    snare.timingOffsetMs = -2.0f;
    snare.mutationRate = 0.1f;

    auto& hh = s.lanes[2];
    hh.id = 2;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {8, 8};
    hh.hitCount = 6;
    hh.baseVelocity = 70;
    hh.probability = 0.9f;
    hh.velocitySpread = 0.15f;
    hh.timingOffsetMs = 1.0f;
    hh.mutationRate = 0.15f;
    hh.swingAmount = 0.1f;

    auto& ghost = s.lanes[3];
    ghost.id = 3;
    ghost.role = Role::Ghost;
    ghost.midiNote = 39;
    ghost.cycle = {5, 16};
    ghost.hitCount = 3;
    ghost.baseVelocity = 45;
    ghost.probability = 0.65f;
    ghost.ghostFloor = 20;
    ghost.velocitySpread = 0.2f;
    ghost.timingOffsetMs = -1.5f;
    ghost.mutationRate = 0.2f;

    s.macros.humanize = 0.25f;
    s.macros.syncopation = 0.3f;
    s.macros.density = 0.4f;
    return s;
}

GrooveState makeAfrobeat12_8() {
    GrooveState s{};
    s.activeLaneCount = 5;
    s.seed = 88;

    auto& bell = s.lanes[0];
    bell.id = 0;
    bell.role = Role::AnchorPulse;
    bell.midiNote = 56;
    bell.cycle = {12, 8};
    bell.hitCount = 7;
    bell.baseVelocity = 90;
    bell.probability = 1.0f;
    bell.noteDuration = 0.1f;
    bell.timeline = true;
    bell.fixedPatternLength = 12;
    bell.fixedPattern = {true, false, true, false, true, true, false, true, false, true, false, true};

    auto& kick = s.lanes[1];
    kick.id = 1;
    kick.role = Role::Backbeat;
    kick.midiNote = 36;
    kick.cycle = {4, 4};
    kick.hitCount = 4;
    kick.baseVelocity = 112;
    kick.probability = 1.0f;
    kick.noteDuration = 0.25f;

    auto& snare = s.lanes[2];
    snare.id = 2;
    snare.role = Role::Accent;
    snare.midiNote = 38;
    snare.cycle = {3, 8};
    snare.hitCount = 2;
    snare.baseVelocity = 95;
    snare.probability = 0.9f;

    auto& shaker = s.lanes[3];
    shaker.id = 3;
    shaker.role = Role::Shimmer;
    shaker.midiNote = 70;
    shaker.cycle = {12, 12};
    shaker.hitCount = 12;
    shaker.baseVelocity = 55;
    shaker.probability = 0.9f;
    shaker.velocitySpread = 0.12f;

    auto& conga = s.lanes[4];
    conga.id = 4;
    conga.role = Role::Ghost;
    conga.midiNote = 63;
    conga.cycle = {5, 8};
    conga.hitCount = 3;
    conga.baseVelocity = 65;
    conga.probability = 0.8f;
    conga.ghostFloor = 25;
    conga.humanizeMs = 2.5f;

    s.macros.swing = 0.1f;
    s.macros.humanize = 0.15f;
    s.macros.density = 0.5f;
    return s;
}

GrooveState makeBalkanAksak() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 33;

    auto& davul = s.lanes[0];
    davul.id = 0;
    davul.role = Role::AnchorPulse;
    davul.midiNote = 36;
    davul.cycle = {3, 8};
    davul.hitCount = 2;
    davul.baseVelocity = 110;
    davul.probability = 1.0f;
    davul.noteDuration = 0.2f;
    davul.cellCount = 3;
    davul.cellSizes = {2, 2, 3};

    auto& rim = s.lanes[1];
    rim.id = 1;
    rim.role = Role::Backbeat;
    rim.midiNote = 37;
    rim.cycle = {3, 8};
    rim.hitCount = 1;
    rim.baseVelocity = 95;
    rim.probability = 1.0f;
    rim.cellCount = 3;
    rim.cellSizes = {2, 2, 3};

    auto& zurna = s.lanes[2];
    zurna.id = 2;
    zurna.role = Role::Ornament;
    zurna.midiNote = 76;
    zurna.cycle = {7, 8};
    zurna.hitCount = 4;
    zurna.baseVelocity = 80;
    zurna.probability = 0.85f;
    zurna.velocitySpread = 0.1f;
    zurna.humanizeMs = 1.5f;

    auto& darbuka = s.lanes[3];
    darbuka.id = 3;
    darbuka.role = Role::Ghost;
    darbuka.midiNote = 43;
    darbuka.cycle = {7, 8};
    darbuka.hitCount = 3;
    darbuka.baseVelocity = 60;
    darbuka.probability = 0.75f;
    darbuka.ghostFloor = 20;
    darbuka.velocitySpread = 0.15f;

    s.macros.complexity = 0.4f;
    s.macros.density = 0.45f;
    return s;
}

GrooveState makeBossaNova() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 17;

    auto& surdo = s.lanes[0];
    surdo.id = 0;
    surdo.role = Role::AnchorPulse;
    surdo.midiNote = 36;
    surdo.cycle = {4, 4};
    surdo.hitCount = 2;
    surdo.baseVelocity = 100;
    surdo.probability = 1.0f;
    surdo.noteDuration = 0.4f;
    surdo.microTimingMs[0] = 0.0f;
    surdo.microTimingMs[1] = 4.0f;
    surdo.microTimingMs[2] = 0.0f;
    surdo.microTimingMs[3] = 3.0f;

    auto& tamborim = s.lanes[1];
    tamborim.id = 1;
    tamborim.role = Role::Accent;
    tamborim.midiNote = 76;
    tamborim.cycle = {16, 16};
    tamborim.hitCount = 5;
    tamborim.baseVelocity = 85;
    tamborim.probability = 1.0f;
    tamborim.noteDuration = 0.08f;
    tamborim.timeline = true;
    tamborim.fixedPatternLength = 16;
    tamborim.fixedPattern = {true,  false, false, true,  false, false, true,  false,
                             false, false, true,  false, true,  false, false, false};

    auto& agogo = s.lanes[2];
    agogo.id = 2;
    agogo.role = Role::Ornament;
    agogo.midiNote = 67;
    agogo.cycle = {8, 8};
    agogo.hitCount = 5;
    agogo.baseVelocity = 70;
    agogo.probability = 0.85f;
    agogo.velocitySpread = 0.08f;
    agogo.microTimingMs[0] = 0.0f;
    agogo.microTimingMs[1] = 3.0f;
    agogo.microTimingMs[2] = -2.0f;
    agogo.microTimingMs[3] = 4.0f;
    agogo.microTimingMs[4] = 0.0f;
    agogo.microTimingMs[5] = 3.0f;
    agogo.microTimingMs[6] = -1.0f;
    agogo.microTimingMs[7] = 2.0f;

    auto& pandeiro = s.lanes[3];
    pandeiro.id = 3;
    pandeiro.role = Role::Shimmer;
    pandeiro.midiNote = 70;
    pandeiro.cycle = {16, 16};
    pandeiro.hitCount = 12;
    pandeiro.baseVelocity = 50;
    pandeiro.probability = 0.9f;
    pandeiro.velocitySpread = 0.15f;
    pandeiro.microTimingMs[0] = 0.0f;
    pandeiro.microTimingMs[1] = 2.0f;
    pandeiro.microTimingMs[2] = -1.0f;
    pandeiro.microTimingMs[3] = 3.0f;
    pandeiro.microTimingMs[4] = 0.0f;
    pandeiro.microTimingMs[5] = 2.0f;
    pandeiro.microTimingMs[6] = -1.0f;
    pandeiro.microTimingMs[7] = 3.0f;
    pandeiro.microTimingMs[8] = 0.0f;
    pandeiro.microTimingMs[9] = 2.0f;
    pandeiro.microTimingMs[10] = -1.0f;
    pandeiro.microTimingMs[11] = 3.0f;
    pandeiro.microTimingMs[12] = 0.0f;
    pandeiro.microTimingMs[13] = 2.0f;
    pandeiro.microTimingMs[14] = -1.0f;
    pandeiro.microTimingMs[15] = 3.0f;

    s.macros.swing = 0.2f;
    s.macros.humanize = 0.2f;
    s.macros.complexity = 0.35f;
    return s;
}

GrooveState makeCarnaticTala() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 44;

    auto& mridBass = s.lanes[0];
    mridBass.id = 0;
    mridBass.role = Role::AnchorPulse;
    mridBass.midiNote = 36;
    mridBass.cycle = {3, 4};
    mridBass.hitCount = 2;
    mridBass.baseVelocity = 105;
    mridBass.probability = 1.0f;
    mridBass.noteDuration = 0.3f;
    mridBass.cellCount = 3;
    mridBass.cellSizes = {4, 2, 2};

    auto& mridTreble = s.lanes[1];
    mridTreble.id = 1;
    mridTreble.role = Role::Accent;
    mridTreble.midiNote = 43;
    mridTreble.cycle = {3, 4};
    mridTreble.hitCount = 3;
    mridTreble.baseVelocity = 90;
    mridTreble.probability = 0.95f;
    mridTreble.cellCount = 3;
    mridTreble.cellSizes = {4, 2, 2};

    auto& ghatam = s.lanes[2];
    ghatam.id = 2;
    ghatam.role = Role::Shimmer;
    ghatam.midiNote = 42;
    ghatam.cycle = {8, 8};
    ghatam.hitCount = 5;
    ghatam.baseVelocity = 65;
    ghatam.probability = 0.85f;
    ghatam.velocitySpread = 0.1f;

    auto& kanjira = s.lanes[3];
    kanjira.id = 3;
    kanjira.role = Role::Ghost;
    kanjira.midiNote = 39;
    kanjira.cycle = {5, 8};
    kanjira.hitCount = 3;
    kanjira.baseVelocity = 50;
    kanjira.probability = 0.7f;
    kanjira.ghostFloor = 20;
    kanjira.velocitySpread = 0.12f;
    kanjira.humanizeMs = 2.0f;

    s.macros.complexity = 0.5f;
    s.macros.density = 0.4f;
    return s;
}

GrooveState makeIDMGlitch() {
    GrooveState s{};
    s.activeLaneCount = 5;
    s.seed = 99;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {4, 16};
    kick.hitCount = 3;
    kick.baseVelocity = 115;
    kick.probability = 0.9f;
    kick.noteDuration = 0.15f;
    kick.mutationRate = 0.25f;
    kick.cellCount = 4;
    kick.cellSizes = {3, 2, 5, 3};

    auto& snare = s.lanes[1];
    snare.id = 1;
    snare.role = Role::Backbeat;
    snare.midiNote = 38;
    snare.cycle = {5, 16};
    snare.hitCount = 2;
    snare.baseVelocity = 100;
    snare.probability = 0.75f;
    snare.mutationRate = 0.3f;
    snare.velocitySpread = 0.2f;

    auto& hh = s.lanes[2];
    hh.id = 2;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {9, 16};
    hh.hitCount = 6;
    hh.baseVelocity = 70;
    hh.probability = 0.85f;
    hh.velocitySpread = 0.18f;
    hh.microTimingMs[0] = 0.0f;
    hh.microTimingMs[1] = -5.0f;
    hh.microTimingMs[2] = 8.0f;
    hh.microTimingMs[3] = -3.0f;
    hh.microTimingMs[4] = 6.0f;
    hh.microTimingMs[5] = -7.0f;
    hh.microTimingMs[6] = 4.0f;
    hh.microTimingMs[7] = -2.0f;
    hh.microTimingMs[8] = 10.0f;

    auto& perc = s.lanes[3];
    perc.id = 3;
    perc.role = Role::Ghost;
    perc.midiNote = 45;
    perc.cycle = {7, 16};
    perc.hitCount = 3;
    perc.baseVelocity = 55;
    perc.probability = 0.6f;
    perc.ghostFloor = 15;
    perc.mutationRate = 0.35f;
    perc.velocitySpread = 0.25f;

    auto& glitch = s.lanes[4];
    glitch.id = 4;
    glitch.role = Role::Ornament;
    glitch.midiNote = 56;
    glitch.cycle = {11, 16};
    glitch.hitCount = 4;
    glitch.baseVelocity = 75;
    glitch.probability = 0.7f;
    glitch.noteDuration = 0.05f;
    glitch.mutationRate = 0.4f;
    glitch.microTimingMs[0] = 5.0f;
    glitch.microTimingMs[1] = -8.0f;
    glitch.microTimingMs[2] = 12.0f;
    glitch.microTimingMs[3] = -4.0f;
    glitch.microTimingMs[4] = 0.0f;
    glitch.microTimingMs[5] = 9.0f;
    glitch.microTimingMs[6] = -6.0f;
    glitch.microTimingMs[7] = 3.0f;
    glitch.microTimingMs[8] = -10.0f;
    glitch.microTimingMs[9] = 7.0f;
    glitch.microTimingMs[10] = -3.0f;

    s.macros.complexity = 0.8f;
    s.macros.tension = 0.6f;
    s.macros.syncopation = 0.5f;
    s.macros.density = 0.35f;
    return s;
}

GrooveState makeEweAgbekor() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 77;

    auto& bell = s.lanes[0];
    bell.id = 0;
    bell.role = Role::AnchorPulse;
    bell.midiNote = 56;
    bell.cycle = {12, 8};
    bell.hitCount = 7;
    bell.baseVelocity = 95;
    bell.probability = 1.0f;
    bell.noteDuration = 0.1f;
    bell.timeline = true;
    bell.fixedPatternLength = 12;
    bell.fixedPattern = {true, false, true, false, true, true, false, true, false, true, false, true};

    auto& kidi = s.lanes[1];
    kidi.id = 1;
    kidi.role = Role::Accent;
    kidi.midiNote = 63;
    kidi.cycle = {5, 12};
    kidi.hitCount = 5;
    kidi.baseVelocity = 85;
    kidi.probability = 0.95f;
    kidi.velocitySpread = 0.08f;
    kidi.humanizeMs = 1.5f;

    auto& sogo = s.lanes[2];
    sogo.id = 2;
    sogo.role = Role::Ghost;
    sogo.midiNote = 43;
    sogo.cycle = {3, 12};
    sogo.hitCount = 3;
    sogo.baseVelocity = 75;
    sogo.probability = 0.9f;
    sogo.ghostFloor = 30;
    sogo.velocitySpread = 0.1f;
    sogo.humanizeMs = 2.0f;

    auto& lead = s.lanes[3];
    lead.id = 3;
    lead.role = Role::Ornament;
    lead.midiNote = 38;
    lead.cycle = {7, 12};
    lead.hitCount = 4;
    lead.baseVelocity = 70;
    lead.probability = 0.8f;
    lead.velocitySpread = 0.12f;
    lead.humanizeMs = 2.5f;

    s.macros.humanize = 0.15f;
    s.macros.density = 0.5f;
    s.macros.complexity = 0.4f;
    return s;
}

GrooveState makeGamelanColotomic() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 63;

    auto& ketuk = s.lanes[0];
    ketuk.id = 0;
    ketuk.role = Role::Shimmer;
    ketuk.midiNote = 76;
    ketuk.cycle = {4, 4};
    ketuk.hitCount = 1;
    ketuk.baseVelocity = 70;
    ketuk.probability = 1.0f;
    ketuk.noteDuration = 0.15f;

    auto& kempul = s.lanes[1];
    kempul.id = 1;
    kempul.role = Role::Accent;
    kempul.midiNote = 67;
    kempul.cycle = {8, 4};
    kempul.hitCount = 1;
    kempul.baseVelocity = 80;
    kempul.probability = 1.0f;
    kempul.noteDuration = 0.3f;

    auto& kenong = s.lanes[2];
    kenong.id = 2;
    kenong.role = Role::Backbeat;
    kenong.midiNote = 56;
    kenong.cycle = {16, 4};
    kenong.hitCount = 1;
    kenong.baseVelocity = 90;
    kenong.probability = 1.0f;
    kenong.noteDuration = 0.5f;

    auto& gong = s.lanes[3];
    gong.id = 3;
    gong.role = Role::AnchorPulse;
    gong.midiNote = 36;
    gong.cycle = {32, 4};
    gong.hitCount = 1;
    gong.baseVelocity = 110;
    gong.probability = 1.0f;
    gong.noteDuration = 0.8f;

    s.macros.density = 0.3f;
    s.macros.complexity = 0.2f;
    return s;
}

GrooveState makePolymetricFoundation() {
    GrooveState s{};
    s.activeLaneCount = 2;
    s.seed = 101;

    auto& bell = s.lanes[0];
    bell.id = 0;
    bell.role = Role::AnchorPulse;
    bell.midiNote = 56;
    bell.cycle = {12, 8};
    bell.hitCount = 7;
    bell.baseVelocity = 90;
    bell.probability = 1.0f;
    bell.noteDuration = 0.1f;

    auto& counter = s.lanes[1];
    counter.id = 1;
    counter.role = Role::Accent;
    counter.midiNote = 42;
    counter.cycle = {7, 8};
    counter.hitCount = 5;
    counter.baseVelocity = 80;
    counter.probability = 1.0f;

    return s;
}

GrooveState makeEweEnsemble() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 102;

    auto& bell = s.lanes[0];
    bell.id = 0;
    bell.role = Role::AnchorPulse;
    bell.midiNote = 56;
    bell.cycle = {12, 8};
    bell.hitCount = 7;
    bell.baseVelocity = 110;
    bell.probability = 1.0f;
    bell.noteDuration = 0.1f;

    auto& support = s.lanes[1];
    support.id = 1;
    support.role = Role::Accent;
    support.midiNote = 43;
    support.cycle = {12, 8};
    support.hitCount = 3;
    support.baseVelocity = 90;
    support.probability = 1.0f;
    support.ghostFloor = 40;

    auto& respond = s.lanes[2];
    respond.id = 2;
    respond.role = Role::Ghost;
    respond.midiNote = 45;
    respond.cycle = {5, 8};
    respond.hitCount = 3;
    respond.baseVelocity = 85;
    respond.probability = 0.9f;
    respond.ghostFloor = 50;

    auto& lead = s.lanes[3];
    lead.id = 3;
    lead.role = Role::Ornament;
    lead.midiNote = 47;
    lead.cycle = {7, 8};
    lead.hitCount = 5;
    lead.rotation = 2;
    lead.baseVelocity = 75;
    lead.probability = 0.85f;
    lead.ghostFloor = 60;
    lead.kotekanSourceLane = 0;

    s.macros.humanize = 0.15f;
    return s;
}

GrooveState makeMandingDjembe() {
    GrooveState s{};
    s.activeLaneCount = 3;
    s.seed = 103;

    auto& dunun = s.lanes[0];
    dunun.id = 0;
    dunun.role = Role::AnchorPulse;
    dunun.midiNote = 36;
    dunun.cycle = {8, 8};
    dunun.hitCount = 3;
    dunun.baseVelocity = 100;
    dunun.probability = 1.0f;

    auto& sangban = s.lanes[1];
    sangban.id = 1;
    sangban.role = Role::Accent;
    sangban.midiNote = 43;
    sangban.cycle = {8, 8};
    sangban.hitCount = 5;
    sangban.rotation = 1;
    sangban.baseVelocity = 85;
    sangban.probability = 0.95f;
    sangban.ghostFloor = 45;

    auto& djembe = s.lanes[2];
    djembe.id = 2;
    djembe.role = Role::Shimmer;
    djembe.midiNote = 50;
    djembe.cycle = {8, 8};
    djembe.hitCount = 7;
    djembe.baseVelocity = 95;
    djembe.probability = 0.9f;
    djembe.ghostFloor = 55;

    s.macros.humanize = 0.15f;
    return s;
}

GrooveState makeCubanSon() {
    GrooveState s{};
    s.activeLaneCount = 5;
    s.seed = 104;

    auto& clave = s.lanes[0];
    clave.id = 0;
    clave.role = Role::AnchorPulse;
    clave.midiNote = 75;
    clave.cycle = {16, 16};
    clave.hitCount = 5;
    clave.baseVelocity = 100;
    clave.probability = 1.0f;
    clave.swingAmount = 0.25f;

    auto& cascara = s.lanes[1];
    cascara.id = 1;
    cascara.role = Role::Accent;
    cascara.midiNote = 37;
    cascara.cycle = {8, 8};
    cascara.hitCount = 5;
    cascara.rotation = 1;
    cascara.baseVelocity = 80;
    cascara.probability = 0.95f;
    cascara.ghostFloor = 45;
    cascara.swingAmount = 0.25f;

    auto& tumbao = s.lanes[2];
    tumbao.id = 2;
    tumbao.role = Role::Backbeat;
    tumbao.midiNote = 36;
    tumbao.cycle = {8, 8};
    tumbao.hitCount = 3;
    tumbao.baseVelocity = 95;
    tumbao.probability = 1.0f;
    tumbao.swingAmount = 0.20f;

    auto& conga = s.lanes[3];
    conga.id = 3;
    conga.role = Role::Ghost;
    conga.midiNote = 63;
    conga.cycle = {16, 16};
    conga.hitCount = 7;
    conga.rotation = 2;
    conga.baseVelocity = 85;
    conga.probability = 0.9f;
    conga.ghostFloor = 55;
    conga.swingAmount = 0.30f;

    auto& shaker = s.lanes[4];
    shaker.id = 4;
    shaker.role = Role::Shimmer;
    shaker.midiNote = 70;
    shaker.cycle = {8, 8};
    shaker.hitCount = 7;
    shaker.baseVelocity = 50;
    shaker.probability = 0.85f;
    shaker.ghostFloor = 30;
    shaker.swingAmount = 0.20f;

    s.macros.swing = 0.25f;
    s.macros.syncopation = 0.4f;
    return s;
}

GrooveState makeAfrobeatLagos() {
    GrooveState s{};
    s.activeLaneCount = 6;
    s.seed = 105;

    auto& bell = s.lanes[0];
    bell.id = 0;
    bell.role = Role::AnchorPulse;
    bell.midiNote = 56;
    bell.cycle = {12, 8};
    bell.hitCount = 7;
    bell.baseVelocity = 105;
    bell.probability = 1.0f;
    bell.noteDuration = 0.1f;
    bell.timeline = true;
    bell.fixedPatternLength = 12;
    bell.fixedPattern = {true, false, true, false, true, true, false, true, false, true, false, true};

    auto& hh = s.lanes[1];
    hh.id = 1;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {16, 16};
    hh.hitCount = 13;
    hh.baseVelocity = 70;
    hh.probability = 0.9f;
    hh.ghostFloor = 50;
    hh.mutationRate = 0.05f;

    auto& kick = s.lanes[2];
    kick.id = 2;
    kick.role = Role::Backbeat;
    kick.midiNote = 36;
    kick.cycle = {16, 16};
    kick.hitCount = 3;
    kick.baseVelocity = 110;
    kick.probability = 1.0f;
    kick.noteDuration = 0.25f;

    auto& snare = s.lanes[3];
    snare.id = 3;
    snare.role = Role::Accent;
    snare.midiNote = 38;
    snare.cycle = {16, 16};
    snare.hitCount = 5;
    snare.rotation = 3;
    snare.baseVelocity = 90;
    snare.probability = 0.9f;
    snare.ghostFloor = 40;
    snare.phraseLength = 8.0f;
    snare.phraseGap = 4.0f;
    snare.mutationRate = 0.20f;

    auto& shakerL = s.lanes[4];
    shakerL.id = 4;
    shakerL.role = Role::Ornament;
    shakerL.midiNote = 70;
    shakerL.cycle = {12, 8};
    shakerL.hitCount = 9;
    shakerL.rotation = 2;
    shakerL.baseVelocity = 55;
    shakerL.probability = 0.85f;
    shakerL.ghostFloor = 35;
    shakerL.phraseLength = 12.0f;
    shakerL.phraseGap = 4.0f;
    shakerL.phraseOffset = 4.0f;
    shakerL.mutationRate = 0.10f;

    auto& conga = s.lanes[5];
    conga.id = 5;
    conga.role = Role::Ghost;
    conga.midiNote = 63;
    conga.cycle = {8, 8};
    conga.hitCount = 3;
    conga.rotation = 1;
    conga.baseVelocity = 80;
    conga.probability = 0.8f;
    conga.ghostFloor = 50;
    conga.phraseLength = 6.0f;
    conga.phraseGap = 6.0f;
    conga.phraseOffset = 8.0f;
    conga.mutationRate = 0.25f;

    s.macros.density = 0.5f;
    s.macros.humanize = 0.15f;
    return s;
}

GrooveState makeBaliKotekan() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 106;

    auto& polos = s.lanes[0];
    polos.id = 0;
    polos.role = Role::AnchorPulse;
    polos.midiNote = 72;
    polos.cycle = {8, 16};
    polos.hitCount = 5;
    polos.baseVelocity = 90;
    polos.probability = 1.0f;
    polos.noteDuration = 0.12f;
    polos.ghostFloor = 40;

    auto& sangsih = s.lanes[1];
    sangsih.id = 1;
    sangsih.role = Role::Accent;
    sangsih.midiNote = 74;
    sangsih.cycle = {8, 16};
    sangsih.hitCount = 5;
    sangsih.baseVelocity = 85;
    sangsih.probability = 0.95f;
    sangsih.noteDuration = 0.12f;
    sangsih.ghostFloor = 40;
    sangsih.kotekanSourceLane = 0;

    auto& jegogan = s.lanes[2];
    jegogan.id = 2;
    jegogan.role = Role::Backbeat;
    jegogan.midiNote = 48;
    jegogan.cycle = {8, 4};
    jegogan.hitCount = 2;
    jegogan.baseVelocity = 100;
    jegogan.probability = 1.0f;
    jegogan.noteDuration = 0.5f;

    auto& reyong = s.lanes[3];
    reyong.id = 3;
    reyong.role = Role::Ghost;
    reyong.midiNote = 67;
    reyong.cycle = {16, 16};
    reyong.hitCount = 5;
    reyong.rotation = 3;
    reyong.baseVelocity = 75;
    reyong.probability = 0.85f;
    reyong.ghostFloor = 50;

    s.macros.complexity = 0.3f;
    return s;
}

GrooveState makeJavaColotomic() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 107;

    auto& ketuk = s.lanes[0];
    ketuk.id = 0;
    ketuk.role = Role::Shimmer;
    ketuk.midiNote = 76;
    ketuk.cycle = {4, 4};
    ketuk.hitCount = 1;
    ketuk.baseVelocity = 70;
    ketuk.probability = 1.0f;
    ketuk.noteDuration = 0.15f;

    auto& kempul = s.lanes[1];
    kempul.id = 1;
    kempul.role = Role::Accent;
    kempul.midiNote = 60;
    kempul.cycle = {8, 4};
    kempul.hitCount = 1;
    kempul.baseVelocity = 85;
    kempul.probability = 1.0f;
    kempul.noteDuration = 0.3f;

    auto& kenong = s.lanes[2];
    kenong.id = 2;
    kenong.role = Role::Backbeat;
    kenong.midiNote = 55;
    kenong.cycle = {16, 4};
    kenong.hitCount = 1;
    kenong.baseVelocity = 95;
    kenong.probability = 1.0f;
    kenong.noteDuration = 0.5f;

    auto& gong = s.lanes[3];
    gong.id = 3;
    gong.role = Role::AnchorPulse;
    gong.midiNote = 48;
    gong.cycle = {32, 4};
    gong.hitCount = 1;
    gong.baseVelocity = 110;
    gong.probability = 1.0f;
    gong.noteDuration = 0.8f;

    s.macros.density = 0.3f;
    s.macros.complexity = 0.2f;
    return s;
}

GrooveState makeTintal() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 108;

    auto& sam = s.lanes[0];
    sam.id = 0;
    sam.role = Role::AnchorPulse;
    sam.midiNote = 36;
    sam.cycle = {16, 4};
    sam.hitCount = 4;
    sam.baseVelocity = 110;
    sam.probability = 1.0f;
    sam.noteDuration = 0.3f;

    auto& theka = s.lanes[1];
    theka.id = 1;
    theka.role = Role::Accent;
    theka.midiNote = 38;
    theka.cycle = {16, 8};
    theka.hitCount = 7;
    theka.baseVelocity = 90;
    theka.probability = 0.95f;

    auto& dugun = s.lanes[2];
    dugun.id = 2;
    dugun.role = Role::Shimmer;
    dugun.midiNote = 42;
    dugun.cycle = {16, 8};
    dugun.hitCount = 9;
    dugun.rotation = 2;
    dugun.baseVelocity = 70;
    dugun.probability = 0.9f;

    auto& tigun = s.lanes[3];
    tigun.id = 3;
    tigun.role = Role::Ghost;
    tigun.midiNote = 46;
    tigun.cycle = {16, 16};
    tigun.hitCount = 5;
    tigun.rotation = 4;
    tigun.baseVelocity = 55;
    tigun.probability = 0.8f;

    s.macros.complexity = 0.4f;
    return s;
}

GrooveState makeRupakTal() {
    GrooveState s{};
    s.activeLaneCount = 3;
    s.seed = 109;

    auto& sam = s.lanes[0];
    sam.id = 0;
    sam.role = Role::AnchorPulse;
    sam.midiNote = 36;
    sam.cycle = {7, 4};
    sam.hitCount = 3;
    sam.baseVelocity = 120;
    sam.probability = 1.0f;
    sam.ghostFloor = 50;

    auto& theka = s.lanes[1];
    theka.id = 1;
    theka.role = Role::Accent;
    theka.midiNote = 38;
    theka.cycle = {7, 8};
    theka.hitCount = 4;
    theka.rotation = 1;
    theka.baseVelocity = 85;
    theka.probability = 0.9f;
    theka.ghostFloor = 40;

    auto& counter = s.lanes[2];
    counter.id = 2;
    counter.role = Role::Shimmer;
    counter.midiNote = 42;
    counter.cycle = {7, 8};
    counter.hitCount = 5;
    counter.rotation = 3;
    counter.baseVelocity = 70;
    counter.probability = 0.85f;
    counter.ghostFloor = 35;

    s.macros.complexity = 0.4f;
    return s;
}

GrooveState makeRachenitsa() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 110;

    auto& tupanBass = s.lanes[0];
    tupanBass.id = 0;
    tupanBass.role = Role::AnchorPulse;
    tupanBass.midiNote = 36;
    tupanBass.cycle = {7, 8};
    tupanBass.hitCount = 3;
    tupanBass.baseVelocity = 110;
    tupanBass.probability = 1.0f;
    tupanBass.noteDuration = 0.2f;

    auto& tupanRim = s.lanes[1];
    tupanRim.id = 1;
    tupanRim.role = Role::Backbeat;
    tupanRim.midiNote = 37;
    tupanRim.cycle = {7, 8};
    tupanRim.hitCount = 4;
    tupanRim.rotation = 2;
    tupanRim.baseVelocity = 85;
    tupanRim.probability = 1.0f;

    auto& kaval = s.lanes[2];
    kaval.id = 2;
    kaval.role = Role::Ornament;
    kaval.midiNote = 76;
    kaval.cycle = {7, 8};
    kaval.hitCount = 2;
    kaval.rotation = 1;
    kaval.baseVelocity = 75;
    kaval.probability = 0.9f;

    auto& gadulka = s.lanes[3];
    gadulka.id = 3;
    gadulka.role = Role::Shimmer;
    gadulka.midiNote = 42;
    gadulka.cycle = {7, 16};
    gadulka.hitCount = 5;
    gadulka.baseVelocity = 65;
    gadulka.probability = 0.85f;

    s.macros.complexity = 0.4f;
    s.macros.density = 0.45f;
    return s;
}

GrooveState makeKopanitsa() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 111;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {11, 8};
    kick.hitCount = 4;
    kick.baseVelocity = 115;
    kick.probability = 1.0f;
    kick.humanizeMs = 2.0f;

    auto& snare = s.lanes[1];
    snare.id = 1;
    snare.role = Role::Backbeat;
    snare.midiNote = 38;
    snare.cycle = {11, 8};
    snare.hitCount = 5;
    snare.rotation = 3;
    snare.baseVelocity = 90;
    snare.probability = 0.95f;
    snare.humanizeMs = 3.0f;

    auto& hh = s.lanes[2];
    hh.id = 2;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {11, 16};
    hh.hitCount = 7;
    hh.baseVelocity = 70;
    hh.probability = 0.9f;
    hh.humanizeMs = 2.0f;

    auto& bellAccent = s.lanes[3];
    bellAccent.id = 3;
    bellAccent.role = Role::Ornament;
    bellAccent.midiNote = 56;
    bellAccent.cycle = {11, 8};
    bellAccent.hitCount = 3;
    bellAccent.rotation = 2;
    bellAccent.baseVelocity = 80;
    bellAccent.probability = 0.9f;
    bellAccent.humanizeMs = 2.0f;

    s.macros.complexity = 0.4f;
    return s;
}

GrooveState makeReichProcess() {
    GrooveState s{};
    s.activeLaneCount = 3;
    s.seed = 112;

    auto& fixed = s.lanes[0];
    fixed.id = 0;
    fixed.role = Role::AnchorPulse;
    fixed.midiNote = 76;
    fixed.cycle = {12, 8};
    fixed.hitCount = 5;
    fixed.baseVelocity = 90;
    fixed.probability = 1.0f;
    fixed.noteDuration = 0.15f;

    auto& drifting = s.lanes[1];
    drifting.id = 1;
    drifting.role = Role::AnchorPulse;
    drifting.midiNote = 76;
    drifting.cycle = {12, 8};
    drifting.hitCount = 5;
    drifting.baseVelocity = 85;
    drifting.probability = 1.0f;
    drifting.noteDuration = 0.15f;
    drifting.driftRate = 0.25f;

    auto& anchor = s.lanes[2];
    anchor.id = 2;
    anchor.role = Role::Shimmer;
    anchor.midiNote = 42;
    anchor.cycle = {4, 4};
    anchor.hitCount = 4;
    anchor.baseVelocity = 70;
    anchor.probability = 1.0f;

    s.macros.complexity = 0.2f;
    return s;
}

GrooveState makeRileyLayers() {
    GrooveState s{};
    s.activeLaneCount = 5;
    s.seed = 113;

    auto& foundation = s.lanes[0];
    foundation.id = 0;
    foundation.role = Role::AnchorPulse;
    foundation.midiNote = 36;
    foundation.cycle = {8, 8};
    foundation.hitCount = 5;
    foundation.baseVelocity = 95;
    foundation.probability = 1.0f;

    auto& voiceA = s.lanes[1];
    voiceA.id = 1;
    voiceA.role = Role::Accent;
    voiceA.midiNote = 42;
    voiceA.cycle = {12, 8};
    voiceA.hitCount = 7;
    voiceA.baseVelocity = 80;
    voiceA.probability = 0.9f;
    voiceA.phraseLength = 16.0f;
    voiceA.phraseGap = 8.0f;
    voiceA.phraseOffset = 4.0f;

    auto& voiceB = s.lanes[2];
    voiceB.id = 2;
    voiceB.role = Role::Shimmer;
    voiceB.midiNote = 56;
    voiceB.cycle = {10, 8};
    voiceB.hitCount = 6;
    voiceB.baseVelocity = 75;
    voiceB.probability = 0.85f;
    voiceB.phraseLength = 12.0f;
    voiceB.phraseGap = 12.0f;
    voiceB.phraseOffset = 12.0f;

    auto& voiceC = s.lanes[3];
    voiceC.id = 3;
    voiceC.role = Role::Ghost;
    voiceC.midiNote = 60;
    voiceC.cycle = {7, 8};
    voiceC.hitCount = 4;
    voiceC.baseVelocity = 70;
    voiceC.probability = 0.8f;
    voiceC.phraseLength = 8.0f;
    voiceC.phraseGap = 16.0f;
    voiceC.phraseOffset = 20.0f;

    auto& voiceD = s.lanes[4];
    voiceD.id = 4;
    voiceD.role = Role::Ornament;
    voiceD.midiNote = 76;
    voiceD.cycle = {9, 4};
    voiceD.hitCount = 3;
    voiceD.baseVelocity = 65;
    voiceD.probability = 0.75f;
    voiceD.phraseLength = 24.0f;
    voiceD.phraseGap = 8.0f;
    voiceD.phraseOffset = 32.0f;

    s.macros.complexity = 0.3f;
    return s;
}

GrooveState makeNancarrowTempi() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 114;

    auto& anchor = s.lanes[0];
    anchor.id = 0;
    anchor.role = Role::AnchorPulse;
    anchor.midiNote = 36;
    anchor.cycle = {4, 4};
    anchor.hitCount = 4;
    anchor.baseVelocity = 100;
    anchor.probability = 1.0f;
    anchor.tempoMultiplier = 1.0f;

    auto& doubleTime = s.lanes[1];
    doubleTime.id = 1;
    doubleTime.role = Role::Shimmer;
    doubleTime.midiNote = 42;
    doubleTime.cycle = {8, 8};
    doubleTime.hitCount = 5;
    doubleTime.baseVelocity = 70;
    doubleTime.probability = 0.9f;
    doubleTime.tempoMultiplier = 2.0f;

    auto& halfTime = s.lanes[2];
    halfTime.id = 2;
    halfTime.role = Role::Accent;
    halfTime.midiNote = 56;
    halfTime.cycle = {3, 4};
    halfTime.hitCount = 2;
    halfTime.baseVelocity = 85;
    halfTime.probability = 1.0f;
    halfTime.tempoMultiplier = 0.5f;

    auto& hemiola = s.lanes[3];
    hemiola.id = 3;
    hemiola.role = Role::Ghost;
    hemiola.midiNote = 45;
    hemiola.cycle = {6, 8};
    hemiola.hitCount = 4;
    hemiola.baseVelocity = 75;
    hemiola.probability = 0.85f;
    hemiola.tempoMultiplier = 1.5f;

    s.macros.complexity = 0.5f;
    return s;
}

GrooveState makeMinimalTechno() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 115;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {4, 4};
    kick.hitCount = 4;
    kick.baseVelocity = 120;
    kick.probability = 1.0f;
    kick.noteDuration = 0.2f;

    auto& hh = s.lanes[1];
    hh.id = 1;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {16, 16};
    hh.hitCount = 8;
    hh.baseVelocity = 75;
    hh.probability = 0.95f;

    auto& ghost = s.lanes[2];
    ghost.id = 2;
    ghost.role = Role::Ghost;
    ghost.midiNote = 39;
    ghost.cycle = {7, 8};
    ghost.hitCount = 5;
    ghost.rotation = 2;
    ghost.baseVelocity = 55;
    ghost.probability = 0.8f;

    auto& clap = s.lanes[3];
    clap.id = 3;
    clap.role = Role::Backbeat;
    clap.midiNote = 38;
    clap.cycle = {16, 16};
    clap.hitCount = 2;
    clap.rotation = 4;
    clap.baseVelocity = 100;
    clap.probability = 1.0f;

    s.macros.density = 0.4f;
    return s;
}

GrooveState makeDeepHouse() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 116;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {4, 4};
    kick.hitCount = 4;
    kick.baseVelocity = 115;
    kick.probability = 1.0f;
    kick.noteDuration = 0.25f;
    kick.humanizeMs = 2.0f;

    auto& openHat = s.lanes[1];
    openHat.id = 1;
    openHat.role = Role::Shimmer;
    openHat.midiNote = 46;
    openHat.cycle = {16, 16};
    openHat.hitCount = 6;
    openHat.rotation = 3;
    openHat.baseVelocity = 70;
    openHat.probability = 0.9f;
    openHat.swingAmount = 0.40f;
    openHat.humanizeMs = 4.0f;

    auto& shakerL = s.lanes[2];
    shakerL.id = 2;
    shakerL.role = Role::Ghost;
    shakerL.midiNote = 70;
    shakerL.cycle = {16, 16};
    shakerL.hitCount = 10;
    shakerL.baseVelocity = 50;
    shakerL.probability = 0.85f;
    shakerL.swingAmount = 0.45f;
    shakerL.humanizeMs = 5.0f;

    auto& rim = s.lanes[3];
    rim.id = 3;
    rim.role = Role::Accent;
    rim.midiNote = 37;
    rim.cycle = {8, 8};
    rim.hitCount = 3;
    rim.rotation = 1;
    rim.baseVelocity = 90;
    rim.probability = 0.9f;
    rim.swingAmount = 0.35f;
    rim.humanizeMs = 4.0f;

    s.macros.swing = 0.35f;
    s.macros.humanize = 0.1f;
    return s;
}

GrooveState makeSambaBatucada() {
    GrooveState s{};
    s.activeLaneCount = 5;
    s.seed = 117;

    auto& surdo = s.lanes[0];
    surdo.id = 0;
    surdo.role = Role::AnchorPulse;
    surdo.midiNote = 36;
    surdo.cycle = {4, 4};
    surdo.hitCount = 2;
    surdo.rotation = 1;
    surdo.baseVelocity = 110;
    surdo.probability = 1.0f;
    surdo.noteDuration = 0.4f;
    surdo.swingAmount = 0.20f;

    auto& tamborim = s.lanes[1];
    tamborim.id = 1;
    tamborim.role = Role::Accent;
    tamborim.midiNote = 50;
    tamborim.cycle = {16, 16};
    tamborim.hitCount = 7;
    tamborim.rotation = 2;
    tamborim.baseVelocity = 90;
    tamborim.probability = 0.95f;
    tamborim.ghostFloor = 50;
    tamborim.swingAmount = 0.25f;

    auto& agogo = s.lanes[2];
    agogo.id = 2;
    agogo.role = Role::Ornament;
    agogo.midiNote = 56;
    agogo.cycle = {16, 16};
    agogo.hitCount = 5;
    agogo.baseVelocity = 85;
    agogo.probability = 0.9f;
    agogo.ghostFloor = 40;
    agogo.swingAmount = 0.20f;

    auto& repinique = s.lanes[3];
    repinique.id = 3;
    repinique.role = Role::Backbeat;
    repinique.midiNote = 47;
    repinique.cycle = {8, 8};
    repinique.hitCount = 3;
    repinique.rotation = 1;
    repinique.baseVelocity = 100;
    repinique.probability = 1.0f;
    repinique.ghostFloor = 30;
    repinique.swingAmount = 0.15f;

    auto& caixa = s.lanes[4];
    caixa.id = 4;
    caixa.role = Role::Shimmer;
    caixa.midiNote = 38;
    caixa.cycle = {16, 16};
    caixa.hitCount = 13;
    caixa.baseVelocity = 70;
    caixa.probability = 0.9f;
    caixa.ghostFloor = 55;
    caixa.swingAmount = 0.25f;

    s.macros.swing = 0.2f;
    s.macros.humanize = 0.25f;
    return s;
}

GrooveState makeBossaTrio() {
    GrooveState s{};
    s.activeLaneCount = 3;
    s.seed = 118;

    auto& bass = s.lanes[0];
    bass.id = 0;
    bass.role = Role::AnchorPulse;
    bass.midiNote = 36;
    bass.cycle = {16, 16};
    bass.hitCount = 5;
    bass.baseVelocity = 95;
    bass.probability = 1.0f;
    bass.noteDuration = 0.3f;
    bass.swingAmount = 0.15f;

    auto& ride = s.lanes[1];
    ride.id = 1;
    ride.role = Role::Shimmer;
    ride.midiNote = 51;
    ride.cycle = {8, 8};
    ride.hitCount = 4;
    ride.baseVelocity = 65;
    ride.probability = 0.9f;
    ride.ghostFloor = 30;
    ride.swingAmount = 0.10f;

    auto& brush = s.lanes[2];
    brush.id = 2;
    brush.role = Role::Ghost;
    brush.midiNote = 38;
    brush.cycle = {16, 16};
    brush.hitCount = 9;
    brush.rotation = 3;
    brush.baseVelocity = 40;
    brush.probability = 0.8f;
    brush.ghostFloor = 25;
    brush.swingAmount = 0.20f;

    s.macros.swing = 0.15f;
    s.macros.humanize = 0.15f;
    return s;
}

GrooveState makeClassicFunk() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 119;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {16, 16};
    kick.hitCount = 3;
    kick.baseVelocity = 110;
    kick.probability = 1.0f;
    kick.noteDuration = 0.2f;
    kick.velocitySpread = 0.20f;
    kick.timingOffsetMs = 3.0f;

    auto& snareAccent = s.lanes[1];
    snareAccent.id = 1;
    snareAccent.role = Role::Backbeat;
    snareAccent.midiNote = 38;
    snareAccent.cycle = {8, 8};
    snareAccent.hitCount = 2;
    snareAccent.rotation = 4;
    snareAccent.baseVelocity = 105;
    snareAccent.probability = 1.0f;
    snareAccent.velocitySpread = 0.15f;
    snareAccent.timingOffsetMs = -2.0f;

    auto& snareGhost = s.lanes[2];
    snareGhost.id = 2;
    snareGhost.role = Role::Ghost;
    snareGhost.midiNote = 38;
    snareGhost.cycle = {16, 16};
    snareGhost.hitCount = 11;
    snareGhost.baseVelocity = 40;
    snareGhost.probability = 0.85f;
    snareGhost.ghostFloor = 30;
    snareGhost.velocitySpread = 0.65f;
    snareGhost.timingOffsetMs = 1.0f;

    auto& hh = s.lanes[3];
    hh.id = 3;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {16, 16};
    hh.hitCount = 14;
    hh.baseVelocity = 75;
    hh.probability = 0.9f;
    hh.ghostFloor = 45;
    hh.velocitySpread = 0.40f;

    s.macros.tension = 0.6f;
    s.macros.syncopation = 0.3f;
    return s;
}

GrooveState makeNeoSoul() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 120;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {16, 16};
    kick.hitCount = 4;
    kick.baseVelocity = 95;
    kick.probability = 1.0f;
    kick.velocitySpread = 0.30f;
    kick.timingOffsetMs = 4.0f;

    auto& snare = s.lanes[1];
    snare.id = 1;
    snare.role = Role::Backbeat;
    snare.midiNote = 38;
    snare.cycle = {8, 8};
    snare.hitCount = 2;
    snare.rotation = 4;
    snare.baseVelocity = 90;
    snare.probability = 0.95f;
    snare.ghostFloor = 35;
    snare.velocitySpread = 0.45f;
    snare.timingOffsetMs = -3.0f;

    auto& hh = s.lanes[2];
    hh.id = 2;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {16, 16};
    hh.hitCount = 10;
    hh.baseVelocity = 60;
    hh.probability = 0.85f;
    hh.ghostFloor = 40;
    hh.velocitySpread = 0.55f;
    hh.timingOffsetMs = 1.0f;

    auto& rimClick = s.lanes[3];
    rimClick.id = 3;
    rimClick.role = Role::Ornament;
    rimClick.midiNote = 37;
    rimClick.cycle = {12, 8};
    rimClick.hitCount = 5;
    rimClick.rotation = 2;
    rimClick.baseVelocity = 55;
    rimClick.probability = 0.8f;
    rimClick.ghostFloor = 30;
    rimClick.velocitySpread = 0.50f;
    rimClick.timingOffsetMs = -1.0f;

    s.macros.humanize = 0.4f;
    s.macros.tension = 0.4f;
    return s;
}

GrooveState makeJazzBop() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 121;

    auto& ride = s.lanes[0];
    ride.id = 0;
    ride.role = Role::AnchorPulse;
    ride.midiNote = 51;
    ride.cycle = {4, 4};
    ride.hitCount = 4;
    ride.baseVelocity = 85;
    ride.probability = 1.0f;
    ride.swingAmount = 0.45f;

    auto& hhFoot = s.lanes[1];
    hhFoot.id = 1;
    hhFoot.role = Role::Backbeat;
    hhFoot.midiNote = 44;
    hhFoot.cycle = {4, 4};
    hhFoot.hitCount = 2;
    hhFoot.rotation = 1;
    hhFoot.baseVelocity = 70;
    hhFoot.probability = 1.0f;
    hhFoot.swingAmount = 0.40f;

    auto& kick = s.lanes[2];
    kick.id = 2;
    kick.role = Role::Ghost;
    kick.midiNote = 36;
    kick.cycle = {8, 8};
    kick.hitCount = 3;
    kick.baseVelocity = 80;
    kick.probability = 0.9f;
    kick.ghostFloor = 30;
    kick.swingAmount = 0.35f;
    kick.mutationRate = 0.15f;

    auto& snareComp = s.lanes[3];
    snareComp.id = 3;
    snareComp.role = Role::Ornament;
    snareComp.midiNote = 38;
    snareComp.cycle = {16, 16};
    snareComp.hitCount = 5;
    snareComp.rotation = 2;
    snareComp.baseVelocity = 65;
    snareComp.probability = 0.8f;
    snareComp.ghostFloor = 40;
    snareComp.swingAmount = 0.30f;
    snareComp.mutationRate = 0.25f;

    s.macros.swing = 0.4f;
    s.macros.humanize = 0.2f;
    return s;
}

GrooveState makeElvinCascade() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 122;

    auto& ride = s.lanes[0];
    ride.id = 0;
    ride.role = Role::AnchorPulse;
    ride.midiNote = 51;
    ride.cycle = {4, 4};
    ride.hitCount = 3;
    ride.baseVelocity = 90;
    ride.probability = 1.0f;
    ride.swingAmount = 0.40f;
    ride.mutationRate = 0.10f;

    auto& snareCross = s.lanes[1];
    snareCross.id = 1;
    snareCross.role = Role::Accent;
    snareCross.midiNote = 38;
    snareCross.cycle = {3, 4};
    snareCross.hitCount = 2;
    snareCross.baseVelocity = 75;
    snareCross.probability = 0.9f;
    snareCross.ghostFloor = 45;
    snareCross.swingAmount = 0.35f;
    snareCross.mutationRate = 0.20f;

    auto& tom = s.lanes[2];
    tom.id = 2;
    tom.role = Role::Ghost;
    tom.midiNote = 45;
    tom.cycle = {5, 4};
    tom.hitCount = 3;
    tom.rotation = 1;
    tom.baseVelocity = 70;
    tom.probability = 0.85f;
    tom.ghostFloor = 40;
    tom.swingAmount = 0.30f;
    tom.mutationRate = 0.20f;

    auto& bassDrum = s.lanes[3];
    bassDrum.id = 3;
    bassDrum.role = Role::Ornament;
    bassDrum.midiNote = 36;
    bassDrum.cycle = {7, 4};
    bassDrum.hitCount = 4;
    bassDrum.rotation = 2;
    bassDrum.baseVelocity = 85;
    bassDrum.probability = 0.9f;
    bassDrum.ghostFloor = 30;
    bassDrum.swingAmount = 0.35f;
    bassDrum.mutationRate = 0.15f;

    s.macros.swing = 0.35f;
    s.macros.complexity = 0.6f;
    s.macros.humanize = 0.2f;
    return s;
}

GrooveState makeJungleBreak() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 123;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {16, 16};
    kick.hitCount = 5;
    kick.rotation = 3;
    kick.baseVelocity = 110;
    kick.probability = 1.0f;
    kick.noteDuration = 0.15f;
    kick.velocitySpread = 0.25f;
    kick.mutationRate = 0.15f;

    auto& snare = s.lanes[1];
    snare.id = 1;
    snare.role = Role::Backbeat;
    snare.midiNote = 38;
    snare.cycle = {4, 4};
    snare.hitCount = 2;
    snare.rotation = 1;
    snare.baseVelocity = 105;
    snare.probability = 1.0f;
    snare.velocitySpread = 0.15f;
    snare.mutationRate = 0.10f;

    auto& hh = s.lanes[2];
    hh.id = 2;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {16, 16};
    hh.hitCount = 7;
    hh.baseVelocity = 80;
    hh.probability = 0.9f;
    hh.ghostFloor = 45;
    hh.velocitySpread = 0.40f;
    hh.mutationRate = 0.15f;

    auto& ghostLayer = s.lanes[3];
    ghostLayer.id = 3;
    ghostLayer.role = Role::Ghost;
    ghostLayer.midiNote = 38;
    ghostLayer.cycle = {16, 16};
    ghostLayer.hitCount = 11;
    ghostLayer.rotation = 2;
    ghostLayer.baseVelocity = 35;
    ghostLayer.probability = 0.75f;
    ghostLayer.ghostFloor = 25;
    ghostLayer.velocitySpread = 0.60f;
    ghostLayer.mutationRate = 0.20f;

    s.macros.syncopation = 0.5f;
    s.macros.tension = 0.6f;
    return s;
}

GrooveState makeLiquidDnB() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 124;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {8, 8};
    kick.hitCount = 2;
    kick.baseVelocity = 100;
    kick.probability = 1.0f;
    kick.noteDuration = 0.25f;
    kick.velocitySpread = 0.15f;
    kick.mutationRate = 0.05f;

    auto& snare = s.lanes[1];
    snare.id = 1;
    snare.role = Role::Backbeat;
    snare.midiNote = 38;
    snare.cycle = {4, 4};
    snare.hitCount = 2;
    snare.rotation = 1;
    snare.baseVelocity = 95;
    snare.probability = 1.0f;
    snare.velocitySpread = 0.10f;
    snare.mutationRate = 0.05f;

    auto& hh = s.lanes[2];
    hh.id = 2;
    hh.role = Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {16, 16};
    hh.hitCount = 9;
    hh.baseVelocity = 65;
    hh.probability = 0.9f;
    hh.ghostFloor = 35;
    hh.velocitySpread = 0.30f;
    hh.mutationRate = 0.10f;

    auto& ride = s.lanes[3];
    ride.id = 3;
    ride.role = Role::Ghost;
    ride.midiNote = 51;
    ride.cycle = {8, 8};
    ride.hitCount = 5;
    ride.rotation = 2;
    ride.baseVelocity = 55;
    ride.probability = 0.85f;
    ride.ghostFloor = 30;
    ride.velocitySpread = 0.25f;
    ride.mutationRate = 0.10f;

    s.macros.density = 0.35f;
    s.macros.humanize = 0.1f;
    return s;
}

GrooveState makeAfroElectronic() {
    GrooveState s{};
    s.activeLaneCount = 5;
    s.seed = 125;

    auto& clave = s.lanes[0];
    clave.id = 0;
    clave.role = Role::AnchorPulse;
    clave.midiNote = 75;
    clave.cycle = {8, 8};
    clave.hitCount = 3;
    clave.baseVelocity = 100;
    clave.probability = 1.0f;
    clave.swingAmount = 0.20f;

    auto& technoKick = s.lanes[1];
    technoKick.id = 1;
    technoKick.role = Role::Backbeat;
    technoKick.midiNote = 36;
    technoKick.cycle = {4, 4};
    technoKick.hitCount = 4;
    technoKick.baseVelocity = 110;
    technoKick.probability = 1.0f;
    technoKick.noteDuration = 0.2f;

    auto& polos = s.lanes[2];
    polos.id = 2;
    polos.role = Role::Shimmer;
    polos.midiNote = 62;
    polos.cycle = {12, 8};
    polos.hitCount = 7;
    polos.baseVelocity = 70;
    polos.probability = 0.9f;
    polos.ghostFloor = 45;
    polos.swingAmount = 0.10f;

    auto& sangsih = s.lanes[3];
    sangsih.id = 3;
    sangsih.role = Role::Accent;
    sangsih.midiNote = 64;
    sangsih.cycle = {12, 8};
    sangsih.hitCount = 5;
    sangsih.rotation = 2;
    sangsih.baseVelocity = 65;
    sangsih.probability = 0.85f;
    sangsih.ghostFloor = 40;
    sangsih.kotekanSourceLane = 0;
    sangsih.swingAmount = 0.10f;

    auto& ghostShaker = s.lanes[4];
    ghostShaker.id = 4;
    ghostShaker.role = Role::Ghost;
    ghostShaker.midiNote = 70;
    ghostShaker.cycle = {16, 16};
    ghostShaker.hitCount = 9;
    ghostShaker.rotation = 3;
    ghostShaker.baseVelocity = 50;
    ghostShaker.probability = 0.8f;
    ghostShaker.ghostFloor = 30;
    ghostShaker.swingAmount = 0.15f;

    s.macros.swing = 0.15f;
    s.macros.complexity = 0.5f;
    return s;
}

GrooveState makeBalkanFunk() {
    GrooveState s{};
    s.activeLaneCount = 4;
    s.seed = 126;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {7, 8};
    kick.hitCount = 3;
    kick.baseVelocity = 105;
    kick.probability = 1.0f;
    kick.noteDuration = 0.2f;

    auto& funkSnare = s.lanes[1];
    funkSnare.id = 1;
    funkSnare.role = Role::Backbeat;
    funkSnare.midiNote = 38;
    funkSnare.cycle = {7, 8};
    funkSnare.hitCount = 5;
    funkSnare.rotation = 1;
    funkSnare.baseVelocity = 85;
    funkSnare.probability = 0.9f;
    funkSnare.ghostFloor = 60;
    funkSnare.velocitySpread = 0.35f;

    auto& ghostHat = s.lanes[2];
    ghostHat.id = 2;
    ghostHat.role = Role::Shimmer;
    ghostHat.midiNote = 42;
    ghostHat.cycle = {14, 16};
    ghostHat.hitCount = 9;
    ghostHat.baseVelocity = 55;
    ghostHat.probability = 0.85f;
    ghostHat.ghostFloor = 40;
    ghostHat.velocitySpread = 0.20f;
    ghostHat.swingAmount = 0.15f;

    auto& rimAccent = s.lanes[3];
    rimAccent.id = 3;
    rimAccent.role = Role::Accent;
    rimAccent.midiNote = 37;
    rimAccent.cycle = {7, 8};
    rimAccent.hitCount = 2;
    rimAccent.rotation = 3;
    rimAccent.baseVelocity = 90;
    rimAccent.probability = 1.0f;

    s.macros.tension = 0.4f;
    s.macros.syncopation = 0.3f;
    return s;
}

GrooveState makeCompositionalArc() {
    GrooveState s{};
    s.activeLaneCount = 6;
    s.seed = 127;

    auto& kick = s.lanes[0];
    kick.id = 0;
    kick.role = Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {4, 4};
    kick.hitCount = 4;
    kick.baseVelocity = 110;
    kick.probability = 1.0f;
    kick.noteDuration = 0.25f;

    auto& bell = s.lanes[1];
    bell.id = 1;
    bell.role = Role::Accent;
    bell.midiNote = 56;
    bell.cycle = {12, 8};
    bell.hitCount = 7;
    bell.baseVelocity = 90;
    bell.probability = 1.0f;
    bell.noteDuration = 0.1f;

    auto& ghostHat = s.lanes[2];
    ghostHat.id = 2;
    ghostHat.role = Role::Shimmer;
    ghostHat.midiNote = 42;
    ghostHat.cycle = {16, 16};
    ghostHat.hitCount = 11;
    ghostHat.rotation = 2;
    ghostHat.baseVelocity = 55;
    ghostHat.probability = 0.85f;
    ghostHat.ghostFloor = 50;
    ghostHat.velocitySpread = 0.25f;

    auto& conga = s.lanes[3];
    conga.id = 3;
    conga.role = Role::Ghost;
    conga.midiNote = 63;
    conga.cycle = {8, 8};
    conga.hitCount = 5;
    conga.rotation = 1;
    conga.baseVelocity = 75;
    conga.probability = 0.85f;
    conga.ghostFloor = 40;
    conga.velocitySpread = 0.15f;
    conga.phraseLength = 12.0f;
    conga.phraseGap = 4.0f;

    auto& rimAccent = s.lanes[4];
    rimAccent.id = 4;
    rimAccent.role = Role::Ornament;
    rimAccent.midiNote = 37;
    rimAccent.cycle = {7, 8};
    rimAccent.hitCount = 3;
    rimAccent.baseVelocity = 85;
    rimAccent.probability = 0.8f;
    rimAccent.phraseLength = 8.0f;
    rimAccent.phraseGap = 8.0f;

    auto& leadOrn = s.lanes[5];
    leadOrn.id = 5;
    leadOrn.role = Role::Fill;
    leadOrn.midiNote = 47;
    leadOrn.cycle = {5, 8};
    leadOrn.hitCount = 3;
    leadOrn.rotation = 2;
    leadOrn.baseVelocity = 80;
    leadOrn.probability = 0.75f;
    leadOrn.ghostFloor = 55;
    leadOrn.velocitySpread = 0.30f;
    leadOrn.phraseLength = 6.0f;
    leadOrn.phraseGap = 6.0f;

    s.macros.density = 0.5f;
    s.macros.complexity = 0.4f;
    return s;
}

GrooveState makeFactoryPreset(int index) {
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
    case 5:
        return makeAfroHousePhrases();
    case 6:
        return makeReichPhasing();
    case 7:
        return makeKotekanInterlock();
    case 8:
        return makePocketGroove();
    case 9:
        return makeAfrobeat12_8();
    case 10:
        return makeBalkanAksak();
    case 11:
        return makeBossaNova();
    case 12:
        return makeCarnaticTala();
    case 13:
        return makeIDMGlitch();
    case 14:
        return makeEweAgbekor();
    case 15:
        return makeGamelanColotomic();
    case 16:
        return makePolymetricFoundation();
    case 17:
        return makeEweEnsemble();
    case 18:
        return makeMandingDjembe();
    case 19:
        return makeCubanSon();
    case 20:
        return makeAfrobeatLagos();
    case 21:
        return makeBaliKotekan();
    case 22:
        return makeJavaColotomic();
    case 23:
        return makeTintal();
    case 24:
        return makeRupakTal();
    case 25:
        return makeRachenitsa();
    case 26:
        return makeKopanitsa();
    case 27:
        return makeReichProcess();
    case 28:
        return makeRileyLayers();
    case 29:
        return makeNancarrowTempi();
    case 30:
        return makeMinimalTechno();
    case 31:
        return makeDeepHouse();
    case 32:
        return makeSambaBatucada();
    case 33:
        return makeBossaTrio();
    case 34:
        return makeClassicFunk();
    case 35:
        return makeNeoSoul();
    case 36:
        return makeJazzBop();
    case 37:
        return makeElvinCascade();
    case 38:
        return makeJungleBreak();
    case 39:
        return makeLiquidDnB();
    case 40:
        return makeAfroElectronic();
    case 41:
        return makeBalkanFunk();
    case 42:
        return makeCompositionalArc();
    default:
        return GrooveState{};
    }
}

const PresetInfo& getFactoryPresetInfo(int index) {
    static constexpr PresetInfo kInfos[kFactoryPresetCount] = {
        {"Four on the Floor", "Classic club groove with straight 8th hats and polymetric open hat"},
        {"Polymetric Drift", "Prime-number cycles (3, 5, 7, 11) creating evolving phase patterns"},
        {"Sparse Pulse", "Minimal, spacious groove with wide spacing and gentle ghost notes"},
        {"Breakbeat", "Syncopated kick with punchy snare, fast hats and ghost toms"},
        {"Latin Feel", "Clave-inspired pattern with conga, shaker and cowbell ornaments"},
        {"Afro-House Phrases", "Offset phrase loops — shaker continuous, conga and djembe breathe on staggered cycles"},
        {"Reich Phasing", "Two identical patterns gradually phase apart creating emergent resultant rhythms"},
        {"Kotekan Interlock", "Balinese interlocking pair — polos and sangsih fill each other's gaps"},
        {"Pocket Groove", "J Dilla-style micro-timing — kick pushes late, snare pulls early, gentle mutation"},
        {"Afrobeat 12/8", "Compound-time groove with timeline bell pattern, four-on-the-floor kick and conga ghosts"},
        {"Balkan Aksak", "7/8 aksak [2+2+3] additive cells — davul, rim, zurna and darbuka"},
        {"Bossa Nova", "Clave timeline with ginga micro-timing — surdo, tamborim, agogo and pandeiro"},
        {"Carnatic Tala", "Adi tala [4+2+2] additive cells — mridangam, ghatam and kanjira"},
        {"IDM Glitch", "Irregular additive cells with heavy mutation and erratic micro-timing offsets"},
        {"Sub-Saharan: Agbekor", "Ewe-inspired polymetric ensemble — gankogui bell timeline, kidi, sogo and lead drum"},
        {"Gamelan: Colotomic", "Javanese colotomic nesting — ketuk, kempul, kenong and gong ageng at 4:8:16:32 ratios"},
        {"Polymetric Foundation", "Two-lane polymetric demonstration — 12-step bell against 7-step counter pattern"},
        {"Ewe Polymetric Ensemble", "Gankogui bell timeline with support, responding, and interlocking lead drums"},
        {"Manding Djembe", "Three-lane djembe ensemble — dunun, sangban, and lead sharing an 8-step cycle"},
        {"Cuban Son Montuno", "Clave matrix with cascara, tumbao bass, conga, and shaker layers"},
        {"Afrobeat Lagos",
         "Extended Afrobeat groove — bell timeline with staggered phrase gating on snare, shaker, and conga"},
        {"Balinese Kotekan", "Polos and sangsih interlocking with jegogan bass and reyong accents"},
        {"Javanese Colotomic", "Nested colotomic hierarchy — ketuk, kempul, kenong, and gong at 4:8:16:32"},
        {"Tintal Groove", "Hindustani 16-beat cycle with sam, theka, dugun, and tigun layakari layers"},
        {"Rupak Tal", "7-beat Hindustani cycle — sam marker, theka, and counter rhythm"},
        {"Rachenitsa 7/8", "Bulgarian rachenitsa — tupan, kaval, and gadulka in seven-beat cycle"},
        {"Kopanitsa 11/8", "Fast Bulgarian kopanitsa — kick, snare, hat, and bell in eleven-beat cycle"},
        {"Reich Phase Process", "Gradual phase-shifting — two identical 12-step patterns drifting apart"},
        {"Riley Layered Entry", "Staggered voice entries with phrase gating — foundation plus four gated layers"},
        {"Nancarrow Tempi", "Independent tempo multipliers — anchor, double-time, half-time, and hemiola voices"},
        {"Minimal Techno", "Four-on-the-floor kick with polymetric 7-step ghost percussion and sparse clap"},
        {"Deep House", "Swung house groove — open hat, shaker, and rim with moderate swing and humanize"},
        {"Samba Batucada", "Full batucada ensemble — surdo, tamborim, agogo, repinique, and caixa"},
        {"Bossa Nova Trio", "Intimate bossa — E(5,16) bass, ride cymbal, and ghost brush"},
        {"Classic Funk", "JB-style pocket — kick pushes late, snare pulls early, dense ghost layer"},
        {"Neo-Soul Pocket", "Loose, warm groove with heavy humanize and polymetric 12-step rim click"},
        {"Jazz Bop Ride", "Bop timekeeping — ride and hi-hat foot with mutating kick and snare comp"},
        {"Elvin Jones Cascade", "Polyrhythmic cascade — 4, 3, 5, and 7-step cycles with swing and mutation"},
        {"Jungle Break", "Syncopated chopped break at 170 — dense ghost layer with rolling undertow"},
        {"Liquid Drum and Bass", "Clean two-step kick with steady hat and warm ride cymbal wash"},
        {"Afro-Electronic Fusion", "Cuban clave meets techno kick and gamelan-inspired kotekan shimmer"},
        {"Balkan Funk", "7/8 aksak with funk ghost notes and micro-timing on the hi-hat"},
        {"Compositional Arc", "Six-lane layered build — three continuous lanes plus three gated ornamental voices"},
    };
    static constexpr PresetInfo kEmpty{"", ""};
    if (index >= 0 && index < kFactoryPresetCount)
        return kInfos[index];
    return kEmpty;
}

} // namespace poly
