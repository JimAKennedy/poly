#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "pluginterfaces/vst/ivstevents.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"

#include "poly_test_host.h"
#include "probe_processor.h"

using namespace Steinberg;
using namespace Steinberg::Vst;
using probe::ProbeEvent;
using probe::ProbeProcessor;

static HostApplication sHostApp;

class ProbeTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        proc_ = new ProbeProcessor();
        ASSERT_EQ(proc_->initialize(&sHostApp), kResultOk);

        ProcessSetup ps{};
        ps.processMode = kRealtime;
        ps.symbolicSampleSize = kSample32;
        ps.maxSamplesPerBlock = 512;
        ps.sampleRate = 44100.0;
        proc_->setupProcessing(ps);

        ASSERT_EQ(proc_->setActive(true), kResultOk);
    }

    void TearDown() override {
        if (proc_) {
            proc_->setActive(false);
            proc_->terminate();
            proc_->release();
            proc_ = nullptr;
        }
    }

    void feedEvents(EventList& inputEvents) {
        ParameterChanges inParams;
        ParameterChanges outParams;

        ProcessData data{};
        data.processMode = kRealtime;
        data.symbolicSampleSize = kSample32;
        data.numSamples = 512;
        data.numInputs = 0;
        data.inputs = nullptr;
        data.numOutputs = 0;
        data.outputs = nullptr;
        data.inputEvents = &inputEvents;
        data.outputEvents = nullptr;
        data.inputParameterChanges = &inParams;
        data.outputParameterChanges = &outParams;
        data.processContext = nullptr;

        proc_->process(data);
    }

    ProbeProcessor* proc_ = nullptr;
};

// --- T02: Standalone probe tests ---

TEST_F(ProbeTestFixture, CapturesNoteOnEvents) {
    EventList events;
    Event ev{};
    ev.type = Event::kNoteOnEvent;
    ev.ppqPosition = 1.5;
    ev.sampleOffset = 0;
    ev.noteOn.channel = 0;
    ev.noteOn.pitch = 36;
    ev.noteOn.velocity = 0.8f;
    events.addEvent(ev);

    ev.ppqPosition = 2.0;
    ev.noteOn.pitch = 42;
    ev.noteOn.velocity = 0.6f;
    events.addEvent(ev);

    feedEvents(events);

    ASSERT_EQ(proc_->events().size(), 2u);
    EXPECT_EQ(proc_->events()[0].pitch, 36);
    EXPECT_DOUBLE_EQ(proc_->events()[0].ppqPosition, 1.5);
    EXPECT_FLOAT_EQ(proc_->events()[0].velocity, 0.8f);
    EXPECT_EQ(proc_->events()[1].pitch, 42);
}

TEST_F(ProbeTestFixture, CapturesNoteOffEvents) {
    EventList events;
    Event ev{};
    ev.type = Event::kNoteOffEvent;
    ev.ppqPosition = 3.0;
    ev.sampleOffset = 0;
    ev.noteOff.channel = 1;
    ev.noteOff.pitch = 38;
    ev.noteOff.velocity = 0.0f;
    events.addEvent(ev);

    feedEvents(events);

    ASSERT_EQ(proc_->events().size(), 1u);
    EXPECT_EQ(proc_->events()[0].type, ProbeEvent::NoteOff);
    EXPECT_EQ(proc_->events()[0].pitch, 38);
    EXPECT_EQ(proc_->events()[0].channel, 1);
}

TEST_F(ProbeTestFixture, EmptyInputProducesNoEvents) {
    EventList events;
    feedEvents(events);
    EXPECT_TRUE(proc_->events().empty());
}

TEST_F(ProbeTestFixture, MultipleBlocksAccumulate) {
    for (int block = 0; block < 3; ++block) {
        EventList events;
        Event ev{};
        ev.type = Event::kNoteOnEvent;
        ev.ppqPosition = static_cast<double>(block);
        ev.sampleOffset = 0;
        ev.noteOn.channel = 0;
        ev.noteOn.pitch = static_cast<int16>(36 + block);
        ev.noteOn.velocity = 0.7f;
        events.addEvent(ev);
        feedEvents(events);
    }

    ASSERT_EQ(proc_->events().size(), 3u);
    EXPECT_EQ(proc_->events()[0].pitch, 36);
    EXPECT_EQ(proc_->events()[1].pitch, 37);
    EXPECT_EQ(proc_->events()[2].pitch, 38);
}

TEST_F(ProbeTestFixture, ClearEventsResets) {
    EventList events;
    Event ev{};
    ev.type = Event::kNoteOnEvent;
    ev.ppqPosition = 0.0;
    ev.sampleOffset = 0;
    ev.noteOn.channel = 0;
    ev.noteOn.pitch = 36;
    ev.noteOn.velocity = 0.5f;
    events.addEvent(ev);
    feedEvents(events);

    ASSERT_EQ(proc_->events().size(), 1u);
    proc_->clearEvents();
    EXPECT_TRUE(proc_->events().empty());
}

