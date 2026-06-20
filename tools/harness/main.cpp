#include "poly/engine.h"
#include "poly/macro.h"
#include "poly/types.h"

#include <cstdio>
#include <cstdlib>

static poly::GrooveState makeDefaultState() {
    poly::GrooveState state{};
    state.activeLaneCount = 4;
    state.seed = 42;

    // Lane 0: Kick — anchor pulse, 4 on the floor
    auto& kick = state.lanes[0];
    kick.id = 0;
    kick.role = poly::Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {.steps = 4, .subdivision = 4};
    kick.hitCount = 4;
    kick.baseVelocity = 110;
    kick.probability = 1.0f;

    // Lane 1: Snare — backbeat on 2 and 4
    auto& snare = state.lanes[1];
    snare.id = 1;
    snare.role = poly::Role::Backbeat;
    snare.midiNote = 38;
    snare.cycle = {.steps = 4, .subdivision = 4};
    snare.hitCount = 2;
    snare.baseVelocity = 100;
    snare.probability = 1.0f;

    // Lane 2: Hi-hat — 8ths
    auto& hh = state.lanes[2];
    hh.id = 2;
    hh.role = poly::Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {.steps = 8, .subdivision = 8};
    hh.hitCount = 8;
    hh.baseVelocity = 80;
    hh.probability = 0.9f;
    hh.velocitySpread = 0.1f;

    // Lane 3: Ghost tom — polymetric 5/16
    auto& ghost = state.lanes[3];
    ghost.id = 3;
    ghost.role = poly::Role::Ghost;
    ghost.midiNote = 45;
    ghost.cycle = {.steps = 5, .subdivision = 16};
    ghost.hitCount = 3;
    ghost.baseVelocity = 50;
    ghost.probability = 0.7f;
    ghost.ghostFloor = 25;
    ghost.velocitySpread = 0.15f;

    return state;
}

int main(int argc, char* argv[]) {
    double bars = 4.0;
    double tempo = 120.0;
    double blockSizePpq = 0.1;

    if (argc > 1) bars = std::atof(argv[1]);
    if (argc > 2) tempo = std::atof(argv[2]);
    if (argc > 3) blockSizePpq = std::atof(argv[3]);

    if (bars <= 0.0 || tempo <= 0.0 || blockSizePpq <= 0.0) {
        std::fprintf(stderr, "Usage: poly_harness [bars=4] [tempo=120] [block_ppq=0.1]\n");
        return 1;
    }

    double totalPpq = bars * 4.0;

    std::printf("# poly_harness: bars=%.1f tempo=%.0f block_ppq=%.3f total_ppq=%.1f\n",
                bars, tempo, blockSizePpq, totalPpq);
    std::printf("# ppq_position  pitch  velocity  duration  channel\n");

    poly::Engine engine;
    poly::GrooveState state = poly::resolveMacros(makeDefaultState());
    poly::NoteEventBuffer buffer;

    int totalEvents = 0;
    double ppq = 0.0;

    while (ppq < totalPpq) {
        double blockEnd = ppq + blockSizePpq;
        if (blockEnd > totalPpq) blockEnd = totalPpq;

        poly::TransportContext tc{};
        tc.ppqStart = ppq;
        tc.ppqEnd = blockEnd;
        tc.tempo = tempo;
        tc.playing = true;

        engine.renderRange(tc, state, buffer);

        for (size_t i = 0; i < buffer.count; ++i) {
            const auto& e = buffer.events[i];
            std::printf("%.6f  %3d  %.3f  %.6f  %d\n",
                        e.ppqPosition, e.pitch, e.velocity,
                        e.duration, e.channel);
            ++totalEvents;
        }

        ppq = blockEnd;
    }

    std::printf("# total_events: %d\n", totalEvents);
    return 0;
}
