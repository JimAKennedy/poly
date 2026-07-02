#include "cross_rhythm_view.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../controller.h"
#include "../plugids.h"
#include "poly/euclidean.h"
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

    int activeLanes[kMaxLanes];
    int activeCount = 0;
    for (int lane = 0; lane < kMaxLanes; ++lane) {
        double activeNorm = controller_->getParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kActive));
        if (activeNorm > 0.5)
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

    const auto& cachedScene = controller_->activeScene();

    int laneSteps[kMaxLanes]{};
    int laneSubdiv[kMaxLanes]{};
    int laneCellCount[kMaxLanes]{};
    int laneHits[kMaxLanes]{};
    int laneRotation[kMaxLanes]{};
    bool laneTimeline[kMaxLanes]{};
    std::array<bool, kMaxSteps> lanePattern[kMaxLanes]{};
    int laneKotekanSrc[kMaxLanes]{};
    double laneSwing[kMaxLanes]{};
    double laneHumanize[kMaxLanes]{};
    for (int i = 0; i < activeCount; ++i) {
        int lane = activeLanes[i];
        laneSteps[i] =
            static_cast<int>(std::round(
                controller_->getParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSteps)) * 63.0)) +
            1;
        int subIdx = static_cast<int>(std::round(
            controller_->getParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreSubdivision)) * 4.0));
        laneSubdiv[i] = 1 << std::clamp(subIdx, 0, 4);
        laneCellCount[i] = static_cast<int>(std::round(
            controller_->getParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreCellCount)) * 64.0));
        laneHits[i] = static_cast<int>(
            std::round(controller_->getParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreHits)) * 64.0));
        laneRotation[i] = static_cast<int>(
            std::round(controller_->getParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreRotation)) * 63.0));
        laneTimeline[i] =
            controller_->getParamNormalized(ParamIDs::laneCoreParam(lane, ParamIDs::kCoreTimeline)) >= 0.5;

        double kotekanNorm = controller_->getParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kKotekanSource));
        laneKotekanSrc[i] = static_cast<int>(std::round(kotekanNorm * 8.0)) - 1;

        laneSwing[i] = controller_->getParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kSwingAmount));
        laneHumanize[i] = controller_->getParamNormalized(ParamIDs::laneParam(lane, ParamIDs::kHumanizeMs));

        lanePattern[i].fill(false);
        if (laneTimeline[i]) {
            const auto& fixedPat = cachedScene.lanes[lane].fixedPattern;
            for (int s = 0; s < laneSteps[i] && s < kMaxSteps; ++s)
                lanePattern[i][s] = fixedPat[s];
        } else {
            euclidean(laneHits[i], laneSteps[i], laneRotation[i], lanePattern[i]);
        }
    }

    for (int i = 0; i < activeCount; ++i) {
        int srcLane = laneKotekanSrc[i];
        if (srcLane < 0 || srcLane >= kMaxLanes || laneTimeline[i])
            continue;
        int srcIdx = -1;
        for (int j = 0; j < activeCount; ++j) {
            if (activeLanes[j] == srcLane) {
                srcIdx = j;
                break;
            }
        }
        if (srcIdx < 0)
            continue;
        if (laneKotekanSrc[srcIdx] == activeLanes[i])
            continue;
        std::array<bool, kMaxSteps> srcPattern{};
        euclidean(laneHits[srcIdx], laneSteps[srcIdx], laneRotation[srcIdx], srcPattern);
        for (int s = 0; s < laneSteps[i] && s < laneSteps[srcIdx]; ++s)
            lanePattern[i][s] = !srcPattern[s];
        for (int s = laneSteps[srcIdx]; s < laneSteps[i]; ++s)
            lanePattern[i][s] = true;
    }

    double cyclePpqs[kMaxLanes]{};
    for (int i = 0; i < activeCount; ++i) {
        double basePpq = 4.0 / laneSubdiv[i];
        if (laneCellCount[i] > 0) {
            const auto& cellSizes = cachedScene.lanes[activeLanes[i]].cellSizes;
            double total = 0.0;
            for (int c = 0; c < laneCellCount[i] && c < kMaxSteps; ++c)
                total += std::max(1, cellSizes[c]) * basePpq;
            cyclePpqs[i] = total;
        } else {
            cyclePpqs[i] = laneSteps[i] * basePpq;
        }
        if (cyclePpqs[i] <= 0.0)
            cyclePpqs[i] = 4.0;
    }

    constexpr double kBarPpq = 4.0;
    constexpr int kMaxBars = 8;
    constexpr int kMinBars = 1;

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

    // Collect unique convergence PPQ positions for lookahead
    static constexpr int kMaxConvPoints = 64;
    double convPpqs[kMaxConvPoints]{};
    int convCount = 0;
    for (int b = 0; b < boundaryCount; ++b) {
        if (!isConvergence[b])
            continue;
        bool dup = false;
        for (int c = 0; c < convCount; ++c) {
            if (std::abs(convPpqs[c] - allBoundaries[b].ppq) < kConvergenceTol) {
                dup = true;
                break;
            }
        }
        if (!dup && convCount < kMaxConvPoints)
            convPpqs[convCount++] = allBoundaries[b].ppq;
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

        double cyclePpq = cyclePpqs[i];
        double basePpq = 4.0 / laneSubdiv[i];
        int cellCount = laneCellCount[i];
        int steps = laneSteps[i];
        double swing = laneSwing[i];
        double humanize = laneHumanize[i];

        auto drawStepMarker = [&](double stepPpqPos, int step, bool isHit) {
            double swingOffset = 0.0;
            if (swing > 0.01 && (step % 2) == 1) {
                swingOffset = swing * basePpq * (1.0 / 3.0);
            }
            double displayPpq = stepPpqPos + swingOffset;
            double x = timelineLeft + (displayPpq / spanPpq) * timelineW;
            if (x < timelineLeft || x > timelineRight)
                return;

            if (isHit) {
                constexpr double kDotR = 3.0;
                CRect dot(x - kDotR, rowMid - kDotR, x + kDotR, rowMid + kDotR);
                CColor dotColor = kLaneColors[lane];
                dotColor.alpha = 0xE0;
                context->setFillColor(dotColor);
                context->drawEllipse(dot, kDrawFilled);

                if (humanize > 0.01) {
                    double humanizePpx = humanize * 50.0 / 60000.0 * 120.0;
                    double whiskerPx = (humanizePpx / spanPpq) * timelineW;
                    whiskerPx = std::min(whiskerPx, 8.0);
                    CColor whiskerColor = kLaneColors[lane];
                    whiskerColor.alpha = 0x50;
                    context->setFrameColor(whiskerColor);
                    context->setLineWidth(1.0);
                    double wy = rowMid + kDotR + 2;
                    context->drawLine(CPoint(x - whiskerPx, wy), CPoint(x + whiskerPx, wy));
                    context->drawLine(CPoint(x - whiskerPx, wy - 1), CPoint(x - whiskerPx, wy + 1));
                    context->drawLine(CPoint(x + whiskerPx, wy - 1), CPoint(x + whiskerPx, wy + 1));
                }
            } else {
                double tickH = laneH * 0.15;
                CColor tickColor = kLaneColors[lane];
                tickColor.alpha = 0x25;
                context->setFrameColor(tickColor);
                context->setLineWidth(1.0);
                context->drawLine(CPoint(x, rowMid - tickH), CPoint(x, rowMid + tickH));
            }
        };

        for (double cycleStart = 0.0; cycleStart < spanPpq; cycleStart += cyclePpq) {
            if (cellCount > 0) {
                const auto& cellSizes = cachedScene.lanes[lane].cellSizes;
                double cellPos = 0.0;
                for (int c = 0; c < cellCount && c < kMaxSteps; ++c) {
                    double stepPpqPos = cycleStart + cellPos;
                    if (stepPpqPos >= 0.0 && stepPpqPos <= spanPpq) {
                        bool isHit = (c < steps) && lanePattern[i][c];
                        drawStepMarker(stepPpqPos, c, isHit);
                    }
                    cellPos += std::max(1, cellSizes[c]) * basePpq;
                }
            } else {
                for (int step = 0; step < steps; ++step) {
                    double stepPpqPos = cycleStart + step * basePpq;
                    if (stepPpqPos >= 0.0 && stepPpqPos <= spanPpq) {
                        bool isHit = lanePattern[i][step];
                        drawStepMarker(stepPpqPos, step, isHit);
                    }
                }
            }
        }

        // Ghost dots: draw kotekan source lane's hits as hollow circles on this row
        int srcLane = laneKotekanSrc[i];
        if (srcLane >= 0 && srcLane < kMaxLanes) {
            int srcIdx = -1;
            for (int j = 0; j < activeCount; ++j) {
                if (activeLanes[j] == srcLane) {
                    srcIdx = j;
                    break;
                }
            }
            if (srcIdx >= 0) {
                double srcCyclePpq = cyclePpqs[srcIdx];
                double srcBasePpq = 4.0 / laneSubdiv[srcIdx];
                int srcCellCount = laneCellCount[srcIdx];
                int srcSteps = laneSteps[srcIdx];

                CColor ghostColor = kLaneColors[srcLane];
                ghostColor.alpha = 0x90;

                for (double cycleStart = 0.0; cycleStart < spanPpq; cycleStart += srcCyclePpq) {
                    if (srcCellCount > 0) {
                        const auto& srcCellSizes = cachedScene.lanes[srcLane].cellSizes;
                        double cellPos = 0.0;
                        for (int c = 0; c < srcCellCount && c < kMaxSteps; ++c) {
                            double stepPpqPos = cycleStart + cellPos;
                            if (stepPpqPos >= 0.0 && stepPpqPos <= spanPpq && (c < srcSteps) &&
                                lanePattern[srcIdx][c]) {
                                double x = timelineLeft + (stepPpqPos / spanPpq) * timelineW;
                                constexpr double kGhostR = 3.5;
                                CRect ghost(x - kGhostR, rowMid - kGhostR, x + kGhostR, rowMid + kGhostR);
                                context->setFrameColor(ghostColor);
                                context->setLineWidth(1.5);
                                context->drawEllipse(ghost, kDrawStroked);
                            }
                            cellPos += std::max(1, srcCellSizes[c]) * srcBasePpq;
                        }
                    } else {
                        for (int step = 0; step < srcSteps; ++step) {
                            double stepPpqPos = cycleStart + step * srcBasePpq;
                            if (stepPpqPos >= 0.0 && stepPpqPos <= spanPpq && lanePattern[srcIdx][step]) {
                                double x = timelineLeft + (stepPpqPos / spanPpq) * timelineW;
                                constexpr double kGhostR = 3.5;
                                CRect ghost(x - kGhostR, rowMid - kGhostR, x + kGhostR, rowMid + kGhostR);
                                context->setFrameColor(ghostColor);
                                context->setLineWidth(1.5);
                                context->drawEllipse(ghost, kDrawStroked);
                            }
                        }
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

    // Convergence lookahead: show countdown to next convergence from playhead
    double ppqNorm = controller_->getParamNormalized(ParamIDs::kTransportPpqOutput);
    double currentPpq = ppqNorm * 128.0;
    double playheadPpq = std::fmod(currentPpq, spanPpq);
    if (playheadPpq < 0.0)
        playheadPpq += spanPpq;

    if (convCount > 0 && (ppqNorm > 0.0 || activeCount > 0)) {
        double nextConvPpq = -1.0;
        for (int c = 0; c < convCount; ++c) {
            if (convPpqs[c] > playheadPpq + kConvergenceTol) {
                if (nextConvPpq < 0.0 || convPpqs[c] < nextConvPpq)
                    nextConvPpq = convPpqs[c];
            }
        }
        if (nextConvPpq < 0.0) {
            for (int c = 0; c < convCount; ++c) {
                double wrapped = convPpqs[c] + spanPpq;
                if (nextConvPpq < 0.0 || wrapped < nextConvPpq)
                    nextConvPpq = wrapped;
            }
        }

        if (nextConvPpq >= 0.0) {
            double beatsAway = nextConvPpq - playheadPpq;
            if (beatsAway < 0.0)
                beatsAway += spanPpq;
            double dispPpq = std::fmod(nextConvPpq, spanPpq);
            if (dispPpq < 0.0)
                dispPpq += spanPpq;
            double convX = timelineLeft + (dispPpq / spanPpq) * timelineW;

            char countdownLabel[16];
            if (beatsAway < 1.0)
                snprintf(countdownLabel, sizeof(countdownLabel), "<1");
            else
                snprintf(countdownLabel, sizeof(countdownLabel), "%.0f", std::ceil(beatsAway));

            auto countdownFont = makeOwned<CFontDesc>("Arial", 7.0);
            context->setFont(countdownFont);
            context->setFontColor(CColor(0xFF, 0xD7, 0x00, 0xA0));
            CRect cdRect(convX - 10, laneAreaBot + 1, convX + 10, laneAreaBot + 11);
            context->drawString(countdownLabel, cdRect, kCenterText);
        }
    }

    // Playhead
    if (ppqNorm > 0.0 || activeCount > 0) {
        double playheadX = timelineLeft + (playheadPpq / spanPpq) * timelineW;

        context->setFrameColor(CColor(0xFF, 0xFF, 0xFF, 0xA0));
        context->setLineWidth(1.5);
        context->drawLine(CPoint(playheadX, laneAreaTop - 2), CPoint(playheadX, laneAreaBot));

        constexpr double kTriH = 4.0;
        constexpr double kTriW = 3.5;
        CGraphicsPath* tri = context->createGraphicsPath();
        if (tri) {
            tri->beginSubpath(CPoint(playheadX - kTriW, laneAreaTop - 2 - kTriH));
            tri->addLine(CPoint(playheadX + kTriW, laneAreaTop - 2 - kTriH));
            tri->addLine(CPoint(playheadX, laneAreaTop - 2));
            tri->closeSubpath();
            context->setFillColor(CColor(0xFF, 0xFF, 0xFF, 0xA0));
            context->drawGraphicsPath(tri, CDrawContext::kPathFilled);
            tri->forget();
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
