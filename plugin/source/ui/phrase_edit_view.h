#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"

namespace poly {

class PhraseEditView : public VSTGUI::CView {
public:
    PhraseEditView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller);
    ~PhraseEditView() override;

    void draw(VSTGUI::CDrawContext* context) override;
    bool attached(CView* parent) override;
    bool removed(CView* parent) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    static constexpr int kMaxLanes = 8;
    static constexpr int kKnobCount = 6;
    static constexpr double kDragSensitivity = 200.0;

    enum class ValueFormat { Beats, Percent, BipolarSteps, BipolarMs };

    struct KnobDef {
        int paramOffset;
        const char* label;
        double maxValue;
        ValueFormat format;
    };
    static const KnobDef kKnobs[kKnobCount];

    VSTGUI::CRect laneTabRect(int lane) const;
    VSTGUI::CRect knobRect(int knob) const;
    VSTGUI::CRect schematicRect() const;
    int hitTestTab(const VSTGUI::CPoint& where) const;
    int hitTestKnob(const VSTGUI::CPoint& where) const;
    void drawKnob(VSTGUI::CDrawContext* ctx, const VSTGUI::CRect& rect, double value, const VSTGUI::CColor& color,
                  const KnobDef& def, bool enabled);
    void drawPhraseSchematic(VSTGUI::CDrawContext* ctx, const VSTGUI::CColor& color, double lenBeats, double gapBeats,
                             double ofsBeats);

    Steinberg::Vst::EditController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
    int selectedLane_ = 0;
    int dragKnob_ = -1;
    double dragStartY_ = 0;
    double dragStartValue_ = 0;
};

} // namespace poly
