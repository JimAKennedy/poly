#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/lib/dragging.h"

namespace poly {

class ExportControlsView : public VSTGUI::CView {
public:
    ExportControlsView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller);

    void draw(VSTGUI::CDrawContext* context) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

    bool attached(VSTGUI::CView* parent) override;
    bool removed(VSTGUI::CView* parent) override;

private:
    VSTGUI::CRect exportButtonRect() const;
    VSTGUI::CRect barsLabelRect() const;
    VSTGUI::CRect barsMinusRect() const;
    VSTGUI::CRect barsPlusRect() const;

    void pushParam(Steinberg::Vst::ParamID id, double value);
    int currentBars() const;
    bool captureReady() const;
    void sendExportRequest();
    void openSaveDialog();
    void startDrag();

    Steinberg::Vst::EditController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> exportTimer_;
    bool exportPending_ = false;
    bool dragTracking_ = false;
    VSTGUI::CPoint dragStartPos_;
};

} // namespace poly
