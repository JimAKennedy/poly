//------------------------------------------------------------------------
// Copyright(c) 2025-2026 Jim Kennedy / jk.digital
// Shared headless VST3 UI host for automated interaction testing.
// Canonical source: audio-meta/test_harness/
//------------------------------------------------------------------------

#include "headless_ui_host.h"

#include <cstring>
#include <mutex>
#include <unistd.h>

#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "public.sdk/source/common/pluginview.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/vstguieditor.h"

#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/ccontrol.h"
#include "vstgui/lib/events.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/lib/vstguiinit.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <objc/message.h>
#include <objc/runtime.h>
#endif

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace VSTGUI;

void* moduleHandle = nullptr;

namespace JKDigital {
namespace InteractionTest {

//------------------------------------------------------------------------
static std::once_flag sPlatformInitFlag;

void initPlatformOnce() {
    std::call_once(sPlatformInitFlag, []() {
#ifdef __APPLE__
        VSTGUI::init(CFBundleGetMainBundle());
#else
        VSTGUI::init(nullptr);
#endif
    });
}

//------------------------------------------------------------------------
class TestComponentHandler : public IComponentHandler {
public:
    tresult PLUGIN_API beginEdit(ParamID id) override {
        (void)id;
        return kResultOk;
    }

    tresult PLUGIN_API performEdit(ParamID id,
                                   ParamValue valueNormalized) override {
        editLog.push_back({id, valueNormalized});
        return kResultOk;
    }

    tresult PLUGIN_API endEdit(ParamID id) override {
        (void)id;
        return kResultOk;
    }

    tresult PLUGIN_API restartComponent(int32 flags) override {
        (void)flags;
        return kResultOk;
    }

    const std::vector<ParameterEdit>& getEditLog() const { return editLog; }
    void clearEditLog() { editLog.clear(); }

    tresult PLUGIN_API queryInterface(const TUID iid, void** obj) override {
        if (FUnknownPrivate::iidEqual(iid, IComponentHandler::iid) ||
            FUnknownPrivate::iidEqual(iid, FUnknown::iid)) {
            *obj = this;
            addRef();
            return kResultTrue;
        }
        *obj = nullptr;
        return kNoInterface;
    }
    uint32 PLUGIN_API addRef() override { return 1000; }
    uint32 PLUGIN_API release() override { return 1000; }

private:
    std::vector<ParameterEdit> editLog;
};

//------------------------------------------------------------------------
class TestPlugFrame : public IPlugFrame {
public:
    tresult PLUGIN_API resizeView(IPlugView* /*view*/,
                                  ViewRect* /*newSize*/) override {
        return kResultOk;
    }

