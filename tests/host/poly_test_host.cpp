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

#include "base/source/fobject.h"
#include "controller_base.h"
#include "poly/scene.h"
#include "poly/state_io.h"
#include "processor.h"
#include "ui_snapshot.h"

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

    // Wire processor <-> controller before setActive so PolyProcessor::connect() can dispatch
    // the UISnapshotPtr message that populates the controller's uiSnapshot_ cache.
    if (!connectComponents()) {
        controller_->terminate();
        controller_->release();
        controller_ = nullptr;
        processor_->terminate();
        processor_->release();
        processor_ = nullptr;
        return false;
    }

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
    disconnectComponents();
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

bool PolyTestHost::connectComponents() {
    if (!processor_ || !controller_)
        return false;
    FUnknownPtr<IConnectionPoint> procCp(processor_->unknownCast());
    FUnknownPtr<IConnectionPoint> ctrlCp(controller_->unknownCast());
    if (!procCp || !ctrlCp)
        return false;
    if (procCp->connect(ctrlCp) != kResultOk)
        return false;
    if (ctrlCp->connect(procCp) != kResultOk)
        return false;
    return true;
}

void PolyTestHost::disconnectComponents() {
    if (!processor_ || !controller_)
        return;
    FUnknownPtr<IConnectionPoint> procCp(processor_->unknownCast());
    FUnknownPtr<IConnectionPoint> ctrlCp(controller_->unknownCast());
    if (procCp && ctrlCp) {
        procCp->disconnect(ctrlCp);
        ctrlCp->disconnect(procCp);
    }
}

poly::UISnapshot* PolyTestHost::controllerUiSnapshot() const {
    return controller_ ? controller_->uiSnapshot() : nullptr;
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

std::vector<uint8_t> PolyTestHost::saveFullPluginState() {
    if (!processor_)
        return {};

    MemoryStream procStream;
    if (processor_->getState(&procStream) != kResultOk)
        return {};
    auto procSize = procStream.getSize();
    if (procSize < 0)
        return {};

    MemoryStream ctrlStream;
    int64 ctrlSize = 0;
    if (controller_) {
        if (controller_->getState(&ctrlStream) != kResultOk)
            return {};
        ctrlSize = ctrlStream.getSize();
        if (ctrlSize < 0)
            return {};
    }

    std::vector<uint8_t> buf;
    buf.reserve(sizeof(uint32_t) + static_cast<size_t>(procSize) + sizeof(uint32_t) + static_cast<size_t>(ctrlSize));

    auto appendU32 = [&buf](uint32_t v) {
        for (size_t i = 0; i < sizeof(v); ++i)
            buf.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFFu));
    };

    appendU32(static_cast<uint32_t>(procSize));
    if (procSize > 0 && procStream.getData()) {
        auto* p = reinterpret_cast<const uint8_t*>(procStream.getData());
        buf.insert(buf.end(), p, p + procSize);
    }
    appendU32(static_cast<uint32_t>(ctrlSize));
    if (ctrlSize > 0 && ctrlStream.getData()) {
        auto* p = reinterpret_cast<const uint8_t*>(ctrlStream.getData());
        buf.insert(buf.end(), p, p + ctrlSize);
    }
    return buf;
}

