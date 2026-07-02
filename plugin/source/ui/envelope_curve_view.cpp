#include "envelope_curve_view.h"

#include <algorithm>
#include <cmath>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/cgraphicspath.h"

#include "../controller.h"
#include "../plugids.h"
#include "poly/envelope.h"

using namespace VSTGUI;

namespace poly {

static constexpr int kCurvePoints = 64;

EnvelopeCurveView::EnvelopeCurveView(const VSTGUI::CRect& size, PolyController* controller)
    : CView(size), controller_(controller) {
    setWantsFocus(false);
}

EnvelopeCurveView::~EnvelopeCurveView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool EnvelopeCurveView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = VSTGUI::makeOwned<VSTGUI::CVSTGUITimer>([this](VSTGUI::CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool EnvelopeCurveView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

void EnvelopeCurveView::draw(VSTGUI::CDrawContext* context) {
    auto bounds = getViewSize();

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    auto font = makeOwned<CFontDesc>("Arial", 9.0);

    constexpr double kPad = 8.0;
    constexpr double kLabelH = 14.0;
    double plotLeft = bounds.left + kPad;
    double plotRight = bounds.right - kPad;
    double plotTop = bounds.top + kPad;
    double plotBottom = bounds.bottom - kPad - kLabelH;
    double plotW = plotRight - plotLeft;
    double plotH = plotBottom - plotTop;

    context->setFrameColor(CColor(0x30, 0x30, 0x3C, 0xFF));
    context->setLineWidth(1.0);
    CRect plotFrame(plotLeft, plotTop, plotRight, plotBottom);
    context->drawRect(plotFrame, kDrawStroked);

    static const CColor kLaneColors[] = {
        CColor(0x4A, 0x9E, 0xFF, 0xB0), CColor(0xF5, 0xA6, 0x23, 0xB0), CColor(0x27, 0xAE, 0x60, 0xB0),
        CColor(0xE7, 0x4C, 0x3C, 0xB0), CColor(0x9B, 0x59, 0xB6, 0xB0), CColor(0x1A, 0xBC, 0x9C, 0xB0),
        CColor(0xE6, 0x7E, 0x22, 0xB0), CColor(0x34, 0x98, 0xDB, 0xB0),
    };

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    const auto& cachedScene = controller_->cachedState().sceneA;

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        auto activeId = ParamIDs::laneParam(lane, ParamIDs::kActive);
        if (controller_->getParamNormalized(activeId) <= 0.5)
            continue;

        bool selected = (lane == selectedLane);
        double envVal = controller_->getParamNormalized(ParamIDs::envelopeValueOutput(lane));
        double phase = controller_->getParamNormalized(ParamIDs::lanePhaseOutput(lane));

        const auto& laneConfig = cachedScene.lanes[lane];
        Envelope env{};
        if (laneConfig.envelopeCount > 0)
            env = laneConfig.envelopes[0].envelope;

        auto* path = context->createGraphicsPath();
        if (!path)
            continue;

        for (int i = 0; i <= kCurvePoints; ++i) {
            double t = static_cast<double>(i) / kCurvePoints;
            double val = static_cast<double>(evaluateShapeFull(env, static_cast<float>(t)));
            double x = plotLeft + t * plotW;
            double y = plotBottom - val * plotH;

            if (i == 0)
                path->beginSubpath(CPoint(x, y));
            else
                path->addLine(CPoint(x, y));
        }

        uint8_t curveAlpha = selected ? 0xE0 : 0x50;
        double curveWidth = selected ? 2.0 : 1.0;
        CColor curveColor(kLaneColors[lane].red, kLaneColors[lane].green, kLaneColors[lane].blue, curveAlpha);
        context->setFrameColor(curveColor);
        context->setLineWidth(curveWidth);
        context->drawGraphicsPath(path, CDrawContext::kPathStroked);
        path->forget();

        if (phase > 0.001 || envVal > 0.001) {
            double markerX = plotLeft + phase * plotW;
            double markerY = plotBottom - envVal * plotH;

            uint8_t lineAlpha = selected ? 0xB0 : 0x40;
            auto* linePath = context->createGraphicsPath();
            if (linePath) {
                linePath->beginSubpath(CPoint(markerX, plotTop));
                linePath->addLine(CPoint(markerX, plotBottom));
                context->setFrameColor(
                    CColor(kLaneColors[lane].red, kLaneColors[lane].green, kLaneColors[lane].blue, lineAlpha));
                context->setLineWidth(1.0);
                context->drawGraphicsPath(linePath, CDrawContext::kPathStroked);
                linePath->forget();
            }

            double dotR = selected ? 5.0 : 3.5;
            uint8_t dotAlpha = selected ? 0xFF : 0x90;
            CRect dot(markerX - dotR, markerY - dotR, markerX + dotR, markerY + dotR);
            context->setFillColor(
                CColor(kLaneColors[lane].red, kLaneColors[lane].green, kLaneColors[lane].blue, dotAlpha));
            context->drawEllipse(dot, kDrawFilled);
        }
    }

    context->setFont(font);
    context->setFontColor(CColor(0x6A, 0x6A, 0x80, 0xFF));
    CRect labelRect(plotLeft, plotBottom + 2, plotRight, bounds.bottom - 2);
    context->drawString("Envelope", labelRect, kCenterText);

    setDirty(false);
}

CMouseEventResult EnvelopeCurveView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    auto& lane = controller_->mutableCachedState().sceneA.lanes[selectedLane];

    if (buttons.isRightButton()) {
        if (lane.envelopeCount > 0) {
            lane.envelopes[0].envelope.depth = 1.0f;
            controller_->sendEnvelopeUpdate(selectedLane, 0);
        }
        invalid();
        return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    if (lane.envelopeCount == 0) {
        lane.envelopeCount = 1;
        lane.envelopes[0].active = true;
    }

    dragging_ = true;
    dragStartY_ = static_cast<float>(where.y);
    dragStartDepth_ = lane.envelopes[0].envelope.depth;
    return kMouseEventHandled;
}

CMouseEventResult EnvelopeCurveView::onMouseMoved(CPoint& where, const CButtonState& buttons) {
    if (!dragging_)
        return kMouseEventNotHandled;

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    auto& lane = controller_->mutableCachedState().sceneA.lanes[selectedLane];

    float dy = dragStartY_ - static_cast<float>(where.y);
    float newDepth = std::clamp(dragStartDepth_ + dy * 0.01f, 0.0f, 1.0f);
    lane.envelopes[0].envelope.depth = newDepth;
    invalid();
    return kMouseEventHandled;
}

CMouseEventResult EnvelopeCurveView::onMouseUp(CPoint& where, const CButtonState& buttons) {
    if (!dragging_)
        return kMouseEventNotHandled;
    dragging_ = false;
    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    controller_->sendEnvelopeUpdate(selectedLane, 0);
    return kMouseEventHandled;
}

} // namespace poly
