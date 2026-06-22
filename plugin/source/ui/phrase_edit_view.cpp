#include "phrase_edit_view.h"

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

static const char* kLaneNames[] = {"Kick", "Snare", "HH Cl", "HH Op", "Tom H", "Tom L", "Ride", "Crash"};

const PhraseEditView::KnobDef PhraseEditView::kKnobs[kKnobCount] = {
    {ParamIDs::kPhraseLength, "Len", 64.0},
    {ParamIDs::kPhraseGap, "Gap", 64.0},
    {ParamIDs::kPhraseOffset, "Ofs", 64.0},
};

PhraseEditView::PhraseEditView(const CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

PhraseEditView::~PhraseEditView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool PhraseEditView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = makeOwned<CVSTGUITimer>([this](CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool PhraseEditView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

CRect PhraseEditView::laneTabRect(int lane) const {
    auto bounds = getViewSize();
    double x = bounds.left + 8 + lane * 27.0;
    return CRect(x, bounds.top + 4, x + 24, bounds.top + 18);
}

CRect PhraseEditView::knobRect(int knob) const {
    auto bounds = getViewSize();
    double startX = bounds.left + 310.0;
    double x = startX + knob * 90.0;
    return CRect(x, bounds.top + 16, x + 32, bounds.top + 48);
}

int PhraseEditView::hitTestTab(const CPoint& where) const {
    for (int i = 0; i < kMaxLanes; ++i) {
        if (laneTabRect(i).pointInside(where))
            return i;
    }
    return -1;
}

int PhraseEditView::hitTestKnob(const CPoint& where) const {
    for (int i = 0; i < kKnobCount; ++i) {
        auto r = knobRect(i);
        r.extend(4, 4);
        if (r.pointInside(where))
            return i;
    }
    return -1;
}

void PhraseEditView::drawKnob(CDrawContext* ctx, const CRect& rect, double value, const CColor& color,
                              const char* label, double maxBeats, bool enabled) {
    double cx = rect.left + rect.getWidth() / 2;
    double cy = rect.top + rect.getHeight() / 2;
    double r = std::min(rect.getWidth(), rect.getHeight()) / 2 - 2;

    constexpr double kPi = 3.14159265358979;
    constexpr double kStartAngle = 3.0 * kPi / 4.0;
    constexpr double kSweepAngle = 3.0 * kPi / 2.0;
    constexpr int kSegments = 36;

    uint8_t trackAlpha = enabled ? 0xFF : 0x40;
    ctx->setLineWidth(2.5);
    ctx->setFrameColor(CColor(0x2A, 0x2A, 0x36, trackAlpha));
    for (int i = 0; i < kSegments; ++i) {
        double a0 = kStartAngle + kSweepAngle * i / kSegments;
        double a1 = kStartAngle + kSweepAngle * (i + 1) / kSegments;
        ctx->drawLine(CPoint(cx + r * std::cos(a0), cy + r * std::sin(a0)),
                      CPoint(cx + r * std::cos(a1), cy + r * std::sin(a1)));
    }

    if (enabled && value > 0.005) {
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
    constexpr double kDotR = 3.0;
    CRect dot(hx - kDotR, hy - kDotR, hx + kDotR, hy + kDotR);
    uint8_t dotAlpha = enabled ? 0xFF : 0x30;
    ctx->setFillColor(CColor(0xE8, 0xE8, 0xEC, dotAlpha));
    ctx->drawEllipse(dot, kDrawFilled);

    auto font = makeOwned<CFontDesc>("Arial", 9.0);
    ctx->setFont(font);

    uint8_t labelAlpha = enabled ? 0xA0 : 0x38;
    ctx->setFontColor(CColor(0x88, 0x88, labelAlpha, 0xFF));
    CRect labelRect(rect.left - 8, rect.bottom + 1, rect.right + 8, rect.bottom + 12);
    ctx->drawString(label, labelRect, kCenterText);

    double beats = value * maxBeats;
    char valStr[16];
    if (!enabled) {
        std::snprintf(valStr, sizeof(valStr), "--");
    } else if (beats < 0.05) {
        std::snprintf(valStr, sizeof(valStr), "off");
    } else if (beats >= 10.0) {
        std::snprintf(valStr, sizeof(valStr), "%.0f bt", beats);
    } else {
        std::snprintf(valStr, sizeof(valStr), "%.1f bt", beats);
    }
    auto smallFont = makeOwned<CFontDesc>("Arial", 8.0);
    ctx->setFont(smallFont);
    uint8_t valAlpha = enabled ? 0x78 : 0x30;
    ctx->setFontColor(CColor(0x66, 0x66, valAlpha, 0xFF));
    CRect valRect(rect.left - 8, rect.top - 11, rect.right + 8, rect.top - 1);
    ctx->drawString(valStr, valRect, kCenterText);
}

void PhraseEditView::draw(CDrawContext* context) {
    auto bounds = getViewSize();

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

    auto nameFont = makeOwned<CFontDesc>("Arial", 10.0);
    context->setFont(nameFont);
    context->setFontColor(kLaneColors[selectedLane_]);
    CRect nameRect(bounds.left + 228, bounds.top + 4, bounds.left + 300, bounds.top + 18);
    context->drawString(kLaneNames[selectedLane_], nameRect, kLeftText);

    auto laneColor = kLaneColors[selectedLane_];
    auto lenParamId = ParamIDs::laneParam(selectedLane_, kKnobs[0].paramOffset);
    double lenValue = controller_->getParamNormalized(lenParamId);
    bool lenActive = lenValue > 0.005;

    for (int k = 0; k < kKnobCount; ++k) {
        auto r = knobRect(k);
        auto paramId = ParamIDs::laneParam(selectedLane_, kKnobs[k].paramOffset);
        double value = controller_->getParamNormalized(paramId);
        bool enabled = (k == 0) || lenActive;
        drawKnob(context, r, value, laneColor, kKnobs[k].label, kKnobs[k].maxBeats, enabled);
    }

    setDirty(false);
}

CMouseEventResult PhraseEditView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    if (!(buttons & kLButton))
        return kMouseEventNotHandled;

    int tab = hitTestTab(where);
    if (tab >= 0) {
        selectedLane_ = tab;
        invalid();
        return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    int knob = hitTestKnob(where);
    if (knob >= 0) {
        if (knob > 0) {
            auto lenId = ParamIDs::laneParam(selectedLane_, kKnobs[0].paramOffset);
            if (controller_->getParamNormalized(lenId) < 0.005)
                return kMouseEventNotHandled;
        }
        dragKnob_ = knob;
        dragStartY_ = where.y;
        auto paramId = ParamIDs::laneParam(selectedLane_, kKnobs[knob].paramOffset);
        dragStartValue_ = controller_->getParamNormalized(paramId);
        controller_->beginEdit(paramId);
        return kMouseEventHandled;
    }

    return kMouseEventNotHandled;
}

CMouseEventResult PhraseEditView::onMouseMoved(CPoint& where, const CButtonState& buttons) {
    if (dragKnob_ < 0)
        return kMouseEventNotHandled;

    double delta = (dragStartY_ - where.y) / kDragSensitivity;
    double newValue = dragStartValue_ + delta;
    if (newValue < 0.0)
        newValue = 0.0;
    if (newValue > 1.0)
        newValue = 1.0;

    auto paramId = ParamIDs::laneParam(selectedLane_, kKnobs[dragKnob_].paramOffset);
    controller_->setParamNormalized(paramId, newValue);
    controller_->performEdit(paramId, newValue);
    invalid();
    return kMouseEventHandled;
}

CMouseEventResult PhraseEditView::onMouseUp(CPoint& where, const CButtonState& buttons) {
    if (dragKnob_ < 0)
        return kMouseEventNotHandled;

    auto paramId = ParamIDs::laneParam(selectedLane_, kKnobs[dragKnob_].paramOffset);
    controller_->endEdit(paramId);
    dragKnob_ = -1;
    return kMouseEventHandled;
}

} // namespace poly