    tresult PLUGIN_API queryInterface(const TUID iid, void** obj) override {
        if (FUnknownPrivate::iidEqual(iid, IPlugFrame::iid) ||
            FUnknownPrivate::iidEqual(iid, FUnknown::iid)) {
            *obj = this;
            addRef();
            return kResultTrue;
        }
        *obj = nullptr;
        return kNoInterface;
    }
    uint32 PLUGIN_API addRef() override { return 1000; }
    uint32 PLUGIN_API release() override { return 1000; }
};

//------------------------------------------------------------------------
#ifdef __APPLE__

static void* createHiddenNSView(int width, int height) {
    Class nsViewClass = objc_getClass("NSView");
    if (!nsViewClass)
        return nullptr;

    SEL allocSel = sel_registerName("alloc");
    SEL initFrameSel = sel_registerName("initWithFrame:");

    struct CGRect {
        struct {
            double x, y;
        } origin;
        struct {
            double width, height;
        } size;
    };

    CGRect rect = {{0, 0},
                   {static_cast<double>(width), static_cast<double>(height)}};

    id allocated = ((id(*)(Class, SEL))objc_msgSend)(nsViewClass, allocSel);
    if (!allocated)
        return nullptr;

    id view = ((id(*)(id, SEL, CGRect))objc_msgSend)(allocated, initFrameSel,
                                                      rect);
    return reinterpret_cast<void*>(view);
}

static void releaseNSView(void* view) {
    if (view) {
        SEL releaseSel = sel_registerName("release");
        ((void (*)(id, SEL))objc_msgSend)(reinterpret_cast<id>(view),
                                          releaseSel);
    }
}

#endif

//------------------------------------------------------------------------
struct HeadlessUIHost::Impl {
    ControllerFactory factory;
    std::string resourceDir;
    IEditController* controller = nullptr;
    IPlugView* plugView = nullptr;
    CFrame* frame = nullptr;
    TestComponentHandler componentHandler;
    TestPlugFrame plugFrame;
    HostApplication hostApp;
    void* hiddenView = nullptr;
    bool opened = false;
};

//------------------------------------------------------------------------
HeadlessUIHost::HeadlessUIHost(ControllerFactory factory,
                               std::string resourceDir)
    : pImpl(new Impl) {
    pImpl->factory = std::move(factory);
    pImpl->resourceDir = std::move(resourceDir);
}

//------------------------------------------------------------------------
HeadlessUIHost::~HeadlessUIHost() {
    if (pImpl->opened)
        close();
    delete pImpl;
}

//------------------------------------------------------------------------
bool HeadlessUIHost::open() {
    if (pImpl->opened)
        return false;

    initPlatformOnce();

    auto* controller = pImpl->factory();
    if (!controller)
        return false;

    tresult result = controller->initialize(&pImpl->hostApp);
    if (result != kResultOk && result != kResultTrue) {
        controller->release();
        return false;
    }

    controller->setComponentHandler(&pImpl->componentHandler);
    pImpl->controller = controller;

    char savedCwd[1024] = {};
    getcwd(savedCwd, sizeof(savedCwd));
    if (!pImpl->resourceDir.empty())
        chdir(pImpl->resourceDir.c_str());

    auto* view = controller->createView(ViewType::kEditor);
    if (!view) {
        chdir(savedCwd);
        controller->terminate();
        controller->release();
        pImpl->controller = nullptr;
        return false;
    }
    pImpl->plugView = view;

    ViewRect viewSize{};
    view->getSize(&viewSize);
    int width = viewSize.right - viewSize.left;
    int height = viewSize.bottom - viewSize.top;
    if (width <= 0)
        width = 760;
    if (height <= 0)
        height = 500;

    view->setFrame(&pImpl->plugFrame);

#ifdef __APPLE__
    pImpl->hiddenView = createHiddenNSView(width, height);
    if (!pImpl->hiddenView) {
        chdir(savedCwd);
        view->release();
        controller->terminate();
        controller->release();
        pImpl->plugView = nullptr;
        pImpl->controller = nullptr;
        return false;
    }

    result = view->attached(pImpl->hiddenView, kPlatformTypeNSView);
#else
    result = kResultFalse;
#endif

    chdir(savedCwd);

    if (result != kResultOk && result != kResultTrue) {
        view->release();
#ifdef __APPLE__
        releaseNSView(pImpl->hiddenView);
        pImpl->hiddenView = nullptr;
#endif
        controller->terminate();
        controller->release();
        pImpl->plugView = nullptr;
        pImpl->controller = nullptr;
        return false;
    }

    auto* pluginView = static_cast<CPluginView*>(pImpl->plugView);
    auto* editorView = static_cast<Steinberg::Vst::EditorView*>(pluginView);
    auto* vstGuiEditor =
        static_cast<Steinberg::Vst::VSTGUIEditor*>(editorView);
    pImpl->frame = vstGuiEditor->getFrame();

    pImpl->opened = true;
    return true;
}

//------------------------------------------------------------------------
void HeadlessUIHost::close() {
    if (!pImpl->opened)
        return;

    if (pImpl->plugView) {
        pImpl->plugView->removed();
        pImpl->plugView->setFrame(nullptr);
        pImpl->plugView->release();
        pImpl->plugView = nullptr;
    }

#ifdef __APPLE__
    if (pImpl->hiddenView) {
        releaseNSView(pImpl->hiddenView);
        pImpl->hiddenView = nullptr;
    }
#endif

    if (pImpl->controller) {
        pImpl->controller->terminate();
        pImpl->controller->release();
        pImpl->controller = nullptr;
    }

    pImpl->opened = false;
}

//------------------------------------------------------------------------
void HeadlessUIHost::simulateClick(double x, double y) {
    auto* frame = getFrame();
    if (!frame)
        return;

    CPoint pos(x, y);

    MouseDownEvent downEvent(pos, MouseEventButtonState(MouseButton::Left));
    frame->dispatchEvent(static_cast<Event&>(downEvent));

    MouseUpEvent upEvent(pos, MouseEventButtonState(MouseButton::Left));
    frame->dispatchEvent(static_cast<Event&>(upEvent));
}

//------------------------------------------------------------------------
void HeadlessUIHost::simulateDrag(double startX, double startY,
                                  double endX, double endY, int steps) {
    auto* frame = getFrame();
    if (!frame)
        return;

    CPoint startPos(startX, startY);

    MouseDownEvent downEvent(startPos,
                             MouseEventButtonState(MouseButton::Left));
    frame->dispatchEvent(static_cast<Event&>(downEvent));

    for (int i = 1; i <= steps; ++i) {
        double t = static_cast<double>(i) / static_cast<double>(steps);
        double mx = startX + (endX - startX) * t;
        double my = startY + (endY - startY) * t;

        MouseMoveEvent moveEvent(CPoint(mx, my),
                                 MouseEventButtonState(MouseButton::Left));
        frame->dispatchEvent(static_cast<Event&>(moveEvent));
    }

    CPoint endPos(endX, endY);
    MouseUpEvent upEvent(endPos, MouseEventButtonState(MouseButton::Left));
    frame->dispatchEvent(static_cast<Event&>(upEvent));
}

//------------------------------------------------------------------------
void HeadlessUIHost::simulateMouseMove(double x, double y) {
    auto* frame = getFrame();
    if (!frame)
        return;

    MouseMoveEvent moveEvent(CPoint(x, y));
    frame->dispatchEvent(static_cast<Event&>(moveEvent));
}

//------------------------------------------------------------------------
void HeadlessUIHost::simulateKeyPress(char character) {
    auto* frame = getFrame();
    if (!frame)
        return;

    KeyboardEvent downEvent;
    downEvent.type = EventType::KeyDown;
    downEvent.character = character;
    frame->dispatchEvent(static_cast<Event&>(downEvent));

    KeyboardEvent upEvent;
    upEvent.type = EventType::KeyUp;
    upEvent.character = character;
    frame->dispatchEvent(static_cast<Event&>(upEvent));
}

//------------------------------------------------------------------------
void HeadlessUIHost::simulateScroll(double x, double y, float deltaY) {
    auto* frame = getFrame();
    if (!frame)
        return;

    MouseWheelEvent wheelEvent;
    wheelEvent.type = EventType::MouseWheel;
    wheelEvent.mousePosition = CPoint(x, y);
    wheelEvent.deltaY = static_cast<CCoord>(deltaY);
    frame->dispatchEvent(static_cast<Event&>(wheelEvent));
}

//------------------------------------------------------------------------
double HeadlessUIHost::getParameterValue(ParamID id) const {
    if (!pImpl->controller)
        return 0.0;
    return pImpl->controller->getParamNormalized(id);
}

//------------------------------------------------------------------------
std::string HeadlessUIHost::getParameterString(ParamID id) const {
    if (!pImpl->controller)
        return "";

    Steinberg::Vst::String128 text;
    ParamValue normalized = pImpl->controller->getParamNormalized(id);
    if (pImpl->controller->getParamStringByValue(id, normalized, text) ==
        kResultOk) {
        std::string result;
        for (int i = 0; text[i] != 0; ++i) {
            result += static_cast<char>(text[i]);
        }
        return result;
    }
    return "";
}

//------------------------------------------------------------------------
CPoint HeadlessUIHost::getControlCenter(int32_t tag) const {
    auto* frame = getFrame();
    if (!frame)
        return CPoint(-1, -1);

    auto* control = findControlByTag(frame, tag);
    if (!control)
        return CPoint(-1, -1);

    CRect r = control->getViewSize();

    auto* parent = control->getParentView();
    while (parent && parent != frame) {
        CRect parentRect = parent->getViewSize();
        r.offset(parentRect.left, parentRect.top);
        parent = parent->getParentView();
    }

    return CPoint(r.left + r.getWidth() / 2.0,
                  r.top + r.getHeight() / 2.0);
}

//------------------------------------------------------------------------
CRect HeadlessUIHost::getControlRect(int32_t tag) const {
    auto* frame = getFrame();
    if (!frame)
        return CRect();

    auto* control = findControlByTag(frame, tag);
    if (!control)
        return CRect();

    CRect r = control->getViewSize();

    auto* parent = control->getParentView();
    while (parent && parent != frame) {
        CRect parentRect = parent->getViewSize();
        r.offset(parentRect.left, parentRect.top);
        parent = parent->getParentView();
    }

    return r;
}

//------------------------------------------------------------------------
const std::vector<ParameterEdit>& HeadlessUIHost::getEditLog() const {
    return pImpl->componentHandler.getEditLog();
}

//------------------------------------------------------------------------
void HeadlessUIHost::clearEditLog() {
    pImpl->componentHandler.clearEditLog();
}

//------------------------------------------------------------------------
CFrame* HeadlessUIHost::getFrame() const {
    if (!pImpl->opened)
        return nullptr;
    return pImpl->frame;
}

//------------------------------------------------------------------------
bool HeadlessUIHost::isOpen() const { return pImpl->opened; }

//------------------------------------------------------------------------
CControl* HeadlessUIHost::findControlByTag(CViewContainer* container,
                                           int32_t tag) const {
    if (!container)
        return nullptr;

    for (uint32_t i = 0; i < container->getNbViews(); ++i) {
        auto* child = container->getView(i);
        if (!child)
            continue;

        auto* control = dynamic_cast<CControl*>(child);
        if (control && control->getTag() == tag &&
            control->getMouseEnabled())
            return control;

        auto* childContainer = dynamic_cast<CViewContainer*>(child);
        if (childContainer) {
            auto* found = findControlByTag(childContainer, tag);
            if (found)
                return found;
        }
    }
    return nullptr;
}

//------------------------------------------------------------------------
}  // namespace InteractionTest
}  // namespace JKDigital
