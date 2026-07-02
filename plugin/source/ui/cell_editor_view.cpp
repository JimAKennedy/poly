#include "cell_editor_view.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../controller.h"
#include "../plugids.h"

namespace poly {

using namespace VSTGUI;

static const CColor kLaneColors[] = {
    CColor(0x4A, 0x9E, 0xFF, 0xFF), CColor(0xF5, 0xA6, 0x23, 0xFF), CColor(0x27, 0xAE, 0x60, 0xFF),
    CColor(0xE7, 0x4C, 0x3C, 0xFF), CColor(0x9B, 0x59, 0xB6, 0xFF), CColor(0x1A, 0xBC, 0x9C, 0xFF),
    CColor(0xE6, 0x7E, 0x22, 0xFF), CColor(0x34, 0x98, 0xDB, 0xFF),
};

CellEditorView::CellEditorView(const CRect& size, PolyController* controller) : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

CellEditorView::~CellEditorView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool CellEditorView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = makeOwned<CVSTGUITimer>([this](CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool CellEditorView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

CRect CellEditorView::addButtonRect() const {
    auto bounds = getViewSize();
    return CRect(bounds.right - 48, bounds.top + 4, bounds.right - 28, bounds.top + 18);
}

CRect CellEditorView::removeButtonRect() const {
    auto bounds = getViewSize();
    return CRect(bounds.right - 24, bounds.top + 4, bounds.right - 4, bounds.top + 18);
}

void CellEditorView::draw(CDrawContext* context) {
    auto bounds = getViewSize();

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    auto laneColor = kLaneColors[selectedLane];
    const auto& cfg = controller_->activeScene().lanes[selectedLane];
    int cellCount = cfg.cellCount;

    auto labelFont = makeOwned<CFontDesc>("Arial", 8.0);
    context->setFont(labelFont);
    context->setFontColor(CColor(0x50, 0x50, 0x60, 0xFF));
    CRect titleRect(bounds.left + 8, bounds.top + 4, bounds.left + 120, bounds.top + 16);
    context->drawString("ADDITIVE CELLS", titleRect, kLeftText);

    auto btnFont = makeOwned<CFontDesc>("Arial", 10.0);
    context->setFont(btnFont);

    auto addBtn = addButtonRect();
    context->setFillColor(CColor(0x2A, 0x3A, 0x2A, 0xFF));
    context->drawRect(addBtn, kDrawFilled);
    context->setFrameColor(CColor(0x27, 0xAE, 0x60, 0x80));
    context->setLineWidth(1.0);
    context->drawRect(addBtn, kDrawStroked);
    context->setFontColor(CColor(0x27, 0xAE, 0x60, 0xFF));
    context->drawString("+", addBtn, kCenterText);

    auto removeBtn = removeButtonRect();
    context->setFillColor(CColor(0x3A, 0x2A, 0x2A, 0xFF));
    context->drawRect(removeBtn, kDrawFilled);
    context->setFrameColor(CColor(0xE7, 0x4C, 0x3C, 0x80));
    context->setLineWidth(1.0);
    context->drawRect(removeBtn, kDrawStroked);
    context->setFontColor(CColor(0xE7, 0x4C, 0x3C, 0xFF));
    context->drawString("\xe2\x88\x92", removeBtn, kCenterText);

    double barTop = bounds.top + 22;
    double barH = bounds.getHeight() - 26;
    double barLeft = bounds.left + 8;
    double barRight = bounds.right - 8;
    double barW = barRight - barLeft;

    if (cellCount <= 0) {
        CRect emptyRect(barLeft, barTop, barRight, barTop + barH);
        context->setFillColor(CColor(laneColor.red, laneColor.green, laneColor.blue, 0x18));
        context->drawRect(emptyRect, kDrawFilled);

        auto emptyFont = makeOwned<CFontDesc>("Arial", 10.0);
        context->setFont(emptyFont);
        context->setFontColor(CColor(0x50, 0x50, 0x60, 0xFF));
        context->drawString("Equal cells (standard Euclidean)", emptyRect, kCenterText);
    } else {
        int totalUnits = 0;
        for (int i = 0; i < cellCount; ++i)
            totalUnits += std::max(1, cfg.cellSizes[i]);

        if (totalUnits <= 0)
            totalUnits = cellCount;

        double pxPerUnit = barW / totalUnits;
        double x = barLeft;

        auto cellFont = makeOwned<CFontDesc>("Arial", 9.0);
        context->setFont(cellFont);

        for (int i = 0; i < cellCount; ++i) {
            int size = std::max(1, cfg.cellSizes[i]);
            double cellW = pxPerUnit * size;
            CRect cellR(x + 1, barTop, x + cellW - 1, barTop + barH);

            uint8_t alpha = static_cast<uint8_t>(0x30 + (size - 1) * 0x18);
            context->setFillColor(CColor(laneColor.red, laneColor.green, laneColor.blue, alpha));
            context->drawRect(cellR, kDrawFilled);

            context->setFrameColor(CColor(laneColor.red, laneColor.green, laneColor.blue, 0x60));
            context->setLineWidth(1.0);
            context->drawRect(cellR, kDrawStroked);

            char sizeStr[8];
            std::snprintf(sizeStr, sizeof(sizeStr), "%d", size);
            context->setFontColor(CColor(0xE8, 0xE8, 0xEC, 0xC0));
            context->drawString(sizeStr, cellR, kCenterText);

            x += cellW;
        }

        char summary[32];
        std::snprintf(summary, sizeof(summary), "%d cells", cellCount);
        CRect summaryRect(bounds.left + 120, bounds.top + 4, bounds.right - 54, bounds.top + 16);
        context->setFont(labelFont);
        context->setFontColor(CColor(laneColor.red, laneColor.green, laneColor.blue, 0x80));
        context->drawString(summary, summaryRect, kRightText);
    }

    setDirty(false);
}

int CellEditorView::hitTestCell(const CPoint& where) const {
    auto bounds = getViewSize();
    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));
    const auto& cfg = controller_->activeScene().lanes[selectedLane];
    if (cfg.cellCount <= 0)
        return -1;

    double barLeft = bounds.left + 8;
    double barRight = bounds.right - 8;
    double barW = barRight - barLeft;
    double barTop = bounds.top + 22;
    double barBottom = bounds.top + bounds.getHeight() - 4;

    if (where.y < barTop || where.y > barBottom)
        return -1;

    int totalUnits = 0;
    for (int i = 0; i < cfg.cellCount; ++i)
        totalUnits += std::max(1, cfg.cellSizes[i]);
    if (totalUnits <= 0)
        return -1;

    double pxPerUnit = barW / totalUnits;
    double x = barLeft;
    for (int i = 0; i < cfg.cellCount; ++i) {
        double cellW = pxPerUnit * std::max(1, cfg.cellSizes[i]);
        if (where.x >= x && where.x < x + cellW)
            return i;
        x += cellW;
    }
    return -1;
}

CMouseEventResult CellEditorView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    if (!(buttons & kLButton))
        return kMouseEventNotHandled;

