#include "velocity_view.h"

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../plugids.h"

namespace poly {

VelocityView::VelocityView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setWantsFocus(false);
}

VelocityView::~VelocityView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool VelocityView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = VSTGUI::makeOwned<VSTGUI::CVSTGUITimer>([this](VSTGUI::CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool VelocityView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

static const VSTGUI::CColor kLaneColors[] = {
    VSTGUI::CColor(0x4A, 0x9E, 0xFF, 0xFF), VSTGUI::CColor(0xF5, 0xA6, 0x23, 0xFF),
    VSTGUI::CColor(0x27, 0xAE, 0x60, 0xFF), VSTGUI::CColor(0xE7, 0x4C, 0x3C, 0xFF),
    VSTGUI::CColor(0x9B, 0x59, 0xB6, 0xFF), VSTGUI::CColor(0x1A, 0xBC, 0x9C, 0xFF),
    VSTGUI::CColor(0xE6, 0x7E, 0x22, 0xFF), VSTGUI::CColor(0x34, 0x98, 0xDB, 0xFF),
};

void VelocityView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    constexpr int kMaxLanes = 8;
    constexpr double kPad = 6.0;
    double barW = (bounds.getWidth() - kPad * (kMaxLanes + 1)) / kMaxLanes;
    double maxBarH = bounds.getHeight() - kPad * 2 - 14;

    auto font = makeOwned<CFontDesc>("Arial", 9.0);

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        double x = bounds.left + kPad + lane * (barW + kPad);

        auto activeId = ParamIDs::laneParam(lane, ParamIDs::kActive);
        bool active = controller_->getParamNormalized(activeId) > 0.5;
        bool selected = (lane == selectedLane);

        double vel = 0.0;
        if (active) {
            auto velId = ParamIDs::velocityOutput(lane);
            vel = controller_->getParamNormalized(velId);
        }

        CRect bgRect(x, bounds.top + kPad, x + barW, bounds.top + kPad + maxBarH);
        context->setFillColor(CColor(0x2A, 0x2A, 0x36, 0xFF));
        context->drawRect(bgRect, kDrawFilled);

        if (vel > 0.001) {
            double barH = maxBarH * vel;
            CRect barRect(x, bgRect.bottom - barH, x + barW, bgRect.bottom);

            auto laneColor = kLaneColors[lane];
            uint8_t a = static_cast<uint8_t>(0x40 + vel * 0x8F);
            context->setFillColor(CColor(laneColor.red, laneColor.green, laneColor.blue, a));
            context->drawRect(barRect, kDrawFilled);
        }

        if (selected) {
            context->setFrameColor(
                CColor(kLaneColors[lane].red, kLaneColors[lane].green, kLaneColors[lane].blue, 0xA0));
            context->setLineWidth(1.5);
            context->drawRect(bgRect, kDrawStroked);
        }

        char label[4];
        snprintf(label, sizeof(label), "L%d", lane + 1);
        CRect labelRect(x, bgRect.bottom + 1, x + barW, bounds.bottom - 1);
        context->setFont(font);
        CColor labelColor = selected
                                ? CColor(kLaneColors[lane].red, kLaneColors[lane].green, kLaneColors[lane].blue, 0xFF)
                                : (active ? CColor(0xC0, 0xC0, 0xC8, 0xFF) : CColor(0x50, 0x50, 0x5C, 0xFF));
        context->setFontColor(labelColor);
        context->drawString(label, labelRect, kCenterText);
    }

    setDirty(false);
}

} // namespace poly
