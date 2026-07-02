#include "timeline_step_editor_view.h"

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

TimelineStepEditorView::TimelineStepEditorView(const CRect& size, PolyController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

TimelineStepEditorView::~TimelineStepEditorView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool TimelineStepEditorView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = makeOwned<CVSTGUITimer>([this](CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool TimelineStepEditorView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

CRect TimelineStepEditorView::stepRect(int stepIdx, int totalSteps) const {
    auto bounds = getViewSize();
    double barLeft = bounds.left + 8;
    double barRight = bounds.right - 8;
    double barW = barRight - barLeft;
    double barTop = bounds.top + 22;
    double barH = bounds.getHeight() - 26;
    double stepW = barW / totalSteps;
    return CRect(barLeft + stepIdx * stepW + 1, barTop, barLeft + (stepIdx + 1) * stepW - 1, barTop + barH);
}

int TimelineStepEditorView::hitTestStep(const CPoint& where) const {
    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    const auto& cfg = controller_->cachedState().sceneA.lanes[selectedLane];
    if (!cfg.timeline)
        return -1;
    int patLen = cfg.fixedPatternLength > 0 ? cfg.fixedPatternLength : cfg.cycle.steps;
    if (patLen <= 0)
        return -1;
    for (int i = 0; i < patLen; ++i) {
        if (stepRect(i, patLen).pointInside(where))
            return i;
    }
    return -1;
}

void TimelineStepEditorView::draw(CDrawContext* context) {
    auto bounds = getViewSize();

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    auto laneColor = kLaneColors[selectedLane];
    const auto& state = controller_->cachedState();
    const auto& cfg = state.sceneA.lanes[selectedLane];

    auto labelFont = makeOwned<CFontDesc>("Arial", 8.0);
    context->setFont(labelFont);
    context->setFontColor(CColor(0x50, 0x50, 0x60, 0xFF));
    CRect titleRect(bounds.left + 8, bounds.top + 4, bounds.left + 140, bounds.top + 16);
    context->drawString("TIMELINE PATTERN", titleRect, kLeftText);

    if (!cfg.timeline) {
        double barTop = bounds.top + 22;
        double barH = bounds.getHeight() - 26;
        CRect emptyRect(bounds.left + 8, barTop, bounds.right - 8, barTop + barH);
        context->setFillColor(CColor(laneColor.red, laneColor.green, laneColor.blue, 0x18));
        context->drawRect(emptyRect, kDrawFilled);
        auto emptyFont = makeOwned<CFontDesc>("Arial", 10.0);
        context->setFont(emptyFont);
        context->setFontColor(CColor(0x50, 0x50, 0x60, 0xFF));
        context->drawString("Enable timeline mode to edit", emptyRect, kCenterText);
    } else {
        int patLen = cfg.fixedPatternLength > 0 ? cfg.fixedPatternLength : cfg.cycle.steps;
        if (patLen <= 0)
            patLen = 1;

        char summary[32];
        std::snprintf(summary, sizeof(summary), "%d steps", patLen);
        CRect summaryRect(bounds.left + 140, bounds.top + 4, bounds.right - 8, bounds.top + 16);
        context->setFont(labelFont);
        context->setFontColor(CColor(laneColor.red, laneColor.green, laneColor.blue, 0x80));
        context->drawString(summary, summaryRect, kRightText);

        for (int i = 0; i < patLen && i < kMaxSteps; ++i) {
            CRect sr = stepRect(i, patLen);
            bool on = cfg.fixedPattern[i];

            if (on) {
                context->setFillColor(CColor(laneColor.red, laneColor.green, laneColor.blue, 0x80));
                context->drawRect(sr, kDrawFilled);
            } else {
                context->setFillColor(CColor(laneColor.red, laneColor.green, laneColor.blue, 0x18));
                context->drawRect(sr, kDrawFilled);
            }

            context->setFrameColor(CColor(laneColor.red, laneColor.green, laneColor.blue, 0x40));
            context->setLineWidth(1.0);
            context->drawRect(sr, kDrawStroked);
        }
    }

    setDirty(false);
}

CMouseEventResult TimelineStepEditorView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    if (!(buttons & kLButton))
        return kMouseEventNotHandled;

    int step = hitTestStep(where);
    if (step < 0)
        return kMouseEventNotHandled;

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    auto& cfg = controller_->mutableCachedState().sceneA.lanes[selectedLane];
    cfg.fixedPattern[step] = !cfg.fixedPattern[step];
    controller_->sendTimelinePattern(selectedLane);
    invalid();
    return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

} // namespace poly
