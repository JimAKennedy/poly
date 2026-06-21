#include "lane_grid_view.h"

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
    context->setFillColor(CColor(0x25, 0x25, 0x25, 0xFF));
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

        CColor bg = active ? CColor(0x4A, 0x9E, 0xFF, 0x30) : CColor(0x50, 0x50, 0x50, 0xFF);
        context->setFillColor(bg);
        context->drawRect(laneRect, kDrawFilled);

        CColor border = active ? CColor(0x4A, 0x9E, 0xFF, 0xCC) : CColor(0x70, 0x70, 0x70, 0xFF);
        context->setFrameColor(border);
        context->setLineWidth(1.0);
        context->drawRect(laneRect, kDrawStroked);

        CRect textRect = laneRect;
        textRect.left += 8;
        textRect.right = textRect.left + 80;
        context->setFont(font);
        context->setFontColor(active ? CColor(0xFF, 0xFF, 0xFF, 0xFF) : CColor(0xA0, 0xA0, 0xA0, 0xFF));
        context->drawString(kLaneNames[lane], textRect, kLeftText);

        char laneNum[8];
        snprintf(laneNum, sizeof(laneNum), "L%d", lane + 1);
        CRect numRect = laneRect;
        numRect.right -= 8;
        numRect.left = numRect.right - 30;
        context->setFontColor(CColor(0x80, 0x80, 0x80, 0xFF));
        context->drawString(laneNum, numRect, kRightText);

        auto probId = ParamIDs::laneParam(lane, ParamIDs::kProbability);
        double prob = controller_->getParamNormalized(probId);
        if (active && prob > 0.0) {
            double barW = (laneW - 120) * prob;
            CRect barRect(laneRect.left + 95, laneRect.top + 2, laneRect.left + 95 + barW, laneRect.bottom - 2);
            context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0x60));
            context->drawRect(barRect, kDrawFilled);
        }
    }

    setDirty(false);
}

} // namespace poly
