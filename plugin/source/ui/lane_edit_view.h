#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"

namespace poly {

class LaneEditView : public VSTGUI::CView {
public:
    LaneEditView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller);
    ~LaneEditView() override;

    void draw(VSTGUI::CDrawContext* context) override;
    bool attached(CView* parent) override;
    bool removed(CView* parent) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    static constexpr int kMaxLanes = 8;
    static constexpr int kKnobCount = 10;
    static constexpr double kDragSensitivity = 200.0;

    enum class ValueFormat { Integer, Subdivision, MidiNote, Percent, KotekanSrc };

    struct KnobDef {
        int paramOffset;
        bool isCore;
        const char* label;
        double displayMax;
        ValueFormat format;
    };
    static const KnobDef kKnobs[kKnobCount];

    VSTGUI::CRect laneTabRect(int lane) const;
    VSTGUI::CRect knobRect(int knob) const;
    int hitTestTab(const VSTGUI::CPoint& where) const;
    int hitTestKnob(const VSTGUI::CPoint& where) const;
    Steinberg::Vst::ParamID paramIdForKnob(int knob, int lane) const;
    void drawKnob(VSTGUI::CDrawContext* ctx, const VSTGUI::CRect& rect, double value, const VSTGUI::CColor& color,
                  const KnobDef& def);

    Steinberg::Vst::EditController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
    int selectedLane_ = 0;
    int dragKnob_ = -1;
    double dragStartY_ = 0;
    double dragStartValue_ = 0;
};

} // namespace poly
