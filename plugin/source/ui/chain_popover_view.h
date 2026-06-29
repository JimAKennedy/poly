#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cview.h"

namespace poly {

class ChainPopoverView : public VSTGUI::CView {
public:
    ChainPopoverView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller);

    void draw(VSTGUI::CDrawContext* context) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    static constexpr double kHeaderHeight = 44.0;
    static constexpr double kTopPadding = 8.0;
    static constexpr double kRowHeight = 32.0;
    static constexpr double kControlRowY = 50.0;
    static constexpr double kEntryStartY = 100.0;
    static constexpr double kPanelLeft = 40.0;
    static constexpr double kPanelRight = 560.0;

    VSTGUI::CRect closeButtonRect() const;
    VSTGUI::CRect enabledButtonRect() const;
    VSTGUI::CRect modeButtonRect(int mode) const;
    VSTGUI::CRect addButtonRect() const;
    VSTGUI::CRect removeButtonRect() const;
    VSTGUI::CRect entrySceneRect(int entry, int sceneIdx) const;
    VSTGUI::CRect entryBarsRect(int entry, bool isPlus) const;

    void pushParam(Steinberg::Vst::ParamID id, double value);

    int getEntryCount() const;
    int getChainMode() const;
    bool getChainEnabled() const;
    int getEntryScene(int entry) const;
    int getEntryBars(int entry) const;

    Steinberg::Vst::EditController* controller_;
};

} // namespace poly