TEST_F(ProbeTestFixture, WritesJsonl) {
    EventList events;
    Event ev{};
    ev.type = Event::kNoteOnEvent;
    ev.ppqPosition = 1.0;
    ev.sampleOffset = 0;
    ev.noteOn.channel = 0;
    ev.noteOn.pitch = 36;
    ev.noteOn.velocity = 0.75f;
    events.addEvent(ev);

    ev.type = Event::kNoteOffEvent;
    ev.ppqPosition = 1.5;
    ev.noteOff.channel = 0;
    ev.noteOff.pitch = 36;
    ev.noteOff.velocity = 0.0f;
    events.addEvent(ev);

    feedEvents(events);

    std::string path = std::string(::testing::TempDir()) + "/probe_test.jsonl";
    ASSERT_TRUE(proc_->writeJsonl(path));

    std::ifstream in(path);
    ASSERT_TRUE(in.is_open());

    std::string line1;
    std::string line2;
    std::getline(in, line1);
    std::getline(in, line2);

    EXPECT_NE(line1.find("\"noteOn\""), std::string::npos);
    EXPECT_NE(line1.find("\"pitch\":36"), std::string::npos);
    EXPECT_NE(line1.find("\"ppq\":1.000000"), std::string::npos);
    EXPECT_NE(line2.find("\"noteOff\""), std::string::npos);
    EXPECT_NE(line2.find("\"pitch\":36"), std::string::npos);

    std::remove(path.c_str());
}

// --- T03: Poly -> Probe chain integration ---

TEST(ProbeChain, PolyOutputMatchesProbeCapture) {
    poly::test::PolyTestHost polyHost;
    ASSERT_TRUE(polyHost.setup(44100.0, 512));
    polyHost.playBars(4, 120.0);

    auto polyNoteOns = polyHost.noteOnEvents();
    ASSERT_FALSE(polyNoteOns.empty());

    auto* probe = new ProbeProcessor();
    ASSERT_EQ(probe->initialize(&sHostApp), kResultOk);

    ProcessSetup ps{};
    ps.processMode = kRealtime;
    ps.symbolicSampleSize = kSample32;
    ps.maxSamplesPerBlock = 512;
    ps.sampleRate = 44100.0;
    probe->setupProcessing(ps);
    ASSERT_EQ(probe->setActive(true), kResultOk);

    const size_t kBatchSize = 32;
    const auto& allEvents = polyHost.events();
    for (size_t start = 0; start < allEvents.size(); start += kBatchSize) {
        EventList inputEvents;
        size_t end = std::min(start + kBatchSize, allEvents.size());
        for (size_t i = start; i < end; ++i) {
            const auto& e = allEvents[i];
            Event ev{};
            ev.sampleOffset = e.sampleOffset;
            ev.ppqPosition = e.ppqPosition;
            if (e.type == poly::test::MidiEvent::NoteOn) {
                ev.type = Event::kNoteOnEvent;
                ev.noteOn.channel = e.channel;
                ev.noteOn.pitch = e.pitch;
                ev.noteOn.velocity = e.velocity;
            } else {
                ev.type = Event::kNoteOffEvent;
                ev.noteOff.channel = e.channel;
                ev.noteOff.pitch = e.pitch;
                ev.noteOff.velocity = e.velocity;
            }
            inputEvents.addEvent(ev);
        }

        ParameterChanges inParams;
        ParameterChanges outParams;
        ProcessData data{};
        data.processMode = kRealtime;
        data.symbolicSampleSize = kSample32;
        data.numSamples = 512;
        data.numInputs = 0;
        data.inputs = nullptr;
        data.numOutputs = 0;
        data.outputs = nullptr;
        data.inputEvents = &inputEvents;
        data.outputEvents = nullptr;
        data.inputParameterChanges = &inParams;
        data.outputParameterChanges = &outParams;
        data.processContext = nullptr;

        probe->process(data);
    }

    const auto& probeEvents = probe->events();
    ASSERT_EQ(probeEvents.size(), allEvents.size()) << "Probe should capture all events from Poly";

    size_t noteOnIdx = 0;
    for (const auto& pe : probeEvents) {
        if (pe.type != ProbeEvent::NoteOn)
            continue;
        ASSERT_LT(noteOnIdx, polyNoteOns.size());
        EXPECT_EQ(pe.pitch, polyNoteOns[noteOnIdx].pitch) << "Pitch mismatch at note-on " << noteOnIdx;
        EXPECT_DOUBLE_EQ(pe.ppqPosition, polyNoteOns[noteOnIdx].ppqPosition) << "PPQ mismatch at note-on " << noteOnIdx;
        EXPECT_FLOAT_EQ(pe.velocity, polyNoteOns[noteOnIdx].velocity) << "Velocity mismatch at note-on " << noteOnIdx;
        EXPECT_EQ(pe.channel, polyNoteOns[noteOnIdx].channel) << "Channel mismatch at note-on " << noteOnIdx;
        noteOnIdx++;
    }
    EXPECT_EQ(noteOnIdx, polyNoteOns.size()) << "Not all Poly note-ons found in probe";

    std::string jsonlPath = std::string(::testing::TempDir()) + "/probe_chain.jsonl";
    ASSERT_TRUE(probe->writeJsonl(jsonlPath));

    std::ifstream in(jsonlPath);
    int lineCount = 0;
    std::string line;
    while (std::getline(in, line))
        lineCount++;
    EXPECT_EQ(static_cast<size_t>(lineCount), probeEvents.size());

    std::remove(jsonlPath.c_str());

    probe->setActive(false);
    probe->terminate();
    probe->release();
    polyHost.teardown();
}
