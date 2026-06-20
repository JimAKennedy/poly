#include "velocity_view.h"
#include "../plugids.h"

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

namespace poly {

VelocityView::VelocityView(const VSTGUI::CRect& size,
                           Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setWantsFocus(false);
}

void VelocityView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();

    context->setFillColor(CColor(0x25, 0x25, 0x25, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    constexpr int kMaxLanes = 8;
    constexpr double kPad = 6.0;
    double barW = (bounds.getWidth() - kPad * (kMaxLanes + 1)) / kMaxLanes;
    double maxBarH = bounds.getHeight() - kPad * 2 - 14;

    auto font = makeOwned<CFontDesc>("Arial", 9.0);

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        double x = bounds.left + kPad + lane * (barW + kPad);

        auto activeId = ParamIDs::laneParam(lane, ParamIDs::kActive);
        bool active = controller_->getParamNormalized(activeId) > 0.5;

        double vel = 0.0;
        if (active) {
            auto velId = ParamIDs::velocityOutput(lane);
            vel = controller_->getParamNormalized(velId);
        }

        CRect bgRect(x, bounds.top + kPad,
                     x + barW, bounds.top + kPad + maxBarH);
        context->setFillColor(CColor(0x50, 0x50, 0x50, 0xFF));
        context->drawRect(bgRect, kDrawFilled);

        if (vel > 0.001) {
            double barH = maxBarH * vel;
            CRect barRect(x, bgRect.bottom - barH,
                          x + barW, bgRect.bottom);

            uint8_t a = static_cast<uint8_t>(0x40 + vel * 0x8F);
            context->setFillColor(CColor(0x27, 0xAE, 0x60, a));
            context->drawRect(barRect, kDrawFilled);
        }

        char label[4];
        snprintf(label, sizeof(label), "L%d", lane + 1);
        CRect labelRect(x, bgRect.bottom + 1, x + barW,
                        bounds.bottom - 1);
        context->setFont(font);
        context->setFontColor(active ? CColor(0xCC, 0xCC, 0xCC, 0xFF)
                                     : CColor(0x80, 0x80, 0x80, 0xFF));
        context->drawString(label, labelRect, kCenterText);
    }

    setDirty(false);
}

} // namespace poly
