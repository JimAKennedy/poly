#include "poly_test_host.h"

#include <algorithm>
#include <cstring>

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"

#include "controller_base.h"
#include "processor.h"

void* moduleHandle = nullptr;

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace poly {
namespace test {

static HostApplication sHostApp;

PolyTestHost::PolyTestHost() = default;

PolyTestHost::~PolyTestHost() {
    if (active_ || processor_ || controller_)
        teardown();
}

bool PolyTestHost::setup(double sampleRate, int blockSize) {
    auto* proc = new PolyProcessor(); // ownership-transfer
    processor_ = proc;

    if (processor_->initialize(&sHostApp) != kResultOk) {
        processor_->release();
        processor_ = nullptr;
        return false;
    }

    auto* ctrl = new poly::PolyControllerBase(); // ownership-transfer
    controller_ = ctrl;
    if (controller_->initialize(&sHostApp) != kResultOk) {
        controller_->release();
        controller_ = nullptr;
        processor_->terminate();
        processor_->release();
        processor_ = nullptr;
        return false;
    }

    sampleRate_ = sampleRate;
    blockSize_ = blockSize;

    ProcessSetup ps{};
    ps.processMode = kRealtime;
    ps.symbolicSampleSize = kSample32;
    ps.maxSamplesPerBlock = blockSize;
    ps.sampleRate = sampleRate;
    processor_->setupProcessing(ps);

    leftBuf_.resize(static_cast<size_t>(blockSize), 0.0f);
    rightBuf_.resize(static_cast<size_t>(blockSize), 0.0f);

    if (processor_->setActive(true) != kResultOk) {
        controller_->terminate();
        controller_->release();
        controller_ = nullptr;
        processor_->terminate();
        processor_->release();
        processor_ = nullptr;
        return false;
    }

    active_ = true;
    return true;
}

void PolyTestHost::teardown() {
    if (processor_) {
        if (active_)
            processor_->setActive(false);
        processor_->terminate();
        processor_->release();
        processor_ = nullptr;
    }
    if (controller_) {
        controller_->terminate();
        controller_->release();
        controller_ = nullptr;
    }
    active_ = false;
    pendingParamChanges_.clear();
}

double PolyTestHost::ppqPerBlock(double tempo) const {
    return blockSize_ * tempo / (60.0 * sampleRate_);
}

void PolyTestHost::processBlock(double ppqStart, double tempo, bool playing, bool looping, double loopStart,
                                double loopEnd) {
    if (!processor_ || !active_)
        return;

    ProcessContext ctx{};
    ctx.state = ProcessContext::kProjectTimeMusicValid | ProcessContext::kTempoValid;
    if (playing)
        ctx.state |= ProcessContext::kPlaying;
    if (looping) {
        ctx.state |= ProcessContext::kCycleActive | ProcessContext::kCycleValid;
        ctx.cycleStartMusic = loopStart;
        ctx.cycleEndMusic = loopEnd;
    }
    ctx.sampleRate = sampleRate_;
    ctx.projectTimeMusic = ppqStart;
    ctx.tempo = tempo;

    std::fill(leftBuf_.begin(), leftBuf_.end(), 0.0f);
    std::fill(rightBuf_.begin(), rightBuf_.end(), 0.0f);
    float* outChannels[2] = {leftBuf_.data(), rightBuf_.data()};
    AudioBusBuffers audioOut{};
    audioOut.numChannels = 2;
    audioOut.silenceFlags = 0;
    audioOut.channelBuffers32 = outChannels;

    EventList outputEvents;
    ParameterChanges inputParams;
    ParameterChanges outputParams;

    // Drain any pending param changes queued via injectParamChangeThroughProcess.
    for (const auto& [paramId, value] : pendingParamChanges_) {
        int32 queueIdx = 0;
        auto* queue = inputParams.addParameterData(static_cast<ParamID>(paramId), queueIdx);
        if (queue) {
            int32 pointIdx = 0;
            queue->addPoint(0, value, pointIdx);
        }
    }
    pendingParamChanges_.clear();

    ProcessData data{};
    data.processMode = kRealtime;
    data.symbolicSampleSize = kSample32;
    data.numSamples = blockSize_;
    data.numInputs = 0;
    data.inputs = nullptr;
    data.numOutputs = 1;
    data.outputs = &audioOut;
    data.inputEvents = nullptr;
    data.outputEvents = &outputEvents;
    data.inputParameterChanges = &inputParams;
    data.outputParameterChanges = &outputParams;
    data.processContext = &ctx;

    processor_->process(data);

    int32 eventCount = outputEvents.getEventCount();
    for (int32 i = 0; i < eventCount; ++i) {
        Event ev{};
        if (outputEvents.getEvent(i, ev) != kResultOk)
            continue;
        if (ev.type == Event::kNoteOnEvent) {
            events_.push_back({MidiEvent::NoteOn, ev.ppqPosition, ev.noteOn.pitch, ev.noteOn.velocity,
                               ev.noteOn.channel, ev.sampleOffset});
        } else if (ev.type == Event::kNoteOffEvent) {
            events_.push_back({MidiEvent::NoteOff, ev.ppqPosition, ev.noteOff.pitch, ev.noteOff.velocity,
                               ev.noteOff.channel, ev.sampleOffset});
        }
    }
}

void PolyTestHost::playBars(double bars, double tempo) {
    double totalPpq = bars * 4.0;
    double step = ppqPerBlock(tempo);
    double ppq = 0.0;
    while (ppq < totalPpq) {
        processBlock(ppq, tempo, true);
        ppq += step;
    }
}

void PolyTestHost::stopAndFlush(double ppqPos, double tempo) {
    processBlock(ppqPos, tempo, false);
}

std::vector<MidiEvent> PolyTestHost::noteOnEvents() const {
    std::vector<MidiEvent> result;
    for (const auto& e : events_)
        if (e.type == MidiEvent::NoteOn)
            result.push_back(e);
    return result;
}

std::vector<MidiEvent> PolyTestHost::noteOffEvents() const {
    std::vector<MidiEvent> result;
    for (const auto& e : events_)
        if (e.type == MidiEvent::NoteOff)
            result.push_back(e);
    return result;
}

std::vector<uint8_t> PolyTestHost::saveState() {
    if (!processor_)
        return {};
    MemoryStream stream;
    if (processor_->getState(&stream) != kResultOk)
        return {};
    auto size = stream.getSize();
    if (size <= 0 || !stream.getData())
        return {};
    std::vector<uint8_t> buf(static_cast<size_t>(size));
    std::memcpy(buf.data(), stream.getData(), static_cast<size_t>(size));
    return buf;
}

bool PolyTestHost::loadState(const std::vector<uint8_t>& bytes) {
    if (!processor_ || bytes.empty())
        return false;
    // Push the bytes through processor->setState() first so sceneState_ is refreshed.
    MemoryStream procStream(const_cast<uint8_t*>(bytes.data()), static_cast<TSize>(bytes.size()));
    if (processor_->setState(&procStream) != kResultOk)
        return false;
    // Then, if a controller is attached, hand the same bytes to setComponentState so its
    // param cache mirrors the restored processor state — this is what the DAW does on
    // preset restore, and it is what pluginval verifies.
    if (controller_) {
        MemoryStream ctrlStream(const_cast<uint8_t*>(bytes.data()), static_cast<TSize>(bytes.size()));
        if (controller_->setComponentState(&ctrlStream) != kResultOk)
            return false;
    }
    return true;
}

void PolyTestHost::injectNoteMap(const std::array<int16_t, 128>& map) {
    if (!processor_)
        return;
    auto* msg = new HostMessage; // ownership-transfer to local refcount
    msg->setMessageID("NoteMapUpdate");
    if (auto* attrs = msg->getAttributes()) {
        attrs->setBinary("map", map.data(), static_cast<uint32>(sizeof(int16_t) * map.size()));
    }
    processor_->notify(msg);
    msg->release();
}

bool PolyTestHost::setParamOnController(uint32_t paramId, double normalizedValue) {
    if (!controller_)
        return false;
    return controller_->setParamNormalized(static_cast<ParamID>(paramId), normalizedValue) == kResultOk;
}

double PolyTestHost::getParamOnController(uint32_t paramId) const {
    if (!controller_)
        return -1.0;
    return controller_->getParamNormalized(static_cast<ParamID>(paramId));
}

void PolyTestHost::injectParamChangeThroughProcess(uint32_t paramId, double normalizedValue) {
    pendingParamChanges_.emplace_back(paramId, normalizedValue);
}

} // namespace test
} // namespace poly
