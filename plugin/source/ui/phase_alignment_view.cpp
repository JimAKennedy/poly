#include "phase_alignment_view.h"

#include <cmath>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../plugids.h"

namespace poly {

static constexpr double kTwoPi = 6.283185307179586;
static constexpr int kMaxLanes = 8;

PhaseAlignmentView::PhaseAlignmentView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setWantsFocus(false);
}

PhaseAlignmentView::~PhaseAlignmentView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool PhaseAlignmentView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = VSTGUI::makeOwned<VSTGUI::CVSTGUITimer>([this](VSTGUI::CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool PhaseAlignmentView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

void PhaseAlignmentView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    auto font = makeOwned<CFontDesc>("Arial", 8.0);

    constexpr double kPad = 8.0;
    constexpr double kLabelH = 14.0;
    double areaW = bounds.getWidth() - kPad * 2;
    double areaH = bounds.getHeight() - kPad * 2 - kLabelH;
    double cx = bounds.left + bounds.getWidth() * 0.5;
    double cy = bounds.top + kPad + areaH * 0.5;
    double maxRadius = std::min(areaW, areaH) * 0.45;

    static const CColor kLaneColors[] = {
        CColor(0x4A, 0x9E, 0xFF, 0xB0), CColor(0xF5, 0xA6, 0x23, 0xB0), CColor(0x27, 0xAE, 0x60, 0xB0),
        CColor(0xE7, 0x4C, 0x3C, 0xB0), CColor(0x9B, 0x59, 0xB6, 0xB0), CColor(0x1A, 0xBC, 0x9C, 0xB0),
        CColor(0xE6, 0x7E, 0x22, 0xB0), CColor(0x34, 0x98, 0xDB, 0xB0),
    };

    int activeLanes[kMaxLanes];
    int activeCount = 0;
    for (int lane = 0; lane < kMaxLanes; ++lane) {
        auto activeId = ParamIDs::laneParam(lane, ParamIDs::kActive);
        if (controller_->getParamNormalized(activeId) > 0.5) {
            activeLanes[activeCount++] = lane;
        }
    }

    if (activeCount == 0) {
        context->setFont(font);
        context->setFontColor(CColor(0x50, 0x50, 0x5C, 0xFF));
        CRect msgRect(bounds.left, cy - 8, bounds.right, cy + 8);
        context->drawString("No active lanes", msgRect, kCenterText);
        setDirty(false);
        return;
    }

    double minRadius = maxRadius * 0.3;
    double ringSpacing = activeCount > 1 ? (maxRadius - minRadius) / (activeCount - 1) : 0.0;

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));

    for (int i = 0; i < activeCount; ++i) {
        int lane = activeLanes[i];
        double radius = minRadius + i * ringSpacing;
        bool selected = (lane == selectedLane);

        double phraseLenNorm = controller_->getParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kPhraseLength));
        double phraseGapNorm = controller_->getParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kPhraseGap));
        bool hasPhrasing = phraseLenNorm > 0.001;

        if (hasPhrasing && (phraseLenNorm + phraseGapNorm) > 0.001) {
            double playFrac = phraseLenNorm / (phraseLenNorm + phraseGapNorm);

            double phrasePhase = controller_->getParamNormalized(ParamIDs::phrasePhaseOutput(lane));
            double startAngleDeg = 360.0 * phrasePhase - 90.0;

            double playArc = 360.0 * playFrac;
            double gapArc = 360.0 - playArc;

            CColor playColor = kLaneColors[lane];
            playColor.alpha = selected ? 0x70 : 0x30;
            context->setFrameColor(playColor);
            context->setLineWidth(selected ? 4.0 : 3.0);
            CRect arcRect(cx - radius, cy - radius, cx + radius, cy + radius);
            context->drawArc(arcRect, startAngleDeg, startAngleDeg + playArc, kDrawStroked);

            CColor gapColor(0x40, 0x40, 0x4C, selected ? 0x40 : 0x20);
            context->setFrameColor(gapColor);
            context->setLineWidth(selected ? 4.0 : 3.0);
            context->drawArc(arcRect, startAngleDeg + playArc, startAngleDeg + playArc + gapArc, kDrawStroked);
        } else {
            CColor ringColor = kLaneColors[lane];
            ringColor.alpha = selected ? 0x80 : 0x35;
            context->setFrameColor(ringColor);
            context->setLineWidth(selected ? 1.5 : 1.0);
            CRect ringRect(cx - radius, cy - radius, cx + radius, cy + radius);
            context->drawEllipse(ringRect, kDrawStroked);
        }

        double phase = controller_->getParamNormalized(ParamIDs::lanePhaseOutput(lane));
        double angle = kTwoPi * phase - kTwoPi * 0.25;
        double dotX = cx + radius * std::cos(angle);
        double dotY = cy + radius * std::sin(angle);

        double driftNorm = controller_->getParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kDriftRate));
        double driftRate = driftNorm * 8.0 - 4.0;
        if (std::abs(driftRate) > 0.01) {
            double trailArc = std::clamp(driftRate * 15.0, -90.0, 90.0);
            CColor trailColor = kLaneColors[lane];
            trailColor.alpha = selected ? 0x50 : 0x20;
            context->setFrameColor(trailColor);
            context->setLineWidth(selected ? 3.0 : 2.0);
            double startDeg = phase * 360.0 - 90.0;
            CRect arcRect(cx - radius, cy - radius, cx + radius, cy + radius);
            context->drawArc(arcRect, startDeg, startDeg + trailArc, kDrawStroked);

            double arrowAngle = kTwoPi * (phase + trailArc / 360.0) - kTwoPi * 0.25;
            double arrowX = cx + radius * std::cos(arrowAngle);
            double arrowY = cy + radius * std::sin(arrowAngle);
            double arrowR = selected ? 3.0 : 2.0;
            CRect arrow(arrowX - arrowR, arrowY - arrowR, arrowX + arrowR, arrowY + arrowR);
            trailColor.alpha = selected ? 0x80 : 0x50;
            context->setFillColor(trailColor);
            context->drawEllipse(arrow, kDrawFilled);
        }

        double dotR = selected ? 5.0 : 3.5;
        uint8_t dotAlpha = selected ? 0xFF : 0x90;
        CRect dot(dotX - dotR, dotY - dotR, dotX + dotR, dotY + dotR);
        context->setFillColor(CColor(kLaneColors[lane].red, kLaneColors[lane].green, kLaneColors[lane].blue, dotAlpha));
        context->drawEllipse(dot, kDrawFilled);

        char label[4];
        snprintf(label, sizeof(label), "L%d", lane + 1);
        double labelX = cx + (radius + 8) * std::cos(angle);
        double labelY = cy + (radius + 8) * std::sin(angle);
        CRect labelRect(labelX - 10, labelY - 6, labelX + 10, labelY + 6);
        context->setFont(font);
        uint8_t labelAlpha = selected ? 0xFF : 0x80;
        context->setFontColor(
            CColor(kLaneColors[lane].red, kLaneColors[lane].green, kLaneColors[lane].blue, labelAlpha));
        context->drawString(label, labelRect, kCenterText);
    }

    context->setFont(font);
    context->setFontColor(CColor(0x6A, 0x6A, 0x80, 0xFF));
    CRect titleRect(bounds.left, bounds.bottom - kLabelH - 2, bounds.right, bounds.bottom - 2);
    context->drawString("Phase Alignment", titleRect, kCenterText);

    setDirty(false);
}

} // namespace poly
