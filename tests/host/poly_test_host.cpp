#include "poly_test_host.h"

#include <algorithm>
#include <cstring>

#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"

#include "processor.h"

void* moduleHandle = nullptr;

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace poly {
namespace test {

static HostApplication sHostApp;

PolyTestHost::PolyTestHost() = default;

PolyTestHost::~PolyTestHost() {
    if (active_)
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
        processor_->terminate();
        processor_->release();
        processor_ = nullptr;
        return false;
    }

    active_ = true;
    return true;
}

void PolyTestHost::teardown() {
    if (!processor_)
        return;
    if (active_)
        processor_->setActive(false);
    processor_->terminate();
    processor_->release();
    processor_ = nullptr;
    active_ = false;
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

} // namespace test
} // namespace poly
