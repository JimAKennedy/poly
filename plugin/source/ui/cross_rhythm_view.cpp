#include "cross_rhythm_view.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../controller.h"
#include "../plugids.h"
#include "poly/types.h"

namespace poly {

using namespace VSTGUI;

static const CColor kLaneColors[] = {
    CColor(0x4A, 0x9E, 0xFF, 0xB0), CColor(0xF5, 0xA6, 0x23, 0xB0), CColor(0x27, 0xAE, 0x60, 0xB0),
    CColor(0xE7, 0x4C, 0x3C, 0xB0), CColor(0x9B, 0x59, 0xB6, 0xB0), CColor(0x1A, 0xBC, 0x9C, 0xB0),
    CColor(0xE6, 0x7E, 0x22, 0xB0), CColor(0x34, 0x98, 0xDB, 0xB0),
};

static int64_t gcd64(int64_t a, int64_t b) {
    while (b != 0) {
        int64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

static int64_t lcm64(int64_t a, int64_t b) {
    if (a == 0 || b == 0)
        return 0;
    return (a / gcd64(a, b)) * b;
}

CrossRhythmView::CrossRhythmView(const CRect& size, PolyController* controller) : CView(size), controller_(controller) {
    setWantsFocus(false);
}

CrossRhythmView::~CrossRhythmView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool CrossRhythmView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = makeOwned<CVSTGUITimer>([this](CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool CrossRhythmView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

void CrossRhythmView::draw(CDrawContext* context) {
    auto bounds = getViewSize();

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    auto font = makeOwned<CFontDesc>("Arial", 8.0);

    const auto& gs = controller_->cachedState().sceneA;

    int activeLanes[kMaxLanes];
    int activeCount = 0;
    for (int lane = 0; lane < gs.activeLaneCount && lane < kMaxLanes; ++lane) {
        if (gs.lanes[lane].active)
            activeLanes[activeCount++] = lane;
    }

    if (activeCount == 0) {
        context->setFont(font);
        context->setFontColor(CColor(0x50, 0x50, 0x5C, 0xFF));
        CRect msgRect(bounds.left, bounds.top + bounds.getHeight() * 0.5 - 8, bounds.right,
                      bounds.top + bounds.getHeight() * 0.5 + 8);
        context->drawString("No active lanes", msgRect, kCenterText);
        setDirty(false);
        return;
    }

    constexpr double kPad = 10.0;
    constexpr double kLabelW = 24.0;
    constexpr double kTitleH = 14.0;
    constexpr double kHeaderH = 14.0;

    double timelineLeft = bounds.left + kPad + kLabelW;
    double timelineRight = bounds.right - kPad;
    double timelineW = timelineRight - timelineLeft;

    double laneAreaTop = bounds.top + kPad + kHeaderH;
    double laneAreaBot = bounds.bottom - kPad - kTitleH;
    double laneH = activeCount > 0 ? (laneAreaBot - laneAreaTop) / activeCount : 20.0;
    laneH = std::min(laneH, 24.0);

    // Compute cycle lengths in PPQ for each active lane
    double cyclePpqs[kMaxLanes]{};
    for (int i = 0; i < activeCount; ++i) {
        const auto& cfg = gs.lanes[activeLanes[i]];
        double basePpq = 4.0 / cfg.cycle.subdivision;
        if (cfg.cellCount > 0) {
            double total = 0.0;
            for (int c = 0; c < cfg.cellCount && c < kMaxSteps; ++c)
                total += cfg.cellSizes[c] * basePpq;
            cyclePpqs[i] = total;
        } else {
            cyclePpqs[i] = cfg.cycle.steps * basePpq;
        }
        if (cyclePpqs[i] <= 0.0)
            cyclePpqs[i] = 4.0;
    }

    // Determine span: show enough bars to see at least one full LCM period, capped at 8 bars
    constexpr double kBarPpq = 4.0;
    constexpr int kMaxBars = 8;
    constexpr int kMinBars = 2;

    // Find LCM of cycle lengths (approximate as rational multiples of basePpq)
    int64_t lcmTicks = 1;
    for (int i = 0; i < activeCount; ++i) {
        int64_t ticks = static_cast<int64_t>(std::round(cyclePpqs[i] * 960.0));
        if (ticks > 0)
            lcmTicks = lcm64(lcmTicks, ticks);
    }
    double lcmPpq = lcmTicks / 960.0;
    int barsNeeded = static_cast<int>(std::ceil(lcmPpq / kBarPpq));
    int displayBars = std::clamp(barsNeeded, kMinBars, kMaxBars);
    double spanPpq = displayBars * kBarPpq;

    // Draw bar lines
    context->setFrameColor(CColor(0x30, 0x30, 0x3C, 0xFF));
    context->setLineWidth(1.0);
    for (int bar = 0; bar <= displayBars; ++bar) {
        double x = timelineLeft + (bar * kBarPpq / spanPpq) * timelineW;
        CPoint top(x, laneAreaTop - 2);
        CPoint bot(x, laneAreaBot);
        context->drawLine(top, bot);
    }

    // Draw bar numbers
    context->setFont(font);
    context->setFontColor(CColor(0x50, 0x50, 0x5C, 0xFF));
    for (int bar = 0; bar < displayBars; ++bar) {
        double x = timelineLeft + (bar * kBarPpq / spanPpq) * timelineW;
        char label[8];
        snprintf(label, sizeof(label), "%d", bar + 1);
        CRect labelRect(x, bounds.top + kPad, x + 20, bounds.top + kPad + kHeaderH);
        context->drawString(label, labelRect);
    }

    // Collect all cycle boundary positions (convergence detection)
    struct BoundaryInfo {
        double ppq;
        int laneIdx;
    };
    static constexpr int kMaxBoundaries = 512;
    BoundaryInfo allBoundaries[kMaxBoundaries];
    int boundaryCount = 0;

    for (int i = 0; i < activeCount; ++i) {
        double cyclePpq = cyclePpqs[i];
        for (double pos = 0.0; pos <= spanPpq + 0.001; pos += cyclePpq) {
            if (boundaryCount < kMaxBoundaries) {
                allBoundaries[boundaryCount++] = {pos, i};
            }
        }
    }

    // Find convergence points: positions where 2+ lanes have boundaries within tolerance
    constexpr double kConvergenceTol = 0.01;
    bool isConvergence[kMaxBoundaries]{};
    for (int a = 0; a < boundaryCount; ++a) {
        for (int b = a + 1; b < boundaryCount; ++b) {
            if (allBoundaries[a].laneIdx != allBoundaries[b].laneIdx &&
                std::abs(allBoundaries[a].ppq - allBoundaries[b].ppq) < kConvergenceTol) {
                isConvergence[a] = true;
                isConvergence[b] = true;
            }
        }
    }

    // Draw each lane's row
    for (int i = 0; i < activeCount; ++i) {
        int lane = activeLanes[i];
        double rowTop = laneAreaTop + i * laneH;
        double rowMid = rowTop + laneH * 0.5;

        // Lane label
        char lbl[4];
        snprintf(lbl, sizeof(lbl), "L%d", lane + 1);
        context->setFont(font);
        context->setFontColor(kLaneColors[lane]);
        CRect lblRect(bounds.left + kPad, rowTop, timelineLeft - 2, rowTop + laneH);
        context->drawString(lbl, lblRect, kCenterText);

        // Lane baseline
        CColor lineColor = kLaneColors[lane];
        lineColor.alpha = 0x40;
        context->setFrameColor(lineColor);
        context->setLineWidth(1.0);
        context->drawLine(CPoint(timelineLeft, rowMid), CPoint(timelineRight, rowMid));

        // Draw step/cell markers within cycles
        const auto& cfg = gs.lanes[lane];
        double cyclePpq = cyclePpqs[i];
        double basePpq = 4.0 / cfg.cycle.subdivision;

        for (double cycleStart = 0.0; cycleStart < spanPpq; cycleStart += cyclePpq) {
            if (cfg.cellCount > 0) {
                double cellPos = 0.0;
                for (int c = 0; c < cfg.cellCount && c < kMaxSteps; ++c) {
                    double stepPpq = cycleStart + cellPos;
                    if (stepPpq >= 0.0 && stepPpq <= spanPpq) {
                        double x = timelineLeft + (stepPpq / spanPpq) * timelineW;
                        double tickH = (c == 0) ? laneH * 0.4 : laneH * 0.25;
                        CColor tickColor = kLaneColors[lane];
                        tickColor.alpha = (c == 0) ? 0xC0 : 0x60;
                        context->setFrameColor(tickColor);
                        context->setLineWidth((c == 0) ? 1.5 : 1.0);
                        context->drawLine(CPoint(x, rowMid - tickH), CPoint(x, rowMid + tickH));
                    }
                    cellPos += cfg.cellSizes[c] * basePpq;
                }
            } else {
                for (int step = 0; step < cfg.cycle.steps; ++step) {
                    double stepPpq = cycleStart + step * basePpq;
                    if (stepPpq >= 0.0 && stepPpq <= spanPpq) {
                        double x = timelineLeft + (stepPpq / spanPpq) * timelineW;
                        double tickH = (step == 0) ? laneH * 0.4 : laneH * 0.2;
                        CColor tickColor = kLaneColors[lane];
                        tickColor.alpha = (step == 0) ? 0xC0 : 0x40;
                        context->setFrameColor(tickColor);
                        context->setLineWidth((step == 0) ? 1.5 : 1.0);
                        context->drawLine(CPoint(x, rowMid - tickH), CPoint(x, rowMid + tickH));
                    }
                }
            }
        }
    }

    // Draw convergence markers (on top of everything)
    for (int b = 0; b < boundaryCount; ++b) {
        if (!isConvergence[b])
            continue;
        double x = timelineLeft + (allBoundaries[b].ppq / spanPpq) * timelineW;
        if (x < timelineLeft || x > timelineRight)
            continue;

        // Vertical highlight line across all lanes
        CColor convColor(0xFF, 0xD7, 0x00, 0x30);
        context->setFrameColor(convColor);
        context->setLineWidth(2.0);
        context->drawLine(CPoint(x, laneAreaTop), CPoint(x, laneAreaBot));

        // Diamond marker at the convergence point on this lane's row
        int laneIdx = allBoundaries[b].laneIdx;
        double rowMid = laneAreaTop + laneIdx * laneH + laneH * 0.5;
        constexpr double kDiamond = 3.5;
        CGraphicsPath* path = context->createGraphicsPath();
        if (path) {
            path->beginSubpath(CPoint(x, rowMid - kDiamond));
            path->addLine(CPoint(x + kDiamond, rowMid));
            path->addLine(CPoint(x, rowMid + kDiamond));
            path->addLine(CPoint(x - kDiamond, rowMid));
            path->closeSubpath();
            context->setFillColor(CColor(0xFF, 0xD7, 0x00, 0xC0));
            context->drawGraphicsPath(path, CDrawContext::kPathFilled);
            path->forget();
        }
    }

    // Title
    context->setFont(font);
    context->setFontColor(CColor(0x6A, 0x6A, 0x80, 0xFF));
    CRect titleRect(bounds.left, bounds.bottom - kTitleH - 2, bounds.right, bounds.bottom - 2);
    context->drawString("Cross-Rhythm", titleRect, kCenterText);

    setDirty(false);
}

} // namespace poly
