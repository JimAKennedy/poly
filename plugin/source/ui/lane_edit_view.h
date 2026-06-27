#pragma once

#include <string>

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"

namespace poly {

class PolyController;

class LaneEditView : public VSTGUI::CView {
public:
    LaneEditView(const VSTGUI::CRect& size, PolyController* controller);
    ~LaneEditView() override;

    void draw(VSTGUI::CDrawContext* context) override;
    bool attached(CView* parent) override;
    bool removed(CView* parent) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
    void onKeyboardEvent(VSTGUI::KeyboardEvent& event) override;

private:
    static constexpr int kMaxLanes = 8;
    static constexpr int kLaneKnobCount = 11;
    static constexpr int kPhraseKnobCount = 6;
    static constexpr double kDragSensitivity = 200.0;
    static constexpr int kMaxNameLength = 15;

    enum class ValueFormat { Integer, Subdivision, MidiNote, Percent, KotekanSrc, Beats, BipolarSteps, BipolarMs, Ms };

    struct KnobDef {
        int paramOffset;
        bool isCore;
        const char* label;
        double displayMax;
        ValueFormat format;
    };
    static const KnobDef kLaneKnobs[kLaneKnobCount];
    static const KnobDef kPhraseKnobs[kPhraseKnobCount];

    VSTGUI::CRect laneTabRect(int lane) const;
    VSTGUI::CRect laneNameRect() const;
    VSTGUI::CRect laneKnobRect(int knob) const;
    VSTGUI::CRect phraseKnobRect(int knob) const;
    VSTGUI::CRect schematicRect() const;
    int hitTestTab(const VSTGUI::CPoint& where) const;
    int hitTestLaneKnob(const VSTGUI::CPoint& where) const;
    int hitTestPhraseKnob(const VSTGUI::CPoint& where) const;
    Steinberg::Vst::ParamID paramIdForKnob(const KnobDef& def, int lane) const;
    void drawKnob(VSTGUI::CDrawContext* ctx, const VSTGUI::CRect& rect, double value, const VSTGUI::CColor& color,
                  const KnobDef& def, bool enabled = true);
    void drawPhraseSchematic(VSTGUI::CDrawContext* ctx, const VSTGUI::CColor& color, double lenBeats, double gapBeats,
                             double ofsBeats);
    void beginNameEdit();
    void commitNameEdit();
    void cancelNameEdit();

    PolyController* polyController_;
    Steinberg::Vst::EditController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
    int selectedLane_ = 0;

    bool editingName_ = false;
    std::string editText_;
    int editCursorPos_ = 0;

    enum class DragTarget { None, LaneKnob, PhraseKnob };
    DragTarget dragTarget_ = DragTarget::None;
    int dragKnob_ = -1;
    double dragStartY_ = 0;
    double dragStartValue_ = 0;
};

} // namespace poly
