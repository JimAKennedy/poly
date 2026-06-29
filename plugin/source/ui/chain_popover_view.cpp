#include "chain_popover_view.h"

#include <algorithm>
#include <cstdio>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/cframe.h"

#include "../plugids.h"
#include "poly/scene.h"

namespace poly {

namespace {
static constexpr double kBtnW = 50.0;
static constexpr double kBtnH = 24.0;
static constexpr double kSmallBtnW = 28.0;
static constexpr double kSceneBtnW = 40.0;
static constexpr double kMorphBtnW = 52.0;
} // namespace

ChainPopoverView::ChainPopoverView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

void ChainPopoverView::pushParam(Steinberg::Vst::ParamID id, double value) {
    controller_->beginEdit(id);
    controller_->setParamNormalized(id, value);
    controller_->performEdit(id, value);
    controller_->endEdit(id);
}

int ChainPopoverView::getEntryCount() const {
    return static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kChainEntryCount) * kMaxChainEntries));
}

int ChainPopoverView::getChainMode() const {
    return static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kChainMode) * 2.0));
}

bool ChainPopoverView::getChainEnabled() const {
    return controller_->getParamNormalized(ParamIDs::kChainEnabled) > 0.5;
}

int ChainPopoverView::getEntryScene(int entry) const {
    return static_cast<int>(std::round(
        controller_->getParamNormalized(ParamIDs::chainEntryParam(entry, ParamIDs::kChainEntryScene)) * 2.0));
}

int ChainPopoverView::getEntryBars(int entry) const {
    return static_cast<int>(std::round(
               controller_->getParamNormalized(ParamIDs::chainEntryParam(entry, ParamIDs::kChainEntryBars)) * 31.0)) +
           1;
}

VSTGUI::CRect ChainPopoverView::closeButtonRect() const {
    auto b = getViewSize();
    return {b.right - 36, b.top + kTopPadding, b.right - 8, b.top + kTopPadding + 28};
}

VSTGUI::CRect ChainPopoverView::enabledButtonRect() const {
    auto b = getViewSize();
    double y = b.top + kControlRowY;
    return {b.left + kPanelLeft, y, b.left + kPanelLeft + kBtnW, y + kBtnH};
}

VSTGUI::CRect ChainPopoverView::modeButtonRect(int mode) const {
    auto b = getViewSize();
    double y = b.top + kControlRowY;
    double x = b.left + kPanelLeft + kBtnW + 20.0 + mode * (kBtnW + 4.0);
    return {x, y, x + kBtnW, y + kBtnH};
}

VSTGUI::CRect ChainPopoverView::addButtonRect() const {
    auto b = getViewSize();
    int count = getEntryCount();
    double y = b.top + kEntryStartY + count * kRowHeight + 8.0;
    return {b.left + kPanelLeft, y, b.left + kPanelLeft + kSmallBtnW, y + kBtnH};
}

VSTGUI::CRect ChainPopoverView::removeButtonRect() const {
    auto b = getViewSize();
    int count = getEntryCount();
    double y = b.top + kEntryStartY + count * kRowHeight + 8.0;
    return {b.left + kPanelLeft + kSmallBtnW + 6.0, y, b.left + kPanelLeft + kSmallBtnW * 2 + 6.0, y + kBtnH};
}

VSTGUI::CRect ChainPopoverView::entrySceneRect(int entry, int sceneIdx) const {
    auto b = getViewSize();
    double y = b.top + kEntryStartY + entry * kRowHeight;
    double x = b.left + kPanelLeft + 40.0 + sceneIdx * (kSceneBtnW + 2.0);
    double w = (sceneIdx == 2) ? kMorphBtnW : kSceneBtnW;
    return {x, y, x + w, y + kBtnH};
}

VSTGUI::CRect ChainPopoverView::entryBarsRect(int entry, bool isPlus) const {
    auto b = getViewSize();
    double y = b.top + kEntryStartY + entry * kRowHeight;
    double baseX = b.left + kPanelLeft + 40.0 + 2.0 * (kSceneBtnW + 2.0) + kMorphBtnW + 20.0;
    if (isPlus) {
        return {baseX + kSmallBtnW + 40.0, y, baseX + kSmallBtnW * 2 + 40.0, y + kBtnH};
    }
    return {baseX, y, baseX + kSmallBtnW, y + kBtnH};
}

