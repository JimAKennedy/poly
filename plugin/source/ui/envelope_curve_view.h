#pragma once

#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"

namespace poly {

class PolyController;

class EnvelopeCurveView : public VSTGUI::CView {
public:
    EnvelopeCurveView(const VSTGUI::CRect& size, PolyController* controller);
    ~EnvelopeCurveView() override;

    void draw(VSTGUI::CDrawContext* context) override;
    bool attached(CView* parent) override;
    bool removed(CView* parent) override;

    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    PolyController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
    bool dragging_ = false;
    float dragStartY_ = 0.0f;
    float dragStartDepth_ = 0.0f;
};

} // namespace poly
