#include "micro_timing_editor_view.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../controller.h"
#include "../plugids.h"

namespace poly {

using namespace VSTGUI;

static const CColor kLaneColors[] = {
    CColor(0x4A, 0x9E, 0xFF, 0xFF), CColor(0xF5, 0xA6, 0x23, 0xFF), CColor(0x27, 0xAE, 0x60, 0xFF),
    CColor(0xE7, 0x4C, 0x3C, 0xFF), CColor(0x9B, 0x59, 0xB6, 0xFF), CColor(0x1A, 0xBC, 0x9C, 0xFF),
    CColor(0xE6, 0x7E, 0x22, 0xFF), CColor(0x34, 0x98, 0xDB, 0xFF),
};

MicroTimingEditorView::MicroTimingEditorView(const CRect& size, PolyController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

MicroTimingEditorView::~MicroTimingEditorView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool MicroTimingEditorView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = makeOwned<CVSTGUITimer>([this](CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool MicroTimingEditorView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

CRect MicroTimingEditorView::barArea() const {
    auto bounds = getViewSize();
    return CRect(bounds.left + 8, bounds.top + 18, bounds.right - 8, bounds.bottom - 4);
}

CRect MicroTimingEditorView::stepBarRect(int stepIdx, int totalSteps) const {
    auto area = barArea();
    double stepW = area.getWidth() / totalSteps;
    return CRect(area.left + stepIdx * stepW + 1, area.top, area.left + (stepIdx + 1) * stepW - 1, area.bottom);
}

int MicroTimingEditorView::hitTestStep(const CPoint& where) const {
    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    const auto& cfg = controller_->cachedState().sceneA.lanes[selectedLane];
    int steps = cfg.cycle.steps;
    if (steps <= 0)
        return -1;
    auto area = barArea();
    if (where.y < area.top || where.y > area.bottom)
        return -1;
    for (int i = 0; i < steps; ++i) {
        auto sr = stepBarRect(i, steps);
        if (where.x >= sr.left && where.x < sr.right)
            return i;
    }
    return -1;
}

float MicroTimingEditorView::yToMs(double y) const {
    auto area = barArea();
    double centerY = area.top + area.getHeight() * 0.5;
    double halfH = area.getHeight() * 0.5;
    double norm = (centerY - y) / halfH;
    return static_cast<float>(
        std::clamp(norm * kMaxOffsetMs, -static_cast<double>(kMaxOffsetMs), static_cast<double>(kMaxOffsetMs)));
}

void MicroTimingEditorView::draw(CDrawContext* context) {
    auto bounds = getViewSize();

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    auto laneColor = kLaneColors[selectedLane];
    const auto& cfg = controller_->cachedState().sceneA.lanes[selectedLane];
    int steps = cfg.cycle.steps;

    auto labelFont = makeOwned<CFontDesc>("Arial", 8.0);
    context->setFont(labelFont);
    context->setFontColor(CColor(0x50, 0x50, 0x60, 0xFF));
    CRect titleRect(bounds.left + 8, bounds.top + 4, bounds.left + 140, bounds.top + 16);
    context->drawString("MICRO-TIMING", titleRect, kLeftText);

    if (steps <= 0) {
        setDirty(false);
        return;
    }

    auto area = barArea();
    double centerY = area.top + area.getHeight() * 0.5;

    context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
    context->setLineWidth(1.0);
    context->drawLine(CPoint(area.left, centerY), CPoint(area.right, centerY));

    double halfH = area.getHeight() * 0.5;

    for (int i = 0; i < steps && i < kMaxSteps; ++i) {
        float ms = cfg.microTimingMs[i];
        double norm = ms / kMaxOffsetMs;
        double barH = std::abs(norm) * halfH;

        auto sr = stepBarRect(i, steps);
        CRect barR;
        if (ms >= 0) {
            barR = CRect(sr.left, centerY - barH, sr.right, centerY);
        } else {
            barR = CRect(sr.left, centerY, sr.right, centerY + barH);
        }

        uint8_t alpha = static_cast<uint8_t>(0x40 + std::abs(norm) * 0x80);
        context->setFillColor(CColor(laneColor.red, laneColor.green, laneColor.blue, alpha));
        context->drawRect(barR, kDrawFilled);

        context->setFrameColor(CColor(laneColor.red, laneColor.green, laneColor.blue, 0x40));
        context->setLineWidth(1.0);
        context->drawRect(sr, kDrawStroked);
    }

    if (dragStep_ >= 0 && dragStep_ < steps) {
        float ms = cfg.microTimingMs[dragStep_];
        char tooltip[16];
        std::snprintf(tooltip, sizeof(tooltip), "%.1fms", ms);
        auto sr = stepBarRect(dragStep_, steps);
        auto tooltipFont = makeOwned<CFontDesc>("Arial", 9.0);
        context->setFont(tooltipFont);
        context->setFontColor(CColor(0xE8, 0xE8, 0xEC, 0xFF));
        CRect tooltipRect(sr.left - 10, bounds.top + 4, sr.right + 10, bounds.top + 16);
        context->drawString(tooltip, tooltipRect, kCenterText);
    }

    setDirty(false);
}

CMouseEventResult MicroTimingEditorView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    if (!(buttons & kLButton))
        return kMouseEventNotHandled;

    int step = hitTestStep(where);
    if (step < 0)
        return kMouseEventNotHandled;

    if (buttons & kDoubleClick) {
        int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
        auto& cfg = controller_->mutableCachedState().sceneA.lanes[selectedLane];
        cfg.microTimingMs[step] = 0.0f;
        invalid();
        return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    dragStep_ = step;
    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    auto& cfg = controller_->mutableCachedState().sceneA.lanes[selectedLane];
    cfg.microTimingMs[step] = yToMs(where.y);
    invalid();
    return kMouseEventHandled;
}

CMouseEventResult MicroTimingEditorView::onMouseMoved(CPoint& where, const CButtonState& buttons) {
    if (dragStep_ < 0)
        return kMouseEventNotHandled;

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    auto& cfg = controller_->mutableCachedState().sceneA.lanes[selectedLane];
    cfg.microTimingMs[dragStep_] = yToMs(where.y);
    invalid();
    return kMouseEventHandled;
}

CMouseEventResult MicroTimingEditorView::onMouseUp(CPoint& where, const CButtonState& buttons) {
    if (dragStep_ < 0)
        return kMouseEventNotHandled;
    dragStep_ = -1;
    return kMouseEventHandled;
}

} // namespace poly
