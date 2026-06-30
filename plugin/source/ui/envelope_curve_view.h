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

private:
    PolyController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
};

} // namespace poly
