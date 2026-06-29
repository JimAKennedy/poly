#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cview.h"

namespace poly {

class SceneBarView : public VSTGUI::CView {
public:
    SceneBarView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller);

    void draw(VSTGUI::CDrawContext* context) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    VSTGUI::CRect sceneButtonRect(int index) const;
    VSTGUI::CRect morphSliderRect() const;
    VSTGUI::CRect chainButtonRect() const;

    void pushParam(Steinberg::Vst::ParamID id, double value);
    void toggleChainPopover();
    bool isChainPopoverOpen() const;

    Steinberg::Vst::EditController* controller_;
    bool draggingMorph_ = false;
};

} // namespace poly
