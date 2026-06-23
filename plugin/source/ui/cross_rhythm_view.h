#pragma once

#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"

namespace poly {

class PolyController;

class CrossRhythmView : public VSTGUI::CView {
public:
    CrossRhythmView(const VSTGUI::CRect& size, PolyController* controller);
    ~CrossRhythmView() override;

    void draw(VSTGUI::CDrawContext* context) override;
    bool attached(CView* parent) override;
    bool removed(CView* parent) override;

private:
    PolyController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
};

} // namespace poly
