//------------------------------------------------------------------------
// Copyright(c) 2025-2026 Jim Kennedy / jk.digital
// Shared headless VST3 UI host for automated interaction testing.
// Canonical source: audio-meta/test_harness/
//------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/ccontrol.h"
#include "vstgui/lib/cpoint.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cviewcontainer.h"

namespace JKDigital {
namespace InteractionTest {

//------------------------------------------------------------------------
struct ParameterEdit {
    Steinberg::Vst::ParamID paramId;
    Steinberg::Vst::ParamValue value;
};

//------------------------------------------------------------------------
using ControllerFactory = std::function<Steinberg::Vst::IEditController*()>;

//------------------------------------------------------------------------
class HeadlessUIHost {
public:
    explicit HeadlessUIHost(ControllerFactory factory, std::string resourceDir = "");
    ~HeadlessUIHost();

    HeadlessUIHost(const HeadlessUIHost&) = delete;
    HeadlessUIHost& operator=(const HeadlessUIHost&) = delete;

    bool open();
    void close();

    void simulateClick(double x, double y);
    void simulateDrag(double startX, double startY, double endX, double endY, int steps = 20);
    void simulateMouseMove(double x, double y);
    void simulateKeyPress(char character);
    void simulateScroll(double x, double y, float deltaY);

    double getParameterValue(Steinberg::Vst::ParamID id) const;
    std::string getParameterString(Steinberg::Vst::ParamID id) const;
    VSTGUI::CPoint getControlCenter(int32_t tag) const;
    VSTGUI::CRect getControlRect(int32_t tag) const;

    const std::vector<ParameterEdit>& getEditLog() const;
    void clearEditLog();

    VSTGUI::CFrame* getFrame() const;
    bool isOpen() const;

private:
    VSTGUI::CControl* findControlByTag(VSTGUI::CViewContainer* container, int32_t tag) const;

    struct Impl;
    Impl* pImpl = nullptr;
};

//------------------------------------------------------------------------
void initPlatformOnce();

//------------------------------------------------------------------------
} // namespace InteractionTest
} // namespace JKDigital
