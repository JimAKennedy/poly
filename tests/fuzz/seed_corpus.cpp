// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// Seed-corpus generator for the libFuzzer targets (open-source-launch
// M002/S02). Writes well-formed inputs — serialized groove and scene states in
// the current wire format, and engine-written Standard MIDI Files — into
// <dir>/state_io and <dir>/midi_reader, so each fuzzer starts from the shape of
// a real input rather than from nothing. Generated at build time rather than
// committed: a committed blob would drift from the state format the moment
// kCurrentStateVersion moved, and the fuzzers would be seeded with history.
//
// Usage: fuzz_seed_corpus <dir>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "poly/euclidean.h"
#include "poly/presets.h"
#include "poly/scene.h"
#include "poly/smf_writer.h"
#include "poly/state_io.h"
#include "poly/types.h"

namespace {

bool writeFile(const std::filesystem::path& path, const std::vector<uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::fprintf(stderr, "fuzz_seed_corpus: cannot open %s\n", path.string().c_str());
        return false;
    }
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!out) {
        std::fprintf(stderr, "fuzz_seed_corpus: write failed for %s\n", path.string().c_str());
        return false;
    }
    return true;
}

// Serialize through the same WriteFn shape state_io.h takes everywhere.
template <typename Fn> std::vector<uint8_t> serialize(Fn&& fn) {
    std::vector<uint8_t> bytes;
    auto write = [&](const void* src, size_t len) -> bool {
        const auto* p = static_cast<const uint8_t*>(src);
        bytes.insert(bytes.end(), p, p + len);
        return true;
    };
    if (!fn(write))
        bytes.clear();
    return bytes;
}

// One cycle of E(k,n) at the given subdivision, each note one step long except
// the last, which is sustained to the cycle boundary so the file's final
// note-off encodes the full loop length — the same construction
// tests/midi_reader_tests.cpp uses for its round trips.
std::vector<poly::NoteEvent> euclideanEvents(int k, int n, int subdivision, int16_t pitch, int16_t laneIndex) {
    std::array<bool, poly::kMaxSteps> pattern{};
    poly::euclidean(k, n, 0, pattern);
    const double g = 4.0 / static_cast<double>(subdivision);
    const double loopLen = static_cast<double>(n) * g;
    std::vector<double> onsets;
    for (int i = 0; i < n; ++i) {
        if (pattern[static_cast<size_t>(i)])
            onsets.push_back(static_cast<double>(i) * g);
    }
    double maxOnset = 0.0;
    for (double o : onsets)
        maxOnset = o > maxOnset ? o : maxOnset;
    std::vector<poly::NoteEvent> events;
    for (double o : onsets) {
        poly::NoteEvent ev;
        ev.ppqPosition = o;
        ev.pitch = pitch;
        ev.velocity = 0.8f;
        ev.duration = (o == maxOnset) ? (loopLen - o) : g;
        ev.channel = 0;
        ev.laneIndex = laneIndex;
        events.push_back(ev);
    }
    return events;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: fuzz_seed_corpus <dir>\n");
        return 2;
    }
    const std::filesystem::path root(argv[1]);
    const auto stateDir = root / "state_io";
    const auto midiDir = root / "midi_reader";
    std::error_code ec;
    std::filesystem::create_directories(stateDir, ec);
    std::filesystem::create_directories(midiDir, ec);
    if (ec) {
        std::fprintf(stderr, "fuzz_seed_corpus: cannot create %s: %s\n", root.string().c_str(), ec.message().c_str());
        return 1;
    }

    int written = 0;

    // --- state_io: the current wire format, as the plugin writes it ---
    {
        const poly::GrooveState groove{};
        if (!writeFile(stateDir / "groove-default.bin",
                       serialize([&](auto& w) { return poly::writeGrooveState(w, groove); })))
            return 1;
        ++written;
    }
    for (int i = 0; i < 3; ++i) {
        const poly::GrooveState groove = poly::makeFactoryPreset(i);
        if (!writeFile(stateDir / ("groove-preset-" + std::to_string(i) + ".bin"),
                       serialize([&](auto& w) { return poly::writeGrooveState(w, groove); })))
            return 1;
        ++written;
    }
    {
        const poly::SceneState scene{};
        if (!writeFile(stateDir / "scene-default.bin",
                       serialize([&](auto& w) { return poly::writeSceneState(w, scene); })))
            return 1;
        ++written;
    }

    // --- midi_reader: engine-written SMF, single- and multi-instrument ---
    const struct {
        int k, n;
        const char* name;
    } spellings[] = {{3, 8, "euclid-3-8.mid"}, {5, 12, "euclid-5-12.mid"}, {7, 16, "euclid-7-16.mid"}};
    for (const auto& sp : spellings) {
        const auto events = euclideanEvents(sp.k, sp.n, 16, 36, 0);
        if (!writeFile(midiDir / sp.name, poly::writeSMF(events.data(), events.size(), 120.0)))
            return 1;
        ++written;
    }
    {
        auto events = euclideanEvents(3, 8, 16, 36, 0);
        const auto snare = euclideanEvents(5, 8, 16, 38, 1);
        events.insert(events.end(), snare.begin(), snare.end());
        if (!writeFile(midiDir / "two-instruments.mid", poly::writeSMF(events.data(), events.size(), 120.0)))
            return 1;
        ++written;
        const auto multi = poly::writeMultiTrackSMF(events.data(), events.size(), 120.0, 0.0,
                                                    [](int lane) { return std::string(lane == 0 ? "kick" : "snare"); });
        if (!writeFile(midiDir / "multitrack.mid", multi))
            return 1;
        ++written;
    }

    std::printf("fuzz_seed_corpus: wrote %d seed file(s) under %s\n", written, root.string().c_str());
    return 0;
}
