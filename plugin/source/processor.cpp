#include "processor.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"

#include "plugids.h"
#include "poly/constraint.h"
#include "poly/envelope.h"
#include "poly/macro.h"
#include "poly/scene.h"
#include "poly/smf_writer.h"
#include "poly/state_io.h"

namespace poly {

PolyProcessor::PolyProcessor() {
    setControllerClass(kPolyControllerUID);

    struct LaneDef {
        int16_t note;
        int steps;
        int sub;
        int hits;
        Role role;
    };
    static constexpr LaneDef kDefs[kMaxLanes] = {
        {36, 4, 4, 4, Role::AnchorPulse}, // Kick: 4-on-the-floor
        {38, 4, 4, 2, Role::Backbeat},    // Snare: backbeat
        {42, 8, 8, 8, Role::Shimmer},     // Closed HH: 8ths
        {45, 5, 16, 3, Role::Ghost},      // Low Tom: polymetric 5/16
        {46, 7, 8, 4, Role::Accent},      // Open HH: polymetric 7/8
        {39, 3, 16, 2, Role::Ornament},   // Clap: polymetric 3/16
        {43, 6, 16, 4, Role::Fill},       // Low Tom 2: polymetric 6/16
        {50, 9, 16, 5, Role::Custom},     // High Tom: polymetric 9/16
    };
    for (int i = 0; i < kMaxLanes; ++i) {
        sceneState_.sceneA.lanes[i].id = i;
        sceneState_.sceneA.lanes[i].role = kDefs[i].role;
        sceneState_.sceneA.lanes[i].midiNote = kDefs[i].note;
        sceneState_.sceneA.lanes[i].cycle = {kDefs[i].steps, kDefs[i].sub};
        sceneState_.sceneA.lanes[i].hitCount = kDefs[i].hits;
    }
}

Steinberg::tresult PLUGIN_API PolyProcessor::initialize(Steinberg::FUnknown* context) {
    auto result = AudioEffect::initialize(context);
    if (result != Steinberg::kResultOk)
        return result;

    addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);
    addEventOutput(STR16("MIDI Out"), 16);

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API PolyProcessor::setActive(Steinberg::TBool state) {
    if (state) {
        pendingNoteOffs_.clear();
        noteBuffer_.clear();
        captureBuffer_.clear();
        exportTriggered_ = false;
        expectedNextPpq_ = -1.0;
        macroSmoother_.initialized = false;
    }
    return AudioEffect::setActive(state);
}

Steinberg::tresult PLUGIN_API PolyProcessor::process(Steinberg::Vst::ProcessData& data) {

    // --- Silence audio outputs ---
    if (data.numOutputs > 0) {
        for (Steinberg::int32 ch = 0; ch < data.outputs[0].numChannels; ++ch) {
            if (data.outputs[0].channelBuffers32[ch]) {
                std::memset(data.outputs[0].channelBuffers32[ch], 0,
                            static_cast<size_t>(data.numSamples) * sizeof(float));
            }
        }
        data.outputs[0].silenceFlags = 0x3;
    }

    // --- Read parameter changes ---
    if (auto* paramChanges = data.inputParameterChanges) {
        Steinberg::int32 numParams = paramChanges->getParameterCount();
        for (Steinberg::int32 i = 0; i < numParams; ++i) {
            auto* queue = paramChanges->getParameterData(i);
            if (!queue)
                continue;
            Steinberg::Vst::ParamValue value;
            Steinberg::int32 sampleOffset;
            Steinberg::int32 pointCount = queue->getPointCount();
            if (pointCount > 0 && queue->getPoint(pointCount - 1, sampleOffset, value) == Steinberg::kResultOk) {
                applyParameter(queue->getParameterId(), value);
            }
        }
    }

    // --- Build TransportContext from ProcessContext ---
    if (data.processContext) {
        auto& ctx = *data.processContext;
        tc_.tempo = ctx.tempo;
        tc_.sampleRate = ctx.sampleRate;
        tc_.blockSize = data.numSamples;
        tc_.playing = (ctx.state & Steinberg::Vst::ProcessContext::kPlaying) != 0;
        tc_.looping = (ctx.state & Steinberg::Vst::ProcessContext::kCycleActive) != 0;

        if (ctx.state & Steinberg::Vst::ProcessContext::kProjectTimeMusicValid) {
            double ppqStart = ctx.projectTimeMusic;
            double beatsPerSample = tc_.tempo > 0.0 ? tc_.tempo / (60.0 * tc_.sampleRate) : 0.0;
            tc_.ppqStart = ppqStart;
            tc_.ppqEnd = ppqStart + data.numSamples * beatsPerSample;

            constexpr double kJumpThreshold = 0.001;
            tc_.jumped = (expectedNextPpq_ >= 0.0 && std::abs(ppqStart - expectedNextPpq_) > kJumpThreshold);
            expectedNextPpq_ = tc_.ppqEnd;
        } else {
            tc_.ppqStart = 0.0;
            tc_.ppqEnd = 0.0;
            tc_.jumped = false;
            expectedNextPpq_ = -1.0;
        }

        if (ctx.state & Steinberg::Vst::ProcessContext::kCycleValid) {
            tc_.loopStartPpq = ctx.cycleStartMusic;
            tc_.loopEndPpq = ctx.cycleEndMusic;
        }
    }

    if (!tc_.playing)
        return Steinberg::kResultOk;

    // --- Handle transport jump: kill pending noteoffs, clear capture ---
    if (tc_.jumped) {
        captureBuffer_.clear();
        if (data.outputEvents) {
            PendingNoteOff allOffs[PendingNoteOffBuffer::kCapacity];
            size_t n = pendingNoteOffs_.flushDue(-1e12, 1e12, allOffs, PendingNoteOffBuffer::kCapacity);
            for (size_t i = 0; i < n; ++i) {
                Steinberg::Vst::Event ev = {};
                ev.busIndex = 0;
                ev.sampleOffset = 0;
                ev.ppqPosition = tc_.ppqStart;
                ev.type = Steinberg::Vst::Event::kNoteOffEvent;
                ev.noteOff.channel = allOffs[i].channel;
                ev.noteOff.pitch = allOffs[i].pitch;
                ev.noteOff.velocity = 0.0f;
                ev.noteOff.noteId = -1;
                data.outputEvents->addEvent(ev);
            }
        }
        pendingNoteOffs_.clear();
    }

    // --- Resolve scene with smoothed macros and render ---
    GrooveState base;
    if (sceneState_.select == SceneSelect::Morph) {
        base = interpolateGrooveState(sceneState_.sceneA, sceneState_.sceneB, sceneState_.morphAmount);
    } else if (sceneState_.select == SceneSelect::B) {
        base = sceneState_.sceneB;
    } else {
        base = sceneState_.sceneA;
    }

    macroSmoother_.setTarget(base.macros);
    if (tc_.jumped)
        macroSmoother_.snapToTarget();
    macroSmoother_.advance(tc_.sampleRate, tc_.blockSize);
    base.macros = macroSmoother_.current;

    GrooveState resolved = resolveConstraints(base, resolveMacros(base));
    engine_.renderRange(tc_, resolved, noteBuffer_);

    // --- Emit MIDI events ---
    auto* outputEvents = data.outputEvents;
    if (!outputEvents)
        return Steinberg::kResultOk;

    // Flush due noteoffs
    PendingNoteOff dueOffs[kMaxEventsPerBlock];
    size_t numDue = pendingNoteOffs_.flushDue(tc_.ppqStart, tc_.ppqEnd, dueOffs, kMaxEventsPerBlock);

    for (size_t i = 0; i < numDue; ++i) {
        Steinberg::Vst::Event ev = {};
        ev.busIndex = 0;
        ev.sampleOffset =
            ppqToSampleOffset(dueOffs[i].ppqOff, tc_.ppqStart, tc_.tempo, tc_.sampleRate, data.numSamples);
        ev.ppqPosition = dueOffs[i].ppqOff;
        ev.type = Steinberg::Vst::Event::kNoteOffEvent;
        ev.noteOff.channel = dueOffs[i].channel;
        ev.noteOff.pitch = dueOffs[i].pitch;
        ev.noteOff.velocity = 0.0f;
        ev.noteOff.noteId = -1;
        outputEvents->addEvent(ev);
    }

    // Emit noteons, schedule noteoffs, track per-lane velocity
    float laneVelocity[kMaxLanes] = {};

    for (size_t i = 0; i < noteBuffer_.count; ++i) {
        const auto& note = noteBuffer_.events[i];

        Steinberg::Vst::Event ev = {};
        ev.busIndex = 0;
        ev.sampleOffset = ppqToSampleOffset(note.ppqPosition, tc_.ppqStart, tc_.tempo, tc_.sampleRate, data.numSamples);
        ev.ppqPosition = note.ppqPosition;
        ev.type = Steinberg::Vst::Event::kNoteOnEvent;
        ev.noteOn.channel = note.channel;
        ev.noteOn.pitch = note.pitch;
        ev.noteOn.velocity = note.velocity;
        ev.noteOn.noteId = -1;
        outputEvents->addEvent(ev);

        pendingNoteOffs_.push({
            .ppqOff = note.ppqPosition + note.duration,
            .pitch = note.pitch,
            .channel = note.channel,
        });

        if (note.channel >= 0 && note.channel < kMaxLanes) {
            laneVelocity[note.channel] = note.velocity;
        }
    }

    // --- Push to capture buffer ---
    for (size_t i = 0; i < noteBuffer_.count; ++i) {
        captureBuffer_.push(noteBuffer_.events[i]);
    }

    // --- Export trigger: snapshot to staging buffer ---
    if (exportTriggered_ && !exportReady_.load(std::memory_order_acquire)) {
        exportTriggered_ = false;
        exportEventCount_ = captureBuffer_.copyRaw(exportEvents_.data(), exportEvents_.size());
        exportTempo_ = tc_.tempo;
        exportReady_.store(true, std::memory_order_release);
    }

    if (auto* outParams = data.outputParameterChanges) {
        if (noteBuffer_.count > 0) {
            for (int lane = 0; lane < kMaxLanes; ++lane) {
                if (laneVelocity[lane] > 0.0f) {
                    Steinberg::int32 idx;
                    auto* queue = outParams->addParameterData(ParamIDs::velocityOutput(lane), idx);
                    if (queue) {
                        Steinberg::int32 ptIdx;
                        queue->addPoint(0, laneVelocity[lane], ptIdx);
                    }
                }
            }
        }

        for (int lane = 0; lane < kMaxLanes; ++lane) {
            if (!resolved.lanes[lane].active)
                continue;

            double cycleBeats = resolved.lanes[lane].cycle.steps * (4.0 / resolved.lanes[lane].cycle.subdivision);
            double lanePhase = std::fmod(tc_.ppqStart / cycleBeats, 1.0);
            if (lanePhase < 0.0)
                lanePhase += 1.0;

            if (resolved.lanes[lane].driftRate != 0.0f) {
                double barPos = tc_.ppqStart / 4.0;
                int stepsInCycle = resolved.lanes[lane].cycle.steps;
                auto driftSteps =
                    static_cast<int64_t>(std::floor(barPos * static_cast<double>(resolved.lanes[lane].driftRate)));
                double driftFrac =
                    static_cast<double>(((driftSteps % stepsInCycle) + stepsInCycle) % stepsInCycle) / stepsInCycle;
                lanePhase = std::fmod(lanePhase + driftFrac + 1.0, 1.0);
            }

            Steinberg::int32 idx;
            auto* phaseQueue = outParams->addParameterData(ParamIDs::lanePhaseOutput(lane), idx);
            if (phaseQueue) {
                Steinberg::int32 ptIdx;
                phaseQueue->addPoint(0, lanePhase, ptIdx);
            }

            double envValue = 0.5;
            if (resolved.lanes[lane].envelopeCount > 0) {
                const auto& env = resolved.lanes[lane].envelopes[0].envelope;
                double envPhase = computeEnvelopePhase(tc_.ppqStart, env.periodBars, env.phaseOffset);
                envValue = static_cast<double>(evaluateShapeFull(env, static_cast<float>(envPhase)));
            }

            auto* envQueue = outParams->addParameterData(ParamIDs::envelopeValueOutput(lane), idx);
            if (envQueue) {
                Steinberg::int32 ptIdx;
                envQueue->addPoint(0, envValue, ptIdx);
            }

            double phrasePhase = 0.0;
            if (resolved.lanes[lane].phraseLength > 0.0f) {
                double phraseLenPpq = static_cast<double>(resolved.lanes[lane].phraseLength);
                double phraseGapPpq = static_cast<double>(resolved.lanes[lane].phraseGap);
                double phraseCyclePpq = phraseLenPpq + phraseGapPpq;
                if (phraseCyclePpq > 0.0) {
                    double offPpq = static_cast<double>(resolved.lanes[lane].phraseOffset);
                    double pos = std::fmod(tc_.ppqStart - offPpq, phraseCyclePpq);
                    if (pos < 0.0)
                        pos += phraseCyclePpq;
                    phrasePhase = pos / phraseCyclePpq;
                }
            }

            auto* phraseQueue = outParams->addParameterData(ParamIDs::phrasePhaseOutput(lane), idx);
            if (phraseQueue) {
                Steinberg::int32 ptIdx;
                phraseQueue->addPoint(0, phrasePhase, ptIdx);
            }
        }
    }

    return Steinberg::kResultOk;
}

void PolyProcessor::applyParameter(Steinberg::Vst::ParamID id, double normalized) {
    using namespace ParamIDs;

    if (id == kSceneSelect) {
        int sel = static_cast<int>(std::round(normalized * 2.0));
        sceneState_.select = static_cast<SceneSelect>(std::clamp(sel, 0, 2));
        return;
    }
    if (id == kSceneMorph) {
        sceneState_.morphAmount = static_cast<float>(normalized);
        return;
    }

    auto& gs = (sceneState_.select == SceneSelect::B) ? sceneState_.sceneB : sceneState_.sceneA;

    if (id >= kLaneCoreBase &&
        id < kLaneCoreBase + static_cast<Steinberg::Vst::ParamID>(kMaxLanes * kCoreParamsPerLane)) {
        auto rel = static_cast<int>(id - kLaneCoreBase);
        int lane = rel / kCoreParamsPerLane;
        int offset = rel % kCoreParamsPerLane;
        auto& cfg = gs.lanes[lane];
        switch (offset) {
        case kCoreSteps:
            cfg.cycle.steps = 1 + static_cast<int>(std::round(normalized * 63.0));
            break;
        case kCoreSubdivision: {
            static constexpr int subdivs[] = {1, 2, 4, 8, 16};
            int idx = static_cast<int>(std::round(normalized * 4.0));
            cfg.cycle.subdivision = subdivs[std::clamp(idx, 0, 4)];
            break;
        }
        case kCoreHits:
            cfg.hitCount = static_cast<int>(std::round(normalized * 64.0));
            break;
        case kCoreRotation:
            cfg.rotation = static_cast<int>(std::round(normalized * 63.0));
            break;
        case kCoreMidiNote:
            cfg.midiNote = static_cast<int16_t>(std::round(normalized * 127.0));
            break;
        case kCoreCellCount:
            cfg.cellCount = static_cast<int>(std::round(normalized * 64.0));
            break;
        default:
            break;
        }
        return;
    }

    if (id < static_cast<Steinberg::Vst::ParamID>(kMaxLanes * kParamsPerLane)) {
        int lane = static_cast<int>(id) / kParamsPerLane;
        int offset = static_cast<int>(id) % kParamsPerLane;
        auto& cfg = gs.lanes[lane];

        switch (offset) {
        case kProbability:
            cfg.probability = static_cast<float>(normalized);
            break;
        case kBaseVelocity:
            cfg.baseVelocity = static_cast<uint8_t>(std::round(normalized * 127.0));
            break;
        case kEmphasisProb:
            cfg.emphasisProb = static_cast<float>(normalized);
            break;
        case kGhostFloor:
            cfg.ghostFloor = static_cast<uint8_t>(std::round(normalized * 127.0));
            break;
        case kVelocitySpread:
            cfg.velocitySpread = static_cast<float>(normalized);
            break;
        case kSwingAmount:
            cfg.swingAmount = static_cast<float>(normalized);
            break;
        case kHumanizeMs:
            cfg.humanizeMs = static_cast<float>(normalized * 50.0);
            break;
        case kNoteDuration:
            cfg.noteDuration = static_cast<float>(normalized * 4.0);
            break;
        case kActive:
            cfg.active = (normalized > 0.5);
            break;
        case kPhraseLength:
            cfg.phraseLength = static_cast<float>(normalized * 64.0);
            break;
        case kPhraseGap:
            cfg.phraseGap = static_cast<float>(normalized * 64.0);
            break;
        case kPhraseOffset:
            cfg.phraseOffset = static_cast<float>(normalized * 64.0);
            break;
        case kMutationRate:
            cfg.mutationRate = static_cast<float>(normalized);
            break;
        case kDriftRate:
            cfg.driftRate = static_cast<float>(normalized * 8.0 - 4.0);
            break;
        case kTimingOffset:
            cfg.timingOffsetMs = static_cast<float>(normalized * 40.0 - 20.0);
            break;
        case kKotekanSource:
            cfg.kotekanSourceLane = static_cast<int>(std::round(normalized * 8.0)) - 1;
            break;
        default:
            break;
        }
    } else {
        switch (id) {
        case kMacroComplexity:
            gs.macros.complexity = static_cast<float>(normalized);
            break;
        case kMacroDensity:
            gs.macros.density = static_cast<float>(normalized);
            break;
        case kMacroSyncopation:
            gs.macros.syncopation = static_cast<float>(normalized);
            break;
        case kMacroSwing:
            gs.macros.swing = static_cast<float>(normalized);
            break;
        case kMacroTension:
            gs.macros.tension = static_cast<float>(normalized);
            break;
        case kMacroHumanize:
            gs.macros.humanize = static_cast<float>(normalized);
            break;
        case kActiveLaneCount:
            gs.activeLaneCount = 1 + static_cast<int>(std::round(normalized * 7.0));
            break;
        case kSeed:
            gs.seed = static_cast<uint64_t>(std::round(normalized * 999999.0));
            break;
        case kExportTrigger:
            if (normalized > 0.5)
                exportTriggered_ = true;
            break;
        case kCaptureLength:
            captureLengthBars_ = 1 + static_cast<int>(std::round(normalized * 31.0));
            break;
        default:
            break;
        }
    }
}

Steinberg::tresult PLUGIN_API PolyProcessor::notify(Steinberg::Vst::IMessage* message) {
    if (!message)
        return Steinberg::kInvalidArgument;

    if (Steinberg::FIDStringsEqual(message->getMessageID(), "RequestMidiExport")) {
        if (exportReady_.load(std::memory_order_acquire)) {
            std::sort(exportEvents_.begin(), exportEvents_.begin() + exportEventCount_,
                      [](const NoteEvent& a, const NoteEvent& b) { return a.ppqPosition < b.ppqPosition; });

            double ppqOffset = (exportEventCount_ > 0) ? exportEvents_[0].ppqPosition : 0.0;
            auto smf = writeSMF(exportEvents_.data(), exportEventCount_, exportTempo_, ppqOffset);

            if (auto* reply = allocateMessage()) { // RT-SAFE-OK: notify() runs on message thread
                reply->setMessageID("MidiExportData");
                if (auto* attrs = reply->getAttributes()) {
                    attrs->setBinary("smf", smf.data(), static_cast<Steinberg::uint32>(smf.size()));
                }
                sendMessage(reply); // RT-SAFE-OK: notify() runs on message thread
                reply->release();
            }

            exportReady_.store(false, std::memory_order_release);
        }
        return Steinberg::kResultOk;
    }

    return AudioEffect::notify(message);
}

Steinberg::tresult PLUGIN_API PolyProcessor::getState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kInvalidArgument;

    auto write = [state](const void* data, size_t size) -> bool {
        Steinberg::int32 written;
        return state->write(const_cast<void*>(data), static_cast<Steinberg::int32>(size), &written) ==
               Steinberg::kResultOk;
    };

    return writeSceneState(write, sceneState_) ? Steinberg::kResultOk : Steinberg::kResultFalse;
}

Steinberg::tresult PLUGIN_API PolyProcessor::setState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kInvalidArgument;

    auto read = [state](void* data, size_t size) -> bool {
        Steinberg::int32 bytesRead;
        return state->read(data, static_cast<Steinberg::int32>(size), &bytesRead) == Steinberg::kResultOk;
    };

    return readSceneState(read, sceneState_) ? Steinberg::kResultOk : Steinberg::kResultFalse;
}

} // namespace poly
