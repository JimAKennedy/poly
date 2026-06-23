#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"

namespace poly {

class LaneGridView : public VSTGUI::CView {
public:
    LaneGridView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller);
    ~LaneGridView() override;

    void draw(VSTGUI::CDrawContext* context) override;
    bool attached(CView* parent) override;
    bool removed(CView* parent) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    static constexpr int kMaxLanes = 8;
    static constexpr double kPad = 4.0;

    int hitTestLane(const VSTGUI::CPoint& where) const;
    double probabilityFromX(int lane, VSTGUI::CCoord x) const;
    VSTGUI::CRect laneRect(int lane) const;

    Steinberg::Vst::EditController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
    int dragLane_ = -1;
};

} // namespace poly
