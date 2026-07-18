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
        chainState_.reset();
        // P2 fix (M046 S01 T03): publish initial stateSnapshot_ before the audio thread
        // starts running so a cold getState() (host loads preset / queries state before
        // the first processBlock) reads a well-defined snapshot instead of falling back
        // to a live sceneState_ read that could race a concurrent process() publish.
        stateSnapshot_ = sceneState_;
        snapshotReady_.store(true, std::memory_order_release);
    }
    return AudioEffect::setActive(state);
}

Steinberg::tresult PLUGIN_API PolyProcessor::connect(Steinberg::Vst::IConnectionPoint* other) {
    auto result = AudioEffect::connect(other);
    if (result == Steinberg::kResultOk || result == Steinberg::kResultTrue) {
        sendSnapshotPointer();
    }
    return result;
}

void PolyProcessor::sendSnapshotPointer() {
    // No processor-side disconnect override: the pointer we ship (`&uiSnapshot_`)
    // refers to a member of this processor and lives as long as the processor does,
    // so nothing on the sender side can dangle. The controller nulls its cached copy
    // via PolyControllerBase::disconnect() when the DAW tears the connection down.
    if (auto* msg = allocateMessage()) { // RT-SAFE-OK: connect() runs on message thread
        msg->setMessageID("UISnapshotPtr");
        if (auto* attrs = msg->getAttributes()) {
            attrs->setInt("ptr", reinterpret_cast<Steinberg::int64>(&uiSnapshot_));
        }
        sendMessage(msg); // RT-SAFE-OK: connect() runs on message thread
        msg->release();
    }
}

void PolyProcessor::updateTransportContext(const Steinberg::Vst::ProcessData& data) {
    if (!data.processContext)
        return;

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

void PolyProcessor::handleTransportJump(Steinberg::Vst::IEventList* outputEvents) {
    if (!tc_.jumped)
        return;

    captureBuffer_.clear();
    if (outputEvents) {
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
            outputEvents->addEvent(ev);
        }
    }
    pendingNoteOffs_.clear();
}

void PolyProcessor::emitMidiOutput(Steinberg::Vst::IEventList* outputEvents, Steinberg::int32 numSamples) {
    PendingNoteOff dueOffs[kMaxEventsPerBlock];
    size_t numDue = pendingNoteOffs_.flushDue(tc_.ppqStart, tc_.ppqEnd, dueOffs, kMaxEventsPerBlock);

    for (size_t i = 0; i < numDue; ++i) {
        Steinberg::Vst::Event ev = {};
        ev.busIndex = 0;
        ev.sampleOffset = ppqToSampleOffset(dueOffs[i].ppqOff, tc_.ppqStart, tc_.tempo, tc_.sampleRate, numSamples);
        ev.ppqPosition = dueOffs[i].ppqOff;
        ev.type = Steinberg::Vst::Event::kNoteOffEvent;
        ev.noteOff.channel = dueOffs[i].channel;
        ev.noteOff.pitch = dueOffs[i].pitch;
        ev.noteOff.velocity = 0.0f;
        ev.noteOff.noteId = -1;
        outputEvents->addEvent(ev);
    }

    // region:emit-note-on
    for (size_t i = 0; i < noteBuffer_.count; ++i) {
        const auto& note = noteBuffer_.events[i];

        Steinberg::Vst::Event ev = {};
        ev.busIndex = 0;
        ev.sampleOffset = ppqToSampleOffset(note.ppqPosition, tc_.ppqStart, tc_.tempo, tc_.sampleRate, numSamples);
        ev.ppqPosition = note.ppqPosition;
        ev.type = Steinberg::Vst::Event::kNoteOnEvent;
        auto mappedPitch = sceneState_.noteMap.apply(note.pitch);
        ev.noteOn.channel = note.channel;
        ev.noteOn.pitch = mappedPitch;
        ev.noteOn.velocity = note.velocity;
        ev.noteOn.noteId = -1;
        outputEvents->addEvent(ev);

        pendingNoteOffs_.push({
            .ppqOff = note.ppqPosition + note.duration,
            .pitch = mappedPitch,
            .channel = note.channel,
        });
    }
    // endregion:emit-note-on
}