void ChainPopoverView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();

    context->setFillColor(CColor(0x14, 0x14, 0x1C, 0xF8));
    context->drawRect(bounds, kDrawFilled);

    // Title
    auto titleFont = makeOwned<CFontDesc>("Arial", 13.0, kBoldFace);
    context->setFont(titleFont);
    context->setFontColor(CColor(0x4A, 0x9E, 0xFF, 0xFF));
    CRect titleRect(bounds.left + 16, bounds.top + kTopPadding, bounds.left + 280, bounds.top + kTopPadding + 28);
    context->drawString("CHAIN CONFIGURATION", titleRect, kLeftText);

    // Close button
    auto btnFont = makeOwned<CFontDesc>("Arial", 10.0);
    context->setFont(btnFont);

    auto closeRect = closeButtonRect();
    context->setFillColor(CColor(0x3A, 0x20, 0x20, 0xFF));
    context->setFrameColor(CColor(0x60, 0x30, 0x30, 0xFF));
    context->setLineWidth(1.0);
    context->drawRect(closeRect, kDrawFilledAndStroked);
    context->setFontColor(CColor(0xE0, 0x80, 0x80, 0xFF));
    context->drawString("\xC3\x97", closeRect, kCenterText);

    // Divider
    CRect divider(bounds.left + 8, bounds.top + kHeaderHeight - 2, bounds.right - 8, bounds.top + kHeaderHeight);
    context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0x40));
    context->drawRect(divider, kDrawFilled);

    bool enabled = getChainEnabled();
    int mode = getChainMode();
    int entryCount = getEntryCount();

    // Enabled toggle
    auto ctrlFont = makeOwned<CFontDesc>("Arial", 9.0, kBoldFace);
    context->setFont(ctrlFont);

    auto enRect = enabledButtonRect();
    context->setFillColor(enabled ? CColor(0x2A, 0x3A, 0x4A, 0xFF) : CColor(0x22, 0x22, 0x30, 0xFF));
    context->setFrameColor(enabled ? CColor(0x4A, 0x9E, 0xFF, 0x80) : CColor(0x3A, 0x3A, 0x48, 0xFF));
    context->drawRect(enRect, kDrawFilledAndStroked);
    context->setFontColor(enabled ? CColor(0x4A, 0x9E, 0xFF, 0xFF) : CColor(0x88, 0x88, 0xA0, 0xFF));
    context->drawString(enabled ? "ON" : "OFF", enRect, kCenterText);

    // Mode buttons
    const char* modeLabels[] = {"1-Shot", "Loop", "Ping"};
    for (int m = 0; m < 3; ++m) {
        auto mr = modeButtonRect(m);
        bool sel = (mode == m);
        context->setFillColor(sel ? CColor(0x4A, 0x9E, 0xFF, 0x30) : CColor(0x22, 0x22, 0x30, 0xFF));
        context->setFrameColor(sel ? CColor(0x4A, 0x9E, 0xFF, 0x80) : CColor(0x3A, 0x3A, 0x48, 0xFF));
        context->drawRect(mr, kDrawFilledAndStroked);
        context->setFontColor(sel ? CColor(0x4A, 0x9E, 0xFF, 0xFF) : CColor(0x88, 0x88, 0xA0, 0xFF));
        context->drawString(modeLabels[m], mr, kCenterText);
    }

    // Mode label
    auto labelFont = makeOwned<CFontDesc>("Arial", 8.0);
    context->setFont(labelFont);
    context->setFontColor(CColor(0x66, 0x66, 0x7A, 0xFF));
    CRect modeLabelRect(bounds.left + kPanelLeft + kBtnW + 20.0, bounds.top + kControlRowY - 14,
                        bounds.left + kPanelLeft + kBtnW + 100.0, bounds.top + kControlRowY);
    context->drawString("MODE", modeLabelRect, kLeftText);

    // Column headers
    auto headerFont = makeOwned<CFontDesc>("Arial", 9.0, kBoldFace);
    context->setFont(headerFont);
    context->setFontColor(CColor(0x66, 0x66, 0x7A, 0xFF));
    double headerY = bounds.top + kEntryStartY - 16.0;
    context->drawString("#", CRect(bounds.left + kPanelLeft, headerY, bounds.left + kPanelLeft + 30, headerY + 14),
                        kLeftText);
    context->drawString("SCENE",
                        CRect(bounds.left + kPanelLeft + 40, headerY, bounds.left + kPanelLeft + 120, headerY + 14),
                        kLeftText);

    double barsHeaderX = bounds.left + kPanelLeft + 40.0 + 2.0 * (kSceneBtnW + 2.0) + kMorphBtnW + 20.0;
    context->drawString("BARS", CRect(barsHeaderX, headerY, barsHeaderX + 80, headerY + 14), kLeftText);

    // Entries
    auto entryFont = makeOwned<CFontDesc>("Arial", 10.0);
    auto entryBoldFont = makeOwned<CFontDesc>("Arial", 9.0, kBoldFace);
    const char* sceneLabels[] = {"A", "B", "Morph"};

    for (int e = 0; e < entryCount; ++e) {
        double rowY = bounds.top + kEntryStartY + e * kRowHeight;
        int scene = getEntryScene(e);
        int bars = getEntryBars(e);

        // Alternating row bg
        if (e % 2 == 0) {
            context->setFillColor(CColor(0x1A, 0x1A, 0x24, 0xFF));
            context->drawRect(CRect(bounds.left + kPanelLeft - 4, rowY, bounds.left + kPanelRight, rowY + kRowHeight),
                              kDrawFilled);
        }

        // Entry number
        char numLabel[8];
        std::snprintf(numLabel, sizeof(numLabel), "%d", e + 1);
        context->setFont(entryBoldFont);
        context->setFontColor(CColor(0x4A, 0x9E, 0xFF, 0xFF));
        context->drawString(
            numLabel, CRect(bounds.left + kPanelLeft, rowY, bounds.left + kPanelLeft + 30, rowY + kBtnH), kLeftText);

        // Scene select buttons (A / B / Morph)
        context->setFont(entryBoldFont);
        for (int s = 0; s < 3; ++s) {
            auto sr = entrySceneRect(e, s);
            bool sel = (scene == s);
            context->setFillColor(sel ? CColor(0x4A, 0x9E, 0xFF, 0x30) : CColor(0x22, 0x22, 0x30, 0xFF));
            context->setFrameColor(sel ? CColor(0x4A, 0x9E, 0xFF, 0x80) : CColor(0x3A, 0x3A, 0x48, 0xFF));
            context->setLineWidth(1.0);
            context->drawRect(sr, kDrawFilledAndStroked);
            context->setFontColor(sel ? CColor(0x4A, 0x9E, 0xFF, 0xFF) : CColor(0x88, 0x88, 0xA0, 0xFF));
            context->drawString(sceneLabels[s], sr, kCenterText);
        }

        // Bars: minus / value / plus
        auto minusRect = entryBarsRect(e, false);
        auto plusRect = entryBarsRect(e, true);

        context->setFont(entryBoldFont);

        context->setFillColor(CColor(0x22, 0x22, 0x30, 0xFF));
        context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
        context->drawRect(minusRect, kDrawFilledAndStroked);
        context->setFontColor(CColor(0xCC, 0xCC, 0xD0, 0xFF));
        context->drawString("\xe2\x88\x92", minusRect, kCenterText);

        context->setFillColor(CColor(0x22, 0x22, 0x30, 0xFF));
        context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
        context->drawRect(plusRect, kDrawFilledAndStroked);
        context->setFontColor(CColor(0xCC, 0xCC, 0xD0, 0xFF));
        context->drawString("+", plusRect, kCenterText);

        // Bars value
        char barsLabel[8];
        std::snprintf(barsLabel, sizeof(barsLabel), "%d", bars);
        context->setFont(entryFont);
        context->setFontColor(CColor(0xE8, 0xE8, 0xEC, 0xFF));
        CRect valRect(minusRect.right + 4, minusRect.top, plusRect.left - 4, plusRect.bottom);
        context->drawString(barsLabel, valRect, kCenterText);
    }

    // Add / Remove buttons
    if (entryCount < kMaxChainEntries) {
        auto addRect = addButtonRect();
        context->setFont(entryBoldFont);
        context->setFillColor(CColor(0x22, 0x22, 0x30, 0xFF));
        context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
        context->setLineWidth(1.0);
        context->drawRect(addRect, kDrawFilledAndStroked);
        context->setFontColor(CColor(0x4A, 0x9E, 0xFF, 0xFF));
        context->drawString("+", addRect, kCenterText);
    }

    if (entryCount > 0) {
        auto rmRect = removeButtonRect();
        context->setFont(entryBoldFont);
        context->setFillColor(CColor(0x22, 0x22, 0x30, 0xFF));
        context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
        context->setLineWidth(1.0);
        context->drawRect(rmRect, kDrawFilledAndStroked);
        context->setFontColor(CColor(0xE0, 0x80, 0x80, 0xFF));
        context->drawString("\xe2\x88\x92", rmRect, kCenterText);
    }

    setDirty(false);
}

