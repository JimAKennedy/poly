#include "lane_grid_view.h"

#include <cmath>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../plugids.h"

namespace poly {

LaneGridView::LaneGridView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setWantsFocus(false);
}

void LaneGridView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();
    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    constexpr int kMaxLanes = 8;
    constexpr double kPad = 4.0;
    double laneH = (bounds.getHeight() - kPad * (kMaxLanes + 1)) / kMaxLanes;
    double laneW = bounds.getWidth() - kPad * 2;

    static const char* kLaneNames[] = {"Kick", "Snare", "HH Closed", "HH Open", "Tom Hi", "Tom Lo", "Ride", "Crash"};

    auto font = makeOwned<CFontDesc>("Arial", 11.0);

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        double y = bounds.top + kPad + lane * (laneH + kPad);
        CRect laneRect(bounds.left + kPad, y, bounds.left + kPad + laneW, y + laneH);

        auto activeId = ParamIDs::laneParam(lane, ParamIDs::kActive);
        double activeNorm = controller_->getParamNormalized(activeId);
        bool active = activeNorm > 0.5;

        CColor bg = active ? CColor(0x4A, 0x9E, 0xFF, 0x18) : CColor(0x2A, 0x2A, 0x36, 0xFF);
        context->setFillColor(bg);
        context->drawRect(laneRect, kDrawFilled);

        CColor border = active ? CColor(0x4A, 0x9E, 0xFF, 0x60) : CColor(0x3A, 0x3A, 0x48, 0xFF);
        context->setFrameColor(border);
        context->setLineWidth(1.0);
        context->drawRect(laneRect, kDrawStroked);

        CRect textRect = laneRect;
        textRect.left += 8;
        textRect.right = textRect.left + 80;
        context->setFont(font);
        context->setFontColor(active ? CColor(0xE8, 0xE8, 0xEC, 0xFF) : CColor(0x68, 0x68, 0x78, 0xFF));
        context->drawString(kLaneNames[lane], textRect, kLeftText);

        char laneNum[8];
        snprintf(laneNum, sizeof(laneNum), "L%d", lane + 1);
        CRect numRect = laneRect;
        numRect.right -= 8;
        numRect.left = numRect.right - 30;
        context->setFontColor(CColor(0x50, 0x50, 0x5C, 0xFF));
        context->drawString(laneNum, numRect, kRightText);

        auto probId = ParamIDs::laneParam(lane, ParamIDs::kProbability);
        double prob = controller_->getParamNormalized(probId);
        if (active && prob > 0.0) {
            double barW = (laneW - 140) * prob;
            CRect barRect(laneRect.left + 95, laneRect.top + 2, laneRect.left + 95 + barW, laneRect.bottom - 2);
            context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0x60));
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
                context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0xFF));
                context->drawEllipse(dot, kDrawFilled);
            }
        }
    }

    setDirty(false);
}

} // namespace poly
