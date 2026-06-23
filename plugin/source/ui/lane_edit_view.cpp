#include "lane_edit_view.h"

#include <cmath>
#include <cstdio>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../plugids.h"

namespace poly {

using namespace VSTGUI;

static const CColor kLaneColors[] = {
    CColor(0x4A, 0x9E, 0xFF, 0xFF), CColor(0xF5, 0xA6, 0x23, 0xFF), CColor(0x27, 0xAE, 0x60, 0xFF),
    CColor(0xE7, 0x4C, 0x3C, 0xFF), CColor(0x9B, 0x59, 0xB6, 0xFF), CColor(0x1A, 0xBC, 0x9C, 0xFF),
    CColor(0xE6, 0x7E, 0x22, 0xFF), CColor(0x34, 0x98, 0xDB, 0xFF),
};

static const char* kNoteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

const LaneEditView::KnobDef LaneEditView::kKnobs[kKnobCount] = {
    {ParamIDs::kCoreSteps, true, "Steps", 64.0, ValueFormat::Integer},
    {ParamIDs::kCoreSubdivision, true, "Subdiv", 4.0, ValueFormat::Subdivision},
    {ParamIDs::kCoreHits, true, "Hits", 64.0, ValueFormat::Integer},
    {ParamIDs::kCoreRotation, true, "Rot", 63.0, ValueFormat::Integer},
    {ParamIDs::kCoreMidiNote, true, "Note", 127.0, ValueFormat::MidiNote},
    {ParamIDs::kBaseVelocity, false, "Vel", 127.0, ValueFormat::Integer},
    {ParamIDs::kGhostFloor, false, "Ghost", 127.0, ValueFormat::Integer},
    {ParamIDs::kVelocitySpread, false, "Spread", 100.0, ValueFormat::Percent},
    {ParamIDs::kSwingAmount, false, "Swing", 100.0, ValueFormat::Percent},
    {ParamIDs::kKotekanSource, false, "Kotek", 8.0, ValueFormat::KotekanSrc},
};

LaneEditView::LaneEditView(const CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

LaneEditView::~LaneEditView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool LaneEditView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = makeOwned<CVSTGUITimer>([this](CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool LaneEditView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

CRect LaneEditView::laneTabRect(int lane) const {
    auto bounds = getViewSize();
    double x = bounds.left + 8 + lane * 27.0;
    return CRect(x, bounds.top + 4, x + 24, bounds.top + 18);
}

CRect LaneEditView::knobRect(int knob) const {
    static constexpr double kKnobX[] = {232, 265, 298, 331, 364, 406, 439, 472, 505, 538};
    auto bounds = getViewSize();
    double x = bounds.left + kKnobX[knob];
    return CRect(x, bounds.top + 16, x + 26, bounds.top + 42);
}

int LaneEditView::hitTestTab(const CPoint& where) const {
    for (int i = 0; i < kMaxLanes; ++i) {
        if (laneTabRect(i).pointInside(where))
            return i;
    }
    return -1;
}

int LaneEditView::hitTestKnob(const CPoint& where) const {
    for (int i = 0; i < kKnobCount; ++i) {
        auto r = knobRect(i);
        r.extend(4, 4);
        if (r.pointInside(where))
            return i;
    }
    return -1;
}

Steinberg::Vst::ParamID LaneEditView::paramIdForKnob(int knob, int lane) const {
    const auto& def = kKnobs[knob];
    if (def.isCore)
        return ParamIDs::laneCoreParam(lane, def.paramOffset);
    return ParamIDs::laneParam(lane, def.paramOffset);
}

void LaneEditView::drawKnob(CDrawContext* ctx, const CRect& rect, double value, const CColor& color,
                            const KnobDef& def) {
    double cx = rect.left + rect.getWidth() / 2;
    double cy = rect.top + rect.getHeight() / 2;
    double r = std::min(rect.getWidth(), rect.getHeight()) / 2 - 2;

    constexpr double kPi = 3.14159265358979;
    constexpr double kStartAngle = 3.0 * kPi / 4.0;
    constexpr double kSweepAngle = 3.0 * kPi / 2.0;
    constexpr int kSegments = 36;

    ctx->setLineWidth(2.0);
    ctx->setFrameColor(CColor(0x2A, 0x2A, 0x36, 0xFF));
    for (int i = 0; i < kSegments; ++i) {
        double a0 = kStartAngle + kSweepAngle * i / kSegments;
        double a1 = kStartAngle + kSweepAngle * (i + 1) / kSegments;
        ctx->drawLine(CPoint(cx + r * std::cos(a0), cy + r * std::sin(a0)),
                      CPoint(cx + r * std::cos(a1), cy + r * std::sin(a1)));
    }

    if (value > 0.005) {
        int valueSegs = std::max(1, static_cast<int>(kSegments * value));
        ctx->setFrameColor(CColor(color.red, color.green, color.blue, 0xCC));
        for (int i = 0; i < valueSegs; ++i) {
            double a0 = kStartAngle + kSweepAngle * i / kSegments;
            double a1 = kStartAngle + kSweepAngle * (i + 1) / kSegments;
            ctx->drawLine(CPoint(cx + r * std::cos(a0), cy + r * std::sin(a0)),
                          CPoint(cx + r * std::cos(a1), cy + r * std::sin(a1)));
        }
    }

    double angle = kStartAngle + kSweepAngle * value;
    double hx = cx + r * std::cos(angle);
    double hy = cy + r * std::sin(angle);
    constexpr double kDotR = 2.5;
    CRect dot(hx - kDotR, hy - kDotR, hx + kDotR, hy + kDotR);
    ctx->setFillColor(CColor(0xE8, 0xE8, 0xEC, 0xFF));
    ctx->drawEllipse(dot, kDrawFilled);

    auto font = makeOwned<CFontDesc>("Arial", 8.0);
    ctx->setFont(font);
    ctx->setFontColor(CColor(0x88, 0x88, 0xA0, 0xFF));
    CRect labelRect(rect.left - 6, rect.bottom + 1, rect.right + 6, rect.bottom + 11);
    ctx->drawString(def.label, labelRect, kCenterText);

    char valStr[16];
    if (def.format == ValueFormat::Integer) {
        int v = static_cast<int>(std::round(value * def.displayMax));
        std::snprintf(valStr, sizeof(valStr), "%d", v);
    } else if (def.format == ValueFormat::Subdivision) {
        static constexpr int subdivs[] = {1, 2, 4, 8, 16};
        int idx = static_cast<int>(std::round(value * 4.0));
        idx = std::clamp(idx, 0, 4);
        std::snprintf(valStr, sizeof(valStr), "1/%d", subdivs[idx]);
    } else if (def.format == ValueFormat::MidiNote) {
        int note = static_cast<int>(std::round(value * 127.0));
        int octave = (note / 12) - 2;
        const char* name = kNoteNames[note % 12];
        std::snprintf(valStr, sizeof(valStr), "%s%d", name, octave);
    } else if (def.format == ValueFormat::Percent) {
        double pct = value * 100.0;
        if (pct < 0.5)
            std::snprintf(valStr, sizeof(valStr), "off");
        else
            std::snprintf(valStr, sizeof(valStr), "%.0f%%", pct);
    } else if (def.format == ValueFormat::KotekanSrc) {
        int src = static_cast<int>(std::round(value * 8.0)) - 1;
        if (src < 0)
            std::snprintf(valStr, sizeof(valStr), "off");
        else
            std::snprintf(valStr, sizeof(valStr), "L%d", src + 1);
    }

    auto smallFont = makeOwned<CFontDesc>("Arial", 7.0);
    ctx->setFont(smallFont);
    ctx->setFontColor(CColor(0x66, 0x66, 0x78, 0xFF));
    CRect valRect(rect.left - 6, rect.top - 10, rect.right + 6, rect.top - 1);
    ctx->drawString(valStr, valRect, kCenterText);
}

void LaneEditView::draw(CDrawContext* context) {
    auto bounds = getViewSize();

    selectedLane_ = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    auto tabFont = makeOwned<CFontDesc>("Arial", 9.0);
    context->setFont(tabFont);

    for (int i = 0; i < kMaxLanes; ++i) {
        auto r = laneTabRect(i);
        auto color = kLaneColors[i];

        if (i == selectedLane_) {
            context->setFillColor(color);
            context->drawRect(r, kDrawFilled);
            context->setFontColor(CColor(0xFF, 0xFF, 0xFF, 0xFF));
        } else {
            context->setFillColor(CColor(color.red, color.green, color.blue, 0x28));
            context->drawRect(r, kDrawFilled);
            context->setFrameColor(CColor(color.red, color.green, color.blue, 0x50));
            context->setLineWidth(1.0);
            context->drawRect(r, kDrawStroked);
            context->setFontColor(CColor(0x80, 0x80, 0x90, 0xFF));
        }

        char lbl[4];
        std::snprintf(lbl, sizeof(lbl), "%d", i + 1);
        context->drawString(lbl, r, kCenterText);
    }

    auto laneColor = kLaneColors[selectedLane_];

    auto groupFont = makeOwned<CFontDesc>("Arial", 7.0);
    context->setFont(groupFont);
    context->setFontColor(CColor(0x50, 0x50, 0x60, 0xFF));
    CRect patternLabel(bounds.left + 232, bounds.top + 4, bounds.left + 395, bounds.top + 14);
    context->drawString("PATTERN", patternLabel, kLeftText);
    CRect voiceLabel(bounds.left + 406, bounds.top + 4, bounds.left + 570, bounds.top + 14);
    context->drawString("VOICE", voiceLabel, kLeftText);

    context->setLineWidth(0.5);
    context->setFrameColor(CColor(0x30, 0x30, 0x40, 0x60));
    double divX = bounds.left + 398;
    context->drawLine(CPoint(divX, bounds.top + 14), CPoint(divX, bounds.bottom - 2));

    for (int k = 0; k < kKnobCount; ++k) {
        auto r = knobRect(k);
        auto paramId = paramIdForKnob(k, selectedLane_);
        double value = controller_->getParamNormalized(paramId);
        drawKnob(context, r, value, laneColor, kKnobs[k]);
    }

    setDirty(false);
}

CMouseEventResult LaneEditView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    if (!(buttons & kLButton))
        return kMouseEventNotHandled;

    int tab = hitTestTab(where);
    if (tab >= 0) {
        selectedLane_ = tab;
        double norm = tab / 7.0;
        controller_->beginEdit(ParamIDs::kSelectedLane);
        controller_->setParamNormalized(ParamIDs::kSelectedLane, norm);
        controller_->performEdit(ParamIDs::kSelectedLane, norm);
        controller_->endEdit(ParamIDs::kSelectedLane);
        invalid();
        return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    int knob = hitTestKnob(where);
    if (knob >= 0) {
        dragKnob_ = knob;
        dragStartY_ = where.y;
        auto paramId = paramIdForKnob(knob, selectedLane_);
        dragStartValue_ = controller_->getParamNormalized(paramId);
        controller_->beginEdit(paramId);
        return kMouseEventHandled;
    }

    return kMouseEventNotHandled;
}

CMouseEventResult LaneEditView::onMouseMoved(CPoint& where, const CButtonState& buttons) {
    if (dragKnob_ < 0)
        return kMouseEventNotHandled;

    double delta = (dragStartY_ - where.y) / kDragSensitivity;
    double newValue = dragStartValue_ + delta;
    if (newValue < 0.0)
        newValue = 0.0;
    if (newValue > 1.0)
        newValue = 1.0;

    auto paramId = paramIdForKnob(dragKnob_, selectedLane_);
    controller_->setParamNormalized(paramId, newValue);
    controller_->performEdit(paramId, newValue);
    invalid();
    return kMouseEventHandled;
}

CMouseEventResult LaneEditView::onMouseUp(CPoint& where, const CButtonState& buttons) {
    if (dragKnob_ < 0)
        return kMouseEventNotHandled;

    auto paramId = paramIdForKnob(dragKnob_, selectedLane_);
    controller_->endEdit(paramId);
    dragKnob_ = -1;
    return kMouseEventHandled;
}

} // namespace poly
