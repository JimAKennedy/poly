#include "envelope_curve_view.h"

#include <cmath>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/cgraphicspath.h"

#include "../plugids.h"

namespace poly {

static constexpr double kTwoPi = 6.283185307179586;
static constexpr int kMaxLanes = 8;
static constexpr int kCurvePoints = 64;

EnvelopeCurveView::EnvelopeCurveView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setWantsFocus(false);
}

void EnvelopeCurveView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();

    context->setFillColor(CColor(0x25, 0x25, 0x25, 0xFF));
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

    context->setFrameColor(CColor(0x40, 0x40, 0x40, 0xFF));
    context->setLineWidth(1.0);
    CRect plotFrame(plotLeft, plotTop, plotRight, plotBottom);
    context->drawRect(plotFrame, kDrawStroked);

    int activeLanes = 0;
    for (int lane = 0; lane < kMaxLanes; ++lane) {
        auto activeId = ParamIDs::laneParam(lane, ParamIDs::kActive);
        if (controller_->getParamNormalized(activeId) > 0.5)
            activeLanes++;
    }

    static const CColor kLaneColors[] = {
        CColor(0x4A, 0x9E, 0xFF, 0xB0), CColor(0xF5, 0xA6, 0x23, 0xB0), CColor(0x27, 0xAE, 0x60, 0xB0),
        CColor(0xE7, 0x4C, 0x3C, 0xB0), CColor(0x9B, 0x59, 0xB6, 0xB0), CColor(0x1A, 0xBC, 0x9C, 0xB0),
        CColor(0xE6, 0x7E, 0x22, 0xB0), CColor(0x34, 0x98, 0xDB, 0xB0),
    };

    for (int lane = 0; lane < kMaxLanes; ++lane) {
        auto activeId = ParamIDs::laneParam(lane, ParamIDs::kActive);
        if (controller_->getParamNormalized(activeId) <= 0.5)
            continue;

        double envVal = controller_->getParamNormalized(ParamIDs::envelopeValueOutput(lane));
        double phase = controller_->getParamNormalized(ParamIDs::lanePhaseOutput(lane));

        auto* path = context->createGraphicsPath();
        if (!path)
            continue;

        for (int i = 0; i <= kCurvePoints; ++i) {
            double t = static_cast<double>(i) / kCurvePoints;
            double val = 0.5 * (1.0 + std::sin(kTwoPi * t));
            double x = plotLeft + t * plotW;
            double y = plotBottom - val * plotH;

            if (i == 0)
                path->beginSubpath(CPoint(x, y));
            else
                path->addLine(CPoint(x, y));
        }

        context->setFrameColor(kLaneColors[lane]);
        context->setLineWidth(1.5);
        context->drawGraphicsPath(path, CDrawContext::kPathStroked);
        path->forget();

        if (phase > 0.001 || envVal > 0.001) {
            double markerX = plotLeft + phase * plotW;
            double markerY = plotBottom - envVal * plotH;

            auto* linePath = context->createGraphicsPath();
            if (linePath) {
                linePath->beginSubpath(CPoint(markerX, plotTop));
                linePath->addLine(CPoint(markerX, plotBottom));
                context->setFrameColor(kLaneColors[lane]);
                context->setLineWidth(1.0);
                context->drawGraphicsPath(linePath, CDrawContext::kPathStroked);
                linePath->forget();
            }

            CRect dot(markerX - 4, markerY - 4, markerX + 4, markerY + 4);
            context->setFillColor(CColor(kLaneColors[lane].red, kLaneColors[lane].green, kLaneColors[lane].blue, 0xFF));
            context->drawEllipse(dot, kDrawFilled);
        }
    }

    context->setFont(font);
    context->setFontColor(CColor(0xA0, 0xA0, 0xA0, 0xFF));
    CRect labelRect(plotLeft, plotBottom + 2, plotRight, bounds.bottom - 2);
    context->drawString("Envelope", labelRect, kCenterText);

    setDirty(false);
}

} // namespace poly