    int selectedLane = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));

    if (addButtonRect().pointInside(where)) {
        auto cellCountId = ParamIDs::laneCoreParam(selectedLane, ParamIDs::kCoreCellCount);
        double cur = controller_->getParamNormalized(cellCountId);
        int count = static_cast<int>(std::round(cur * 64.0));
        if (count < kMaxCells) {
            count++;
            double norm = count / 64.0;
            controller_->beginEdit(cellCountId);
            controller_->setParamNormalized(cellCountId, norm);
            controller_->performEdit(cellCountId, norm);
            controller_->endEdit(cellCountId);
        }
        invalid();
        return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    if (removeButtonRect().pointInside(where)) {
        auto cellCountId = ParamIDs::laneCoreParam(selectedLane, ParamIDs::kCoreCellCount);
        double cur = controller_->getParamNormalized(cellCountId);
        int count = static_cast<int>(std::round(cur * 64.0));
        if (count > 0) {
            count--;
            double norm = count / 64.0;
            controller_->beginEdit(cellCountId);
            controller_->setParamNormalized(cellCountId, norm);
            controller_->performEdit(cellCountId, norm);
            controller_->endEdit(cellCountId);
        }
        invalid();
        return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    int cell = hitTestCell(where);
    if (cell >= 0) {
        dragCell_ = cell;
        dragLane_ = selectedLane;
        dragStartY_ = where.y;
        dragStartSize_ = std::max(1, controller_->activeScene().lanes[selectedLane].cellSizes[cell]);
        return kMouseEventHandled;
    }

    return kMouseEventNotHandled;
}

CMouseEventResult CellEditorView::onMouseMoved(CPoint& where, const CButtonState& /*buttons*/) {
    if (dragCell_ < 0)
        return kMouseEventNotHandled;

    double delta = (dragStartY_ - where.y) / kDragPixelsPerUnit;
    int newSize = std::max(1, dragStartSize_ + static_cast<int>(std::round(delta)));

    auto& lane = controller_->mutableActiveScene().lanes[dragLane_];
    if (lane.cellSizes[dragCell_] != newSize) {
        lane.cellSizes[dragCell_] = newSize;
        controller_->sendCellSizes(dragLane_);
        invalid();
    }
    return kMouseEventHandled;
}

CMouseEventResult CellEditorView::onMouseUp(CPoint& /*where*/, const CButtonState& /*buttons*/) {
    if (dragCell_ < 0)
        return kMouseEventNotHandled;

    dragCell_ = -1;
    dragLane_ = -1;
    return kMouseEventHandled;
}

} // namespace poly