static void outputLaneVisualization(Steinberg::Vst::IParameterChanges* outParams, const LaneConfig& cfg, int lane,
                                    double ppqStart, UISnapshot& snap) {
    auto additive = computeAdditiveCells(cfg);
    double laneTempoScale = (cfg.tempoMultiplier > 0.0f) ? 1.0 / static_cast<double>(cfg.tempoMultiplier) : 1.0;
    double cycleBeats = additive.count > 0 ? additive.totalPpq * laneTempoScale
                                           : cfg.cycle.steps * (4.0 / cfg.cycle.subdivision) * laneTempoScale;
    if (cycleBeats <= 0.0)
        return;
    double lanePhase = std::fmod(ppqStart / cycleBeats, 1.0);
    if (lanePhase < 0.0)
        lanePhase += 1.0;

    if (cfg.driftRate != 0.0f) {
        double barPos = ppqStart / 4.0;
        int stepsInCycle = additive.count > 0 ? additive.count : cfg.cycle.steps;
        auto driftSteps = static_cast<int64_t>(std::floor(barPos * static_cast<double>(cfg.driftRate)));
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
    snap.lanePhases[lane].store(lanePhase, std::memory_order_relaxed);

    double envValue = 0.5;
    if (cfg.envelopeCount > 0) {
        const auto& env = cfg.envelopes[0].envelope;
        double envPhase = computeEnvelopePhase(ppqStart, env.periodBars, env.phaseOffset);
        envValue = static_cast<double>(evaluateShapeFull(env, static_cast<float>(envPhase)));
    }

    auto* envQueue = outParams->addParameterData(ParamIDs::envelopeValueOutput(lane), idx);
    if (envQueue) {
        Steinberg::int32 ptIdx;
        envQueue->addPoint(0, envValue, ptIdx);
    }

    double phrasePhase = 0.0;
    if (cfg.phraseLength > 0.0f) {
        double phraseLenPpq = static_cast<double>(cfg.phraseLength);
        double phraseGapPpq = static_cast<double>(cfg.phraseGap);
        double phraseCyclePpq = phraseLenPpq + phraseGapPpq;
        if (phraseCyclePpq > 0.0) {
            double offPpq = static_cast<double>(cfg.phraseOffset);
            double pos = std::fmod(ppqStart - offPpq, phraseCyclePpq);
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

void PolyProcessor::outputParameterFeedback(Steinberg::Vst::ProcessData& data, const GrooveState& resolved) {
    auto* outParams = data.outputParameterChanges;
    if (!outParams)
        return;

    {
        Steinberg::int32 idx;
        auto* ppqQueue = outParams->addParameterData(ParamIDs::kTransportPpqOutput, idx);
        double ppqNorm = std::fmod(tc_.ppqStart, 128.0) / 128.0;
        if (ppqNorm < 0.0)
            ppqNorm += 1.0;
        if (ppqQueue) {
            Steinberg::int32 ptIdx;
            ppqQueue->addPoint(0, ppqNorm, ptIdx);
        }
        uiSnapshot_.ppqNorm.store(ppqNorm, std::memory_order_relaxed);
        uiSnapshot_.playing.store(true, std::memory_order_release);
    }

    if (noteBuffer_.count > 0) {
        for (int lane = 0; lane < kMaxLanes; ++lane) {
            float vel = 0.0f;
            for (size_t i = 0; i < noteBuffer_.count; ++i) {
                if (noteBuffer_.events[i].laneIndex == lane)
                    vel = noteBuffer_.events[i].velocity;
            }
            if (vel > 0.0f) {
                Steinberg::int32 idx;
                auto* queue = outParams->addParameterData(ParamIDs::velocityOutput(lane), idx);
                if (queue) {
                    Steinberg::int32 ptIdx;
                    queue->addPoint(0, vel, ptIdx);
                }
            }
        }
    }

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        if (resolved.lanes[lane].active)
            outputLaneVisualization(outParams, resolved.lanes[lane], lane, tc_.ppqStart, uiSnapshot_);
    }

    {
        Steinberg::int32 idx;
        auto* readyQueue = outParams->addParameterData(ParamIDs::kCaptureReady, idx);
        if (readyQueue) {
            Steinberg::int32 ptIdx;
            readyQueue->addPoint(0, captureBuffer_.empty() ? 0.0 : 1.0, ptIdx);
        }
    }

    {
        Steinberg::int32 idx;
        auto* countQueue = outParams->addParameterData(ParamIDs::kCaptureEventCount, idx);
        if (countQueue) {
            Steinberg::int32 ptIdx;
            double norm = static_cast<double>(captureBuffer_.count()) / MidiCaptureBuffer::kCapacity;
            countQueue->addPoint(0, norm, ptIdx);
        }
    }
}

Steinberg::tresult PLUGIN_API PolyProcessor::process(Steinberg::Vst::ProcessData& data) {
    if (data.numOutputs > 0) {
        for (Steinberg::int32 ch = 0; ch < data.outputs[0].numChannels; ++ch) {
            if (data.outputs[0].channelBuffers32[ch]) {
                std::memset(data.outputs[0].channelBuffers32[ch], 0,
                            static_cast<size_t>(data.numSamples) * sizeof(float));
            }
        }
        data.outputs[0].silenceFlags = 0x3;
    }

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

    if (stateReady_.load(std::memory_order_acquire)) {
        sceneState_ = pendingState_;
        stateReady_.store(false, std::memory_order_release);
    }
    if (noteMapReady_.load(std::memory_order_acquire)) {
        sceneState_.noteMap = pendingNoteMap_;
        noteMapReady_.store(false, std::memory_order_release);
    }
    auto& activeScene = (sceneState_.select == SceneSelect::B) ? sceneState_.sceneB : sceneState_.sceneA;
    if (cellSizesReady_.load(std::memory_order_acquire)) {
        auto& lane = activeScene.lanes[pendingCellSizes_.laneIndex];
        lane.cellSizes = pendingCellSizes_.sizes;
        cellSizesReady_.store(false, std::memory_order_release);
    }
    if (timelineReady_.load(std::memory_order_acquire)) {
        auto& lane = activeScene.lanes[pendingTimeline_.laneIndex];
        lane.fixedPattern = pendingTimeline_.pattern;
        lane.fixedPatternLength = pendingTimeline_.patternLength;
        timelineReady_.store(false, std::memory_order_release);
    }
    if (microTimingReady_.load(std::memory_order_acquire)) {
        auto& lane = activeScene.lanes[pendingMicroTiming_.laneIndex];
        lane.microTimingMs = pendingMicroTiming_.timingMs;
        microTimingReady_.store(false, std::memory_order_release);
    }
    if (envelopeReady_.load(std::memory_order_acquire)) {
        auto& lane = activeScene.lanes[pendingEnvelope_.laneIndex];
        lane.envelopes[pendingEnvelope_.envelopeIndex].envelope = pendingEnvelope_.envelope;
        lane.envelopes[pendingEnvelope_.envelopeIndex].active = pendingEnvelope_.active;
        envelopeReady_.store(false, std::memory_order_release);
    }
    if (accentMaskReady_.load(std::memory_order_acquire)) {
        auto& lane = activeScene.lanes[pendingAccentMask_.laneIndex];
        lane.accents.steps = pendingAccentMask_.steps;
        accentMaskReady_.store(false, std::memory_order_release);
    }

    updateTransportContext(data);

    if (!tc_.playing) {
        uiSnapshot_.playing.store(false, std::memory_order_relaxed);
        if (!uiSnapshot_.stateReady.load(std::memory_order_acquire)) {
            uiSnapshot_.state = sceneState_;
            uiSnapshot_.stateReady.store(true, std::memory_order_release);
        }
        // P1 fix: republish stateSnapshot_ unconditionally so host-thread getState()
        // sees post-stop edits. Pending notify payloads already drained into sceneState_
        // above (see stateReady_/noteMapReady_/envelopeReady_/etc handlers at 344-379).
        // Unconditional (unlike the playing-path guard below): transport is idle so we
        // are not racing another publish. A stale ready-but-unconsumed snapshot is
        // exactly the P1 hazard — overwriting it with fresher data is the fix.
        // A concurrent getState() read is the P2 torn-read hazard, addressed in T03 by
        // removing the racy fallback and publishing an initial snapshot from setActive.
        stateSnapshot_ = sceneState_;
        snapshotReady_.store(true, std::memory_order_release);
        if (wasPlaying_ && pendingNoteOffs_.count() > 0 && data.outputEvents) {
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
            pendingNoteOffs_.clear();
        }
        if (exportTriggered_ && !exportReady_.load(std::memory_order_acquire)) {
            exportTriggered_ = false;
            exportEventCount_ =
                captureBuffer_.extractLastBars(captureLengthBars_, exportEvents_.data(), exportEvents_.size());
            exportTempo_ = tc_.tempo;
            exportReady_.store(true, std::memory_order_release);
        }
        wasPlaying_ = false;
        return Steinberg::kResultOk;
    }
    wasPlaying_ = true;

    handleTransportJump(data.outputEvents);

    if (sceneState_.chain.enabled && sceneState_.chain.entryCount > 0) {
        if (tc_.jumped)
            chainState_.reset();
        sceneState_.select = chainState_.update(sceneState_.chain, tc_.ppqStart);
    }

    // region:process-render
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
    // endregion:process-render

    if (!data.outputEvents)
        return Steinberg::kResultOk;

    emitMidiOutput(data.outputEvents, data.numSamples);

    for (size_t i = 0; i < noteBuffer_.count; ++i) {
        auto mapped = noteBuffer_.events[i];
        mapped.pitch = sceneState_.noteMap.apply(mapped.pitch);
        captureBuffer_.push(mapped);
    }

    if (exportTriggered_ && !exportReady_.load(std::memory_order_acquire)) {
        exportTriggered_ = false;
        exportEventCount_ =
            captureBuffer_.extractLastBars(captureLengthBars_, exportEvents_.data(), exportEvents_.size());
        exportTempo_ = tc_.tempo;
        exportReady_.store(true, std::memory_order_release);
    }

    outputParameterFeedback(data, resolved);

    // State-snapshot publish: unconditionally republish the latest sceneState_ each
    // playing block. Matches T02's stopped-path semantics — state serialization needs
    // the LATEST scene, not the last-unconsumed one. The T03 setActive(true) initial
    // publish left snapshotReady_ = true, so guarding this on !snapshotReady_ meant
    // scene edits during playback never reached stateSnapshot_ (M046 S01 T05 fix;
    // pluginval regression on PR #80 caught this). The theoretical torn-read window
    // on stateSnapshot_ (writer struct-copy racing reader struct-copy) is the same
    // P2 hazard the pre-T03 fallback carried on sceneState_ — M046 S03 T02 (P4) will
    // replace this with a 2-slot exchange as the durable fix.
    stateSnapshot_ = sceneState_;
    snapshotReady_.store(true, std::memory_order_release);

    if (!uiSnapshot_.stateReady.load(std::memory_order_acquire)) {
        uiSnapshot_.state = sceneState_;
        uiSnapshot_.stateReady.store(true, std::memory_order_release);
    }

    return Steinberg::kResultOk;
}

bool PolyProcessor::applySceneParameter(Steinberg::Vst::ParamID id, double normalized) {
    using namespace ParamIDs;

    if (id == kSceneSelect) {
        int sel = static_cast<int>(std::round(normalized * 2.0));
        sceneState_.select = static_cast<SceneSelect>(std::clamp(sel, 0, 2));
        return true;
    }
    if (id == kSceneMorph) {
        sceneState_.morphAmount = static_cast<float>(normalized);
        return true;
    }
    if (id == kChainEnabled) {
        bool wasEnabled = sceneState_.chain.enabled;
        sceneState_.chain.enabled = (normalized > 0.5);
        if (sceneState_.chain.enabled && !wasEnabled)
            chainState_.reset();
        return true;
    }
    if (id == kChainMode) {
        int m = static_cast<int>(std::round(normalized * 2.0));
        sceneState_.chain.mode = static_cast<ChainMode>(std::clamp(m, 0, 2));
        return true;
    }
    if (id == kChainEntryCount) {
        sceneState_.chain.entryCount = static_cast<int>(std::round(normalized * static_cast<double>(kMaxChainEntries)));
        return true;
    }
    if (id >= kChainEntryBase &&
        id < kChainEntryBase + static_cast<Steinberg::Vst::ParamID>(kMaxChainEntries * kChainParamsPerEntry)) {
        auto rel = static_cast<int>(id - kChainEntryBase);
        int entry = rel / kChainParamsPerEntry;
        int offset = rel % kChainParamsPerEntry;
        if (entry < kMaxChainEntries) {
            auto& e = sceneState_.chain.entries[static_cast<size_t>(entry)];
            if (offset == kChainEntryScene) {
                int sel = static_cast<int>(std::round(normalized * 2.0));
                e.scene = static_cast<SceneSelect>(std::clamp(sel, 0, 2));
            } else if (offset == kChainEntryBars) {
                e.bars = 1 + static_cast<int>(std::round(normalized * 31.0));
            }
        }
        return true;
    }
    return false;
}

static bool applyCoreParam(Steinberg::Vst::ParamID id, double normalized, GrooveState& gs) {
    using namespace ParamIDs;
    if (id < kLaneCoreBase ||
        id >= kLaneCoreBase + static_cast<Steinberg::Vst::ParamID>(kMaxLanes * kCoreParamsPerLane))
        return false;

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
    case kCoreTimeline:
        cfg.timeline = normalized > 0.5;
        break;
    case kCoreFixedPatternLen:
        cfg.fixedPatternLength = static_cast<int>(std::round(normalized * 64.0));
        break;
    case kCoreTempoMult:
        cfg.tempoMultiplier = static_cast<float>(0.25 + normalized * 3.75);
        break;
    case kCoreMidiChannel:
        cfg.midiChannel = static_cast<int16_t>(std::round(normalized * 16.0)) - 1;
        break;
    default:
        break;
    }
    return true;
}

static bool applyExpressionParam(Steinberg::Vst::ParamID id, double normalized, GrooveState& gs) {
    using namespace ParamIDs;
    if (id >= static_cast<Steinberg::Vst::ParamID>(kMaxLanes * kParamsPerLane))
        return false;

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
    return true;
}

bool PolyProcessor::applyLaneParameter(Steinberg::Vst::ParamID id, double normalized, GrooveState& gs) {
    return applyCoreParam(id, normalized, gs) || applyExpressionParam(id, normalized, gs);
}

void PolyProcessor::applyParameter(Steinberg::Vst::ParamID id, double normalized) {
    using namespace ParamIDs;

    if (applySceneParameter(id, normalized))
        return;

    auto& gs = (sceneState_.select == SceneSelect::B) ? sceneState_.sceneB : sceneState_.sceneA;

    if (applyLaneParameter(id, normalized, gs))
        return;

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

Steinberg::tresult PLUGIN_API PolyProcessor::notify(Steinberg::Vst::IMessage* message) {
    if (!message)
        return Steinberg::kInvalidArgument;

    if (Steinberg::FIDStringsEqual(message->getMessageID(), "NoteMapUpdate")) {
        if (auto* attrs = message->getAttributes()) {
            const void* data = nullptr;
            Steinberg::uint32 size = 0;
            if (attrs->getBinary("map", data, size) == Steinberg::kResultOk && size == sizeof(pendingNoteMap_.map)) {
                std::memcpy(pendingNoteMap_.map.data(), data, size);
                noteMapReady_.store(true, std::memory_order_release);
            }
        }
        return Steinberg::kResultOk;
    }

    if (Steinberg::FIDStringsEqual(message->getMessageID(), "CellSizesUpdate")) {
        if (auto* attrs = message->getAttributes()) {
            Steinberg::int64 laneIndex = 0;
            const void* data = nullptr;
            Steinberg::uint32 size = 0;
            if (attrs->getInt("lane", laneIndex) == Steinberg::kResultOk &&
                attrs->getBinary("sizes", data, size) == Steinberg::kResultOk && laneIndex >= 0 &&
                laneIndex < kMaxLanes && size == sizeof(pendingCellSizes_.sizes)) {
                pendingCellSizes_.laneIndex = static_cast<int>(laneIndex);
                std::memcpy(pendingCellSizes_.sizes.data(), data, size);
                cellSizesReady_.store(true, std::memory_order_release);
            }
        }
        return Steinberg::kResultOk;
    }

    if (Steinberg::FIDStringsEqual(message->getMessageID(), "TimelinePatternUpdate")) {
        if (auto* attrs = message->getAttributes()) {
            Steinberg::int64 laneIndex = 0;
            Steinberg::int64 patLen = 0;
            const void* data = nullptr;
            Steinberg::uint32 size = 0;
            if (attrs->getInt("lane", laneIndex) == Steinberg::kResultOk &&
                attrs->getInt("patLen", patLen) == Steinberg::kResultOk &&
                attrs->getBinary("pattern", data, size) == Steinberg::kResultOk && laneIndex >= 0 &&
                laneIndex < kMaxLanes && size == sizeof(pendingTimeline_.pattern)) {
                pendingTimeline_.laneIndex = static_cast<int>(laneIndex);
                pendingTimeline_.patternLength = static_cast<int>(patLen);
                std::memcpy(pendingTimeline_.pattern.data(), data, size);
                timelineReady_.store(true, std::memory_order_release);
            }
        }
        return Steinberg::kResultOk;
    }

    if (Steinberg::FIDStringsEqual(message->getMessageID(), "MicroTimingUpdate")) {
        if (auto* attrs = message->getAttributes()) {
            Steinberg::int64 laneIndex = 0;
            const void* data = nullptr;
            Steinberg::uint32 size = 0;
            if (attrs->getInt("lane", laneIndex) == Steinberg::kResultOk &&
                attrs->getBinary("timing", data, size) == Steinberg::kResultOk && laneIndex >= 0 &&
                laneIndex < kMaxLanes && size == sizeof(pendingMicroTiming_.timingMs)) {
                pendingMicroTiming_.laneIndex = static_cast<int>(laneIndex);
                std::memcpy(pendingMicroTiming_.timingMs.data(), data, size);
                microTimingReady_.store(true, std::memory_order_release);
            }
        }
        return Steinberg::kResultOk;
    }

    if (Steinberg::FIDStringsEqual(message->getMessageID(), "EnvelopeUpdate")) {
        if (auto* attrs = message->getAttributes()) {
            Steinberg::int64 laneIndex = 0;
            Steinberg::int64 envIdx = 0;
            Steinberg::int64 activeVal = 1;
            const void* data = nullptr;
            Steinberg::uint32 size = 0;
            if (attrs->getInt("lane", laneIndex) == Steinberg::kResultOk &&
                attrs->getInt("envIdx", envIdx) == Steinberg::kResultOk &&
                attrs->getBinary("envelope", data, size) == Steinberg::kResultOk && laneIndex >= 0 &&
                laneIndex < kMaxLanes && envIdx >= 0 && envIdx < kMaxEnvelopesPerLane &&
                size == sizeof(pendingEnvelope_.envelope)) {
                pendingEnvelope_.laneIndex = static_cast<int>(laneIndex);
                pendingEnvelope_.envelopeIndex = static_cast<int>(envIdx);
                std::memcpy(&pendingEnvelope_.envelope, data, size);
                attrs->getInt("active", activeVal);
                pendingEnvelope_.active = (activeVal != 0);
                envelopeReady_.store(true, std::memory_order_release);
            }
        }
        return Steinberg::kResultOk;
    }

    if (Steinberg::FIDStringsEqual(message->getMessageID(), "AccentMaskUpdate")) {
        if (auto* attrs = message->getAttributes()) {
            Steinberg::int64 laneIndex = 0;
            const void* data = nullptr;
            Steinberg::uint32 size = 0;
            if (attrs->getInt("lane", laneIndex) == Steinberg::kResultOk &&
                attrs->getBinary("accents", data, size) == Steinberg::kResultOk && laneIndex >= 0 &&
                laneIndex < kMaxLanes && size == sizeof(pendingAccentMask_.steps)) {
                pendingAccentMask_.laneIndex = static_cast<int>(laneIndex);
                std::memcpy(pendingAccentMask_.steps.data(), data, size);
                accentMaskReady_.store(true, std::memory_order_release);
            }
        }
        return Steinberg::kResultOk;
    }

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

// region:get-set-state
// Invariant: stateSnapshot_ is publishable after setActive(true) — see processor.cpp setActive; audit trail M046 S01
// T03
Steinberg::tresult PLUGIN_API PolyProcessor::getState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kInvalidArgument;

    auto write = [state](const void* data, size_t size) -> bool {
        Steinberg::int32 written;
        return state->write(const_cast<void*>(data), static_cast<Steinberg::int32>(size), &written) ==
               Steinberg::kResultOk;
    };

    if (!snapshotReady_.load(std::memory_order_acquire))
        return Steinberg::kResultFalse;
    auto result = writeSceneState(write, stateSnapshot_) ? Steinberg::kResultOk : Steinberg::kResultFalse;
    snapshotReady_.store(false, std::memory_order_release);
    return result;
}

Steinberg::tresult PLUGIN_API PolyProcessor::setState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kInvalidArgument;

    auto read = [state](void* data, size_t size) -> bool {
        Steinberg::int32 bytesRead;
        return state->read(data, static_cast<Steinberg::int32>(size), &bytesRead) == Steinberg::kResultOk;
    };

    if (!readSceneState(read, pendingState_))
        return Steinberg::kResultFalse;
    stateReady_.store(true, std::memory_order_release);
    return Steinberg::kResultOk;
}
// endregion:get-set-state

} // namespace poly
