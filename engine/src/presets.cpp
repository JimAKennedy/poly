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
    };
    static constexpr PresetInfo kEmpty{"", ""};
    if (index >= 0 && index < kFactoryPresetCount)
        return kInfos[index];
    return kEmpty;
}

} // namespace poly
