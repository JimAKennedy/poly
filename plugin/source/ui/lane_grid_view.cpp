#include "lane_grid_view.h"

#include <cmath>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../plugids.h"

namespace poly {

static const VSTGUI::CColor kLaneColors[] = {
    VSTGUI::CColor(0x4A, 0x9E, 0xFF, 0xFF), VSTGUI::CColor(0xF5, 0xA6, 0x23, 0xFF),
    VSTGUI::CColor(0x27, 0xAE, 0x60, 0xFF), VSTGUI::CColor(0xE7, 0x4C, 0x3C, 0xFF),
    VSTGUI::CColor(0x9B, 0x59, 0xB6, 0xFF), VSTGUI::CColor(0x1A, 0xBC, 0x9C, 0xFF),
    VSTGUI::CColor(0xE6, 0x7E, 0x22, 0xFF), VSTGUI::CColor(0x34, 0x98, 0xDB, 0xFF),
};

LaneGridView::LaneGridView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

LaneGridView::~LaneGridView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool LaneGridView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = VSTGUI::makeOwned<VSTGUI::CVSTGUITimer>([this](VSTGUI::CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool LaneGridView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

void LaneGridView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();
    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    double laneH = (bounds.getHeight() - kPad * (kMaxLanes + 1)) / kMaxLanes;
    double laneW = bounds.getWidth() - kPad * 2;

    static const char* kLaneNames[] = {"Kick", "Snare", "HH Closed", "HH Open", "Tom Hi", "Tom Lo", "Ride", "Crash"};

    auto font = makeOwned<CFontDesc>("Arial", 11.0);

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        double y = bounds.top + kPad + lane * (laneH + kPad);
        CRect laneRect(bounds.left + kPad, y, bounds.left + kPad + laneW, y + laneH);

        auto activeId = ParamIDs::laneParam(lane, ParamIDs::kActive);
        double activeNorm = controller_->getParamNormalized(activeId);
        bool active = activeNorm > 0.5;
        bool selected = (lane == selectedLane);

        auto laneColor = kLaneColors[lane];
        uint8_t bgAlpha = selected ? 0x30 : 0x18;
        CColor bg =
            active ? CColor(laneColor.red, laneColor.green, laneColor.blue, bgAlpha) : CColor(0x2A, 0x2A, 0x36, 0xFF);
        context->setFillColor(bg);
        context->drawRect(laneRect, kDrawFilled);

        uint8_t borderAlpha = selected ? 0xC0 : 0x60;
        double borderWidth = selected ? 1.5 : 1.0;
        CColor border = active ? CColor(laneColor.red, laneColor.green, laneColor.blue, borderAlpha)
                               : CColor(0x3A, 0x3A, 0x48, 0xFF);
        context->setFrameColor(border);
        context->setLineWidth(borderWidth);
        context->drawRect(laneRect, kDrawStroked);

        CRect textRect = laneRect;
        textRect.left += 8;
        textRect.right = textRect.left + 80;
        context->setFont(font);
        context->setFontColor(active ? CColor(0xE8, 0xE8, 0xEC, 0xFF) : CColor(0x68, 0x68, 0x78, 0xFF));
        context->drawString(kLaneNames[lane], textRect, kLeftText);

        auto kotekanId = ParamIDs::laneParam(lane, ParamIDs::kKotekanSource);
        int kotekanSrc = static_cast<int>(std::round(controller_->getParamNormalized(kotekanId) * 8.0)) - 1;

        auto cellCountId = ParamIDs::laneCoreParam(lane, ParamIDs::kCoreCellCount);
        int cellCount = static_cast<int>(std::round(controller_->getParamNormalized(cellCountId) * 64.0));

        char laneNum[32];
        if (kotekanSrc >= 0 && kotekanSrc < kMaxLanes) {
            snprintf(laneNum, sizeof(laneNum), "L%d \xe2\x86\x90 L%d", lane + 1, kotekanSrc + 1);
        } else if (cellCount > 0) {
            snprintf(laneNum, sizeof(laneNum), "L%d \xe2\x99\xaa%d", lane + 1, cellCount);
        } else {
            snprintf(laneNum, sizeof(laneNum), "L%d", lane + 1);
        }
        CRect numRect = laneRect;
        numRect.right -= 8;
        numRect.left = numRect.right - 60;
        CColor numColor = (kotekanSrc >= 0) ? CColor(0x9B, 0x59, 0xB6, 0xC0)
                          : (cellCount > 0) ? CColor(0xE6, 0x7E, 0x22, 0xC0)
                                            : CColor(0x50, 0x50, 0x5C, 0xFF);
        context->setFontColor(numColor);
        context->drawString(laneNum, numRect, kRightText);

        auto probId = ParamIDs::laneParam(lane, ParamIDs::kProbability);
        double prob = controller_->getParamNormalized(probId);
        if (active && prob > 0.0) {
            double barW = (laneW - 140) * prob;
            CRect barRect(laneRect.left + 95, laneRect.top + 2, laneRect.left + 95 + barW, laneRect.bottom - 2);
            context->setFillColor(CColor(laneColor.red, laneColor.green, laneColor.blue, 0x60));
            context->drawRect(barRect, kDrawFilled);
        }

        if (active) {
            double phase = controller_->getParamNormalized(ParamIDs::lanePhaseOutput(lane));
            double indicatorR = (laneH - 4) * 0.4;
            double indicatorCx = laneRect.right - 40;
            double indicatorCy = y + laneH * 0.5;

            CRect bgCircle(indicatorCx - indicatorR, indicatorCy - indicatorR, indicatorCx + indicatorR,
                           indicatorCy + indicatorR);
            context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
            context->setLineWidth(1.5);
            context->drawEllipse(bgCircle, kDrawStroked);

            if (phase > 0.001) {
                constexpr double kTwoPi = 6.283185307179586;
                double angle = kTwoPi * phase - kTwoPi * 0.25;
                double dotX = indicatorCx + indicatorR * 0.75 * std::cos(angle);
                double dotY = indicatorCy + indicatorR * 0.75 * std::sin(angle);
                constexpr double kDotR = 2.5;
                CRect dot(dotX - kDotR, dotY - kDotR, dotX + kDotR, dotY + kDotR);
                context->setFillColor(laneColor);
                context->drawEllipse(dot, kDrawFilled);
            }
        }
    }

    setDirty(false);
}

VSTGUI::CRect LaneGridView::laneRect(int lane) const {
    auto bounds = getViewSize();
    double laneH = (bounds.getHeight() - kPad * (kMaxLanes + 1)) / kMaxLanes;
    double laneW = bounds.getWidth() - kPad * 2;
    double y = bounds.top + kPad + lane * (laneH + kPad);
    return VSTGUI::CRect(bounds.left + kPad, y, bounds.left + kPad + laneW, y + laneH);
}

int LaneGridView::hitTestLane(const VSTGUI::CPoint& where) const {
    for (int i = 0; i < kMaxLanes; ++i) {
        if (laneRect(i).pointInside(where))
            return i;
    }
    return -1;
}

double LaneGridView::probabilityFromX(int lane, VSTGUI::CCoord x) const {
    auto lr = laneRect(lane);
    double barLeft = lr.left + 95;
    double barW = lr.getWidth() - 140;
    double norm = (x - barLeft) / barW;
    if (norm < 0.0)
        norm = 0.0;
    if (norm > 1.0)
        norm = 1.0;
    return norm;
}

VSTGUI::CMouseEventResult LaneGridView::onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (!(buttons & VSTGUI::kLButton))
        return VSTGUI::kMouseEventNotHandled;

