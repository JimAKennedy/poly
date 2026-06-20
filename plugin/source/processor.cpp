#include "processor.h"
#include "plugids.h"

#include "poly/macro.h"
#include "poly/state_io.h"

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace poly {

PolyProcessor::PolyProcessor() {
    setControllerClass(kPolyControllerUID);
}

Steinberg::tresult PLUGIN_API PolyProcessor::initialize(
    Steinberg::FUnknown* context) {
    auto result = AudioEffect::initialize(context);
    if (result != Steinberg::kResultOk)
        return result;

    addAudioOutput(STR16("Stereo Out"),
                   Steinberg::Vst::SpeakerArr::kStereo);
    addEventOutput(STR16("MIDI Out"), 1);

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API PolyProcessor::setActive(
    Steinberg::TBool state) {
    if (state) {
        pendingNoteOffs_.clear();
        noteBuffer_.clear();
        expectedNextPpq_ = -1.0;
    }
    return AudioEffect::setActive(state);
}

Steinberg::tresult PLUGIN_API PolyProcessor::process(
    Steinberg::Vst::ProcessData& data) {

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
            if (!queue) continue;
            Steinberg::Vst::ParamValue value;
            Steinberg::int32 sampleOffset;
            Steinberg::int32 pointCount = queue->getPointCount();
            if (pointCount > 0 &&
                queue->getPoint(pointCount - 1, sampleOffset, value) ==
                    Steinberg::kResultOk) {
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
        tc_.playing =
            (ctx.state & Steinberg::Vst::ProcessContext::kPlaying) != 0;
        tc_.looping =
            (ctx.state & Steinberg::Vst::ProcessContext::kCycleActive) != 0;

        if (ctx.state &
            Steinberg::Vst::ProcessContext::kProjectTimeMusicValid) {
            double ppqStart = ctx.projectTimeMusic;
            double beatsPerSample =
                tc_.tempo > 0.0 ? tc_.tempo / (60.0 * tc_.sampleRate) : 0.0;
            tc_.ppqStart = ppqStart;
            tc_.ppqEnd = ppqStart + data.numSamples * beatsPerSample;

            constexpr double kJumpThreshold = 0.001;
            tc_.jumped = (expectedNextPpq_ >= 0.0 &&
                          std::abs(ppqStart - expectedNextPpq_) >
                              kJumpThreshold);
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

    if (!tc_.playing) return Steinberg::kResultOk;

    // --- Handle transport jump: kill pending noteoffs ---
    if (tc_.jumped) {
        if (data.outputEvents) {
            PendingNoteOff allOffs[PendingNoteOffBuffer::kCapacity];
            size_t n = pendingNoteOffs_.flushDue(
                -1e12, 1e12, allOffs, PendingNoteOffBuffer::kCapacity);
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

    // --- Resolve macros and render ---
    GrooveState resolved = resolveMacros(grooveState_);
    engine_.renderRange(tc_, resolved, noteBuffer_);

    // --- Emit MIDI events ---
    auto* outputEvents = data.outputEvents;
    if (!outputEvents) return Steinberg::kResultOk;

    // Flush due noteoffs
    PendingNoteOff dueOffs[kMaxEventsPerBlock];
    size_t numDue = pendingNoteOffs_.flushDue(
        tc_.ppqStart, tc_.ppqEnd, dueOffs, kMaxEventsPerBlock);

    for (size_t i = 0; i < numDue; ++i) {
        Steinberg::Vst::Event ev = {};
        ev.busIndex = 0;
        ev.sampleOffset = ppqToSampleOffset(
            dueOffs[i].ppqOff, tc_.ppqStart, tc_.tempo, tc_.sampleRate,
            data.numSamples);
        ev.ppqPosition = dueOffs[i].ppqOff;
        ev.type = Steinberg::Vst::Event::kNoteOffEvent;
        ev.noteOff.channel = dueOffs[i].channel;
        ev.noteOff.pitch = dueOffs[i].pitch;
        ev.noteOff.velocity = 0.0f;
        ev.noteOff.noteId = -1;
        outputEvents->addEvent(ev);
    }

    // Emit noteons and schedule noteoffs
    for (size_t i = 0; i < noteBuffer_.count; ++i) {
        const auto& note = noteBuffer_.events[i];

        Steinberg::Vst::Event ev = {};
        ev.busIndex = 0;
        ev.sampleOffset = ppqToSampleOffset(
            note.ppqPosition, tc_.ppqStart, tc_.tempo, tc_.sampleRate,
            data.numSamples);
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
    }

    return Steinberg::kResultOk;
}

void PolyProcessor::applyParameter(Steinberg::Vst::ParamID id,
                                    double normalized) {
    using namespace ParamIDs;

    if (id < static_cast<Steinberg::Vst::ParamID>(kMaxLanes * kParamsPerLane)) {
        int lane = static_cast<int>(id) / kParamsPerLane;
        int offset = static_cast<int>(id) % kParamsPerLane;
        auto& cfg = grooveState_.lanes[lane];

        switch (offset) {
        case kProbability:
            cfg.probability = static_cast<float>(normalized);
            break;
        case kBaseVelocity:
            cfg.baseVelocity =
                static_cast<uint8_t>(std::round(normalized * 127.0));
            break;
        case kEmphasisProb:
            cfg.emphasisProb = static_cast<float>(normalized);
            break;
        case kGhostFloor:
            cfg.ghostFloor =
                static_cast<uint8_t>(std::round(normalized * 127.0));
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
        default:
            break;
        }
    } else {
        switch (id) {
        case kMacroComplexity:
            grooveState_.macros.complexity = static_cast<float>(normalized);
            break;
        case kMacroDensity:
            grooveState_.macros.density = static_cast<float>(normalized);
            break;
        case kMacroSyncopation:
            grooveState_.macros.syncopation = static_cast<float>(normalized);
            break;
        case kMacroSwing:
            grooveState_.macros.swing = static_cast<float>(normalized);
            break;
        case kMacroTension:
            grooveState_.macros.tension = static_cast<float>(normalized);
            break;
        case kMacroHumanize:
            grooveState_.macros.humanize = static_cast<float>(normalized);
            break;
        case kActiveLaneCount:
            grooveState_.activeLaneCount =
                1 + static_cast<int>(std::round(normalized * 7.0));
            break;
        case kSeed:
            grooveState_.seed =
                static_cast<uint64_t>(std::round(normalized * 999999.0));
            break;
        default:
            break;
        }
    }
}

Steinberg::tresult PLUGIN_API PolyProcessor::getState(
    Steinberg::IBStream* state) {
    if (!state) return Steinberg::kInvalidArgument;

    auto write = [state](const void* data, size_t size) -> bool {
        Steinberg::int32 written;
        return state->write(const_cast<void*>(data),
                            static_cast<Steinberg::int32>(size),
                            &written) == Steinberg::kResultOk;
    };

    return writeGrooveState(write, grooveState_) ? Steinberg::kResultOk
                                                 : Steinberg::kResultFalse;
}

Steinberg::tresult PLUGIN_API PolyProcessor::setState(
    Steinberg::IBStream* state) {
    if (!state) return Steinberg::kInvalidArgument;

    auto read = [state](void* data, size_t size) -> bool {
        Steinberg::int32 bytesRead;
        return state->read(data, static_cast<Steinberg::int32>(size),
                           &bytesRead) == Steinberg::kResultOk;
    };

    return readGrooveState(read, grooveState_) ? Steinberg::kResultOk
                                               : Steinberg::kResultFalse;
}

} // namespace poly