bool PolyTestHost::loadFullPluginState(const std::vector<uint8_t>& bytes) {
    if (!processor_ || bytes.size() < sizeof(uint32_t))
        return false;

    auto readU32 = [&bytes](size_t offset) -> uint32_t {
        uint32_t v = 0;
        for (size_t i = 0; i < sizeof(v); ++i)
            v |= static_cast<uint32_t>(bytes[offset + i]) << (i * 8);
        return v;
    };

    uint32_t procLen = readU32(0);
    size_t cursor = sizeof(uint32_t);
    if (bytes.size() < cursor + procLen + sizeof(uint32_t))
        return false;
    const uint8_t* procData = bytes.data() + cursor;
    cursor += procLen;

    uint32_t ctrlLen = readU32(cursor);
    cursor += sizeof(uint32_t);
    if (bytes.size() < cursor + ctrlLen)
        return false;
    const uint8_t* ctrlData = bytes.data() + cursor;

    MemoryStream procStream(const_cast<uint8_t*>(procData), static_cast<TSize>(procLen));
    if (processor_->setState(&procStream) != kResultOk)
        return false;

    if (controller_) {
        MemoryStream compStream(const_cast<uint8_t*>(procData), static_cast<TSize>(procLen));
        if (controller_->setComponentState(&compStream) != kResultOk)
            return false;
        if (ctrlLen > 0) {
            MemoryStream ctrlStream(const_cast<uint8_t*>(ctrlData), static_cast<TSize>(ctrlLen));
            if (controller_->setState(&ctrlStream) != kResultOk)
                return false;
        }
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

void PolyTestHost::injectCellSizes(int laneIndex, const std::array<int, poly::kMaxSteps>& sizes) {
    if (!processor_)
        return;
    auto* msg = new HostMessage; // ownership-transfer to local refcount
    msg->setMessageID("CellSizesUpdate");
    if (auto* attrs = msg->getAttributes()) {
        attrs->setInt("lane", laneIndex);
        attrs->setBinary("sizes", sizes.data(), static_cast<uint32>(sizeof(int) * sizes.size()));
    }
    processor_->notify(msg);
    msg->release();
}

void PolyTestHost::injectTimelinePattern(int laneIndex, const std::array<bool, poly::kMaxSteps>& pattern,
                                         int patternLength) {
    if (!processor_)
        return;
    auto* msg = new HostMessage; // ownership-transfer to local refcount
    msg->setMessageID("TimelinePatternUpdate");
    if (auto* attrs = msg->getAttributes()) {
        attrs->setInt("lane", laneIndex);
        attrs->setInt("patLen", patternLength);
        attrs->setBinary("pattern", pattern.data(), static_cast<uint32>(sizeof(bool) * pattern.size()));
    }
    processor_->notify(msg);
    msg->release();
}

void PolyTestHost::injectMicroTiming(int laneIndex, const std::array<float, poly::kMaxSteps>& timingMs) {
    if (!processor_)
        return;
    auto* msg = new HostMessage; // ownership-transfer to local refcount
    msg->setMessageID("MicroTimingUpdate");
    if (auto* attrs = msg->getAttributes()) {
        attrs->setInt("lane", laneIndex);
        attrs->setBinary("timing", timingMs.data(), static_cast<uint32>(sizeof(float) * timingMs.size()));
    }
    processor_->notify(msg);
    msg->release();
}

void PolyTestHost::injectEnvelope(int laneIndex, int envelopeIndex, bool active, const poly::Envelope& envelope) {
    if (!processor_)
        return;
    auto* msg = new HostMessage; // ownership-transfer to local refcount
    msg->setMessageID("EnvelopeUpdate");
    if (auto* attrs = msg->getAttributes()) {
        attrs->setInt("lane", laneIndex);
        attrs->setInt("envIdx", envelopeIndex);
        attrs->setInt("active", active ? 1 : 0);
        attrs->setBinary("envelope", &envelope, static_cast<uint32>(sizeof(envelope)));
    }
    processor_->notify(msg);
    msg->release();
}

void PolyTestHost::injectAccentMask(int laneIndex, const std::array<float, poly::kMaxSteps>& steps) {
    if (!processor_)
        return;
    auto* msg = new HostMessage; // ownership-transfer to local refcount
    msg->setMessageID("AccentMaskUpdate");
    if (auto* attrs = msg->getAttributes()) {
        attrs->setInt("lane", laneIndex);
        attrs->setBinary("accents", steps.data(), static_cast<uint32>(sizeof(float) * steps.size()));
    }
    processor_->notify(msg);
    msg->release();
}

bool PolyTestHost::feedComponentState(const poly::SceneState& scene) {
    if (!controller_)
        return false;

    // Serialize both scenes + select + morph via the same writer the processor uses,
    // then push through setComponentState. Same DAW-preset-restore contract, but
    // driven by an in-memory SceneState so tests can pin select=SceneSelect::B and
    // put distinct values on each scene to force the P7 sceneA-hardcode to surface.
    MemoryStream stream;
    auto write = [&stream](const void* data, size_t size) -> bool {
        int32 written = 0;
        return stream.write(const_cast<void*>(data), static_cast<int32>(size), &written) == kResultOk &&
               written == static_cast<int32>(size);
    };
    if (!poly::writeSceneState(write, scene))
        return false;

    stream.seek(0, IBStream::kIBSeekSet, nullptr);
    return controller_->setComponentState(&stream) == kResultOk;
}

void PolyTestHost::injectPendingNoteOff(double ppqOff, int16_t pitch, int16_t channel) {
    if (!processor_)
        return;
    auto* proc = static_cast<PolyProcessor*>(processor_);
    proc->pushPendingNoteOffForTesting({.ppqOff = ppqOff, .pitch = pitch, .channel = channel});
}

uint64_t PolyTestHost::noteOffDrops() const {
    if (!processor_)
        return 0;
    return static_cast<PolyProcessor*>(processor_)->noteOffDrops();
}

void PolyTestHost::pushCapturedNote(const NoteEvent& note) {
    if (!processor_)
        return;
    static_cast<PolyProcessor*>(processor_)->pushCapturedNoteForTesting(note);
}

size_t PolyTestHost::capturedNoteCount() const {
    if (!processor_)
        return 0;
    return static_cast<PolyProcessor*>(processor_)->captureBufferCount();
}

PolyTestHost::HandshakeDropSnapshot PolyTestHost::handshakeDrops() const {
    HandshakeDropSnapshot snap{};
    if (!processor_)
        return snap;
    auto* proc = static_cast<PolyProcessor*>(processor_);
    const auto& c = proc->handshakeDrops();
    snap.state = c.state.load(std::memory_order_relaxed);
    snap.noteMap = c.noteMap.load(std::memory_order_relaxed);
    snap.cellSizes = c.cellSizes.load(std::memory_order_relaxed);
    snap.timeline = c.timeline.load(std::memory_order_relaxed);
    snap.microTiming = c.microTiming.load(std::memory_order_relaxed);
    snap.envelope = c.envelope.load(std::memory_order_relaxed);
    snap.accentMask = c.accentMask.load(std::memory_order_relaxed);
    return snap;
}

PolyTestHost::HandshakeAppliedSnapshot PolyTestHost::handshakeApplied() const {
    HandshakeAppliedSnapshot snap{};
    if (!processor_)
        return snap;
    auto* proc = static_cast<PolyProcessor*>(processor_);
    const auto& c = proc->handshakeApplied();
    snap.state = c.state.load(std::memory_order_relaxed);
    snap.noteMap = c.noteMap.load(std::memory_order_relaxed);
    snap.cellSizes = c.cellSizes.load(std::memory_order_relaxed);
    snap.timeline = c.timeline.load(std::memory_order_relaxed);
    snap.microTiming = c.microTiming.load(std::memory_order_relaxed);
    snap.envelope = c.envelope.load(std::memory_order_relaxed);
    snap.accentMask = c.accentMask.load(std::memory_order_relaxed);
    return snap;
}

} // namespace test
} // namespace poly