    int lane = hitTestLane(where);
    if (lane < 0)
        return VSTGUI::kMouseEventNotHandled;

    double selNorm = lane / 7.0;
    controller_->beginEdit(ParamIDs::kSelectedLane);
    controller_->setParamNormalized(ParamIDs::kSelectedLane, selNorm);
    controller_->performEdit(ParamIDs::kSelectedLane, selNorm);
    controller_->endEdit(ParamIDs::kSelectedLane);

    auto lr = laneRect(lane);
    double clickX = where.x - lr.left;

    if (clickX < 90) {
        auto activeId = ParamIDs::laneParam(lane, ParamIDs::kActive);
        double cur = controller_->getParamNormalized(activeId);
        double next = cur > 0.5 ? 0.0 : 1.0;
        controller_->beginEdit(activeId);
        controller_->setParamNormalized(activeId, next);
        controller_->performEdit(activeId, next);
        controller_->endEdit(activeId);
        invalid();
        return VSTGUI::kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    auto probId = ParamIDs::laneParam(lane, ParamIDs::kProbability);
    dragLane_ = lane;
    controller_->beginEdit(probId);
    double prob = probabilityFromX(lane, where.x);
    controller_->setParamNormalized(probId, prob);
    controller_->performEdit(probId, prob);
    invalid();
    return VSTGUI::kMouseEventHandled;
}

VSTGUI::CMouseEventResult LaneGridView::onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (dragLane_ < 0)
        return VSTGUI::kMouseEventNotHandled;

    auto probId = ParamIDs::laneParam(dragLane_, ParamIDs::kProbability);
    double prob = probabilityFromX(dragLane_, where.x);
    controller_->setParamNormalized(probId, prob);
    controller_->performEdit(probId, prob);
    invalid();
    return VSTGUI::kMouseEventHandled;
}

VSTGUI::CMouseEventResult LaneGridView::onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (dragLane_ < 0)
        return VSTGUI::kMouseEventNotHandled;

    auto probId = ParamIDs::laneParam(dragLane_, ParamIDs::kProbability);
    controller_->endEdit(probId);
    dragLane_ = -1;
    return VSTGUI::kMouseEventHandled;
}

} // namespace poly
