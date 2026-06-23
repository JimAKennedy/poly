#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"

namespace poly {

class PolyController;

class MicroTimingEditorView : public VSTGUI::CView {
public:
    MicroTimingEditorView(const VSTGUI::CRect& size, PolyController* controller);
    ~MicroTimingEditorView() override;

    void draw(VSTGUI::CDrawContext* context) override;
    bool attached(CView* parent) override;
    bool removed(CView* parent) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    static constexpr float kMaxOffsetMs = 20.0f;

    VSTGUI::CRect barArea() const;
    VSTGUI::CRect stepBarRect(int stepIdx, int totalSteps) const;
    int hitTestStep(const VSTGUI::CPoint& where) const;
    float yToMs(double y) const;

    PolyController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
    int dragStep_ = -1;
};

} // namespace poly
