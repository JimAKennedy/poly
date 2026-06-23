#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"

namespace poly {

class PolyController;

class TimelineStepEditorView : public VSTGUI::CView {
public:
    TimelineStepEditorView(const VSTGUI::CRect& size, PolyController* controller);
    ~TimelineStepEditorView() override;

    void draw(VSTGUI::CDrawContext* context) override;
    bool attached(CView* parent) override;
    bool removed(CView* parent) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    VSTGUI::CRect stepRect(int stepIdx, int totalSteps) const;
    int hitTestStep(const VSTGUI::CPoint& where) const;

    PolyController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
};

} // namespace poly