VSTGUI::CMouseEventResult ChainPopoverView::onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (!(buttons & VSTGUI::kLButton))
        return VSTGUI::kMouseEventNotHandled;

    // Close
    if (closeButtonRect().pointInside(where)) {
        if (auto* frame = getFrame()) {
            frame->removeView(this, true);
        }
        return VSTGUI::kMouseEventHandled;
    }

    // Enable toggle
    if (enabledButtonRect().pointInside(where)) {
        pushParam(ParamIDs::kChainEnabled, getChainEnabled() ? 0.0 : 1.0);
        invalid();
        return VSTGUI::kMouseEventHandled;
    }

    // Mode buttons
    for (int m = 0; m < 3; ++m) {
        if (modeButtonRect(m).pointInside(where)) {
            pushParam(ParamIDs::kChainMode, m / 2.0);
            invalid();
            return VSTGUI::kMouseEventHandled;
        }
    }

    int entryCount = getEntryCount();

    // Add entry
    if (entryCount < kMaxChainEntries && addButtonRect().pointInside(where)) {
        pushParam(ParamIDs::kChainEntryCount, static_cast<double>(entryCount + 1) / kMaxChainEntries);
        invalid();
        return VSTGUI::kMouseEventHandled;
    }

    // Remove entry
    if (entryCount > 0 && removeButtonRect().pointInside(where)) {
        pushParam(ParamIDs::kChainEntryCount, static_cast<double>(entryCount - 1) / kMaxChainEntries);
        invalid();
        return VSTGUI::kMouseEventHandled;
    }

    // Per-entry controls
    for (int e = 0; e < entryCount; ++e) {
        // Scene select
        for (int s = 0; s < 3; ++s) {
            if (entrySceneRect(e, s).pointInside(where)) {
                pushParam(ParamIDs::chainEntryParam(e, ParamIDs::kChainEntryScene), s / 2.0);
                invalid();
                return VSTGUI::kMouseEventHandled;
            }
        }

        // Bars minus
        if (entryBarsRect(e, false).pointInside(where)) {
            int bars = getEntryBars(e);
            if (bars > 1) {
                pushParam(ParamIDs::chainEntryParam(e, ParamIDs::kChainEntryBars),
                          static_cast<double>(bars - 2) / 31.0);
                invalid();
            }
            return VSTGUI::kMouseEventHandled;
        }

        // Bars plus
        if (entryBarsRect(e, true).pointInside(where)) {
            int bars = getEntryBars(e);
            if (bars < 32) {
                pushParam(ParamIDs::chainEntryParam(e, ParamIDs::kChainEntryBars), static_cast<double>(bars) / 31.0);
                invalid();
            }
            return VSTGUI::kMouseEventHandled;
        }
    }

    return VSTGUI::kMouseEventHandled;
}

} // namespace poly
