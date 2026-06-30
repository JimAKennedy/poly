#include "export_controls_view.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cdropsource.h"
#include "vstgui/lib/cfileselector.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/cframe.h"

#include "../controller.h"
#include "../plugids.h"

namespace poly {

static constexpr double kPadX = 10.0;
static constexpr double kButtonW = 70.0;
static constexpr double kButtonH = 28.0;
static constexpr double kArrowW = 24.0;
static constexpr double kBarsDisplayW = 80.0;
static constexpr double kBarsGroupGap = 20.0;

ExportControlsView::ExportControlsView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

VSTGUI::CRect ExportControlsView::exportButtonRect() const {
    auto bounds = getViewSize();
    double y = bounds.top + (bounds.getHeight() - kButtonH) / 2.0;
    double x = bounds.left + kPadX;
    return {x, y, x + kButtonW, y + kButtonH};
}

VSTGUI::CRect ExportControlsView::barsMinusRect() const {
    auto btn = exportButtonRect();
    double x = btn.right + kBarsGroupGap;
    double y = btn.top;
    return {x, y, x + kArrowW, y + kButtonH};
}

VSTGUI::CRect ExportControlsView::barsLabelRect() const {
    auto minus = barsMinusRect();
    double x = minus.right;
    double y = minus.top;
    return {x, y, x + kBarsDisplayW, y + kButtonH};
}

VSTGUI::CRect ExportControlsView::barsPlusRect() const {
    auto label = barsLabelRect();
    double x = label.right;
    double y = label.top;
    return {x, y, x + kArrowW, y + kButtonH};
}

void ExportControlsView::pushParam(Steinberg::Vst::ParamID id, double value) {
    controller_->beginEdit(id);
    controller_->setParamNormalized(id, value);
    controller_->performEdit(id, value);
    controller_->endEdit(id);
}

int ExportControlsView::currentBars() const {
    double norm = controller_->getParamNormalized(ParamIDs::kCaptureLength);
    return 1 + static_cast<int>(std::round(norm * 31.0));
}

bool ExportControlsView::captureReady() const {
    return controller_->getParamNormalized(ParamIDs::kCaptureReady) > 0.5;
}

void ExportControlsView::sendExportRequest() {
    if (auto* msg = controller_->allocateMessage()) {
        msg->setMessageID("RequestMidiExport");
        controller_->sendMessage(msg);
        msg->release();
    }
}

void ExportControlsView::openSaveDialog() {
    auto* ctrl = static_cast<PolyController*>(controller_);
    if (!ctrl->hasPendingSmf())
        return;

    auto smfData = ctrl->consumeSmfData();
    auto* frame = getFrame();
    if (!frame || smfData.empty())
        return;

    auto* selector = VSTGUI::CNewFileSelector::create(frame, VSTGUI::CNewFileSelector::kSelectSaveFile);
    if (!selector)
        return;

    selector->setTitle("Export MIDI");
    selector->setDefaultSaveName("poly-export.mid");
    auto ext = VSTGUI::CFileExtension("MIDI File", "mid", "audio/midi");
    selector->addFileExtension(ext);

    auto data = std::make_shared<std::vector<uint8_t>>(std::move(smfData));
    selector->run([data](VSTGUI::CNewFileSelector* sel) {
        if (sel->getNumSelectedFiles() > 0) {
            auto path = sel->getSelectedFile(0);
            if (path) {
                std::ofstream out(path, std::ios::binary);
                if (out) {
                    out.write(reinterpret_cast<const char*>(data->data()), static_cast<std::streamsize>(data->size()));
                }
            }
        }
    });
    selector->forget();
}

bool ExportControlsView::attached(VSTGUI::CView* parent) {
    if (!CView::attached(parent))
        return false;
    refreshTimer_ = VSTGUI::makeOwned<VSTGUI::CVSTGUITimer>(
        [this](VSTGUI::CVSTGUITimer*) {
            auto* ctrl = static_cast<PolyController*>(controller_);
            if (ctrl->hasPendingSmf()) {
                if (exportPending_) {
                    exportPending_ = false;
                    openSaveDialog();
                } else {
                    ctrl->consumeSmfData();
                }
            }
            invalid();
        },
        100);
    return true;
}

bool ExportControlsView::removed(VSTGUI::CView* parent) {
    refreshTimer_ = nullptr;
    exportTimer_ = nullptr;
    exportPending_ = false;
    return CView::removed(parent);
}

void ExportControlsView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    bool ready = captureReady() && !exportPending_;
    int bars = currentBars();

    auto btnRect = exportButtonRect();
    auto btnFont = makeOwned<CFontDesc>("Arial", 10.0, kBoldFace);
    context->setFont(btnFont);

    if (exportPending_) {
        context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0x30));
        context->setFrameColor(CColor(0x4A, 0x9E, 0xFF, 0x80));
    } else if (ready) {
        context->setFillColor(CColor(0x27, 0xAE, 0x60, 0x40));
        context->setFrameColor(CColor(0x27, 0xAE, 0x60, 0xA0));
    } else {
        context->setFillColor(CColor(0x22, 0x22, 0x30, 0xFF));
        context->setFrameColor(CColor(0x2A, 0x2A, 0x36, 0xFF));
    }
    context->setLineWidth(1.0);
    context->drawRect(btnRect, kDrawFilledAndStroked);

    if (exportPending_) {
        context->setFontColor(CColor(0x4A, 0x9E, 0xFF, 0xFF));
        context->drawString("Saving...", btnRect, kCenterText);
    } else {
        context->setFontColor(ready ? CColor(0x27, 0xAE, 0x60, 0xFF) : CColor(0x50, 0x50, 0x60, 0xFF));
        context->drawString("Export", btnRect, kCenterText);
    }

    auto minusRect = barsMinusRect();
    auto labelRect = barsLabelRect();
    auto plusRect = barsPlusRect();

    context->setFillColor(CColor(0x22, 0x22, 0x30, 0xFF));
    context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
    context->setLineWidth(1.0);

    context->drawRect(minusRect, kDrawFilledAndStroked);
    context->drawRect(labelRect, kDrawFilledAndStroked);
    context->drawRect(plusRect, kDrawFilledAndStroked);

    auto arrowFont = makeOwned<CFontDesc>("Arial", 12.0, kBoldFace);
    context->setFont(arrowFont);
    context->setFontColor(CColor(0x88, 0x88, 0xA0, 0xFF));
    context->drawString("\xe2\x88\x92", minusRect, kCenterText);
    context->drawString("+", plusRect, kCenterText);

    char barsText[16];
    std::snprintf(barsText, sizeof(barsText), "%d bars", bars);
    auto labelFont = makeOwned<CFontDesc>("Arial", 10.0);
    context->setFont(labelFont);
    context->setFontColor(CColor(0xE8, 0xE8, 0xEC, 0xFF));
    context->drawString(barsText, labelRect, kCenterText);

    double countNorm = controller_->getParamNormalized(ParamIDs::kCaptureEventCount);
    int eventCount = static_cast<int>(std::round(countNorm * 2048.0));
    bool hasEvents = eventCount > 0;

    double statusX = bounds.right - kPadX - 160.0;
    double statusY = bounds.top + (bounds.getHeight() - kButtonH) / 2.0;

    if (hasEvents) {
        double dotX = statusX;
        double dotY = statusY + kButtonH / 2.0;
        double dotR = 4.0;
        CRect dotRect(dotX - dotR, dotY - dotR, dotX + dotR, dotY + dotR);
        context->setFillColor(CColor(0x27, 0xAE, 0x60, 0xFF));
        context->drawEllipse(dotRect, kDrawFilled);
    } else {
        double dotX = statusX;
        double dotY = statusY + kButtonH / 2.0;
        double dotR = 4.0;
        CRect dotRect(dotX - dotR, dotY - dotR, dotX + dotR, dotY + dotR);
        context->setFillColor(CColor(0x50, 0x50, 0x60, 0xFF));
        context->drawEllipse(dotRect, kDrawFilled);
    }

    char countText[32];
    std::snprintf(countText, sizeof(countText), "%d events", eventCount);
    auto countFont = makeOwned<CFontDesc>("Arial", 9.0);
    context->setFont(countFont);
    context->setFontColor(hasEvents ? CColor(0xE8, 0xE8, 0xEC, 0xFF) : CColor(0x50, 0x50, 0x60, 0xFF));
    CRect countRect(statusX + 10.0, statusY, statusX + 90.0, statusY + 14.0);
    context->drawString(countText, countRect, kLeftText);

    double barX = statusX + 10.0;
    double barY = statusY + 17.0;
    double barW = 140.0;
    double barH = 6.0;
    CRect barBg(barX, barY, barX + barW, barY + barH);
    context->setFillColor(CColor(0x2A, 0x2A, 0x36, 0xFF));
    context->drawRect(barBg, kDrawFilled);

    if (countNorm > 0.0) {
        double fillW = barW * std::min(countNorm, 1.0);
        CRect barFill(barX, barY, barX + fillW, barY + barH);
        uint8_t r = countNorm > 0.9 ? 0xE7 : 0x27;
        uint8_t g = countNorm > 0.9 ? 0x4C : 0xAE;
        uint8_t b = countNorm > 0.9 ? 0x3C : 0x60;
        context->setFillColor(CColor(r, g, b, 0xCC));
        context->drawRect(barFill, kDrawFilled);
    }

    setDirty(false);
}

VSTGUI::CMouseEventResult ExportControlsView::onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (!(buttons & VSTGUI::kLButton))
        return VSTGUI::kMouseEventNotHandled;

    if (exportButtonRect().pointInside(where) && captureReady() && !exportPending_) {
        dragStartPos_ = where;
        dragTracking_ = true;
        return VSTGUI::kMouseEventHandled;
    }

    int bars = currentBars();
    if (barsMinusRect().pointInside(where) && bars > 1) {
        double norm = static_cast<double>(bars - 2) / 31.0;
        pushParam(ParamIDs::kCaptureLength, norm);
        invalid();
        return VSTGUI::kMouseEventHandled;
    }

    if (barsPlusRect().pointInside(where) && bars < 32) {
        double norm = static_cast<double>(bars) / 31.0;
        pushParam(ParamIDs::kCaptureLength, norm);
        invalid();
        return VSTGUI::kMouseEventHandled;
    }

    return VSTGUI::kMouseEventNotHandled;
}

VSTGUI::CMouseEventResult ExportControlsView::onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (dragTracking_ && (buttons & VSTGUI::kLButton)) {
        auto* ctrl = static_cast<PolyController*>(controller_);
        if (ctrl->hasDragSmf() && VSTGUI::shouldStartDrag(dragStartPos_, where)) {
            dragTracking_ = false;
            startDrag();
            return VSTGUI::kMouseEventHandled;
        }
        return VSTGUI::kMouseEventHandled;
    }
    return VSTGUI::kMouseEventNotHandled;
}

VSTGUI::CMouseEventResult ExportControlsView::onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (dragTracking_) {
        dragTracking_ = false;
        pushParam(ParamIDs::kExportTrigger, 1.0);
        exportPending_ = true;
        exportTimer_ = VSTGUI::makeOwned<VSTGUI::CVSTGUITimer>(
            [this](VSTGUI::CVSTGUITimer*) {
                exportTimer_ = nullptr;
                sendExportRequest();
            },
            200, false);
        invalid();
        return VSTGUI::kMouseEventHandled;
    }
    return VSTGUI::kMouseEventNotHandled;
}

void ExportControlsView::startDrag() {
    auto* ctrl = static_cast<PolyController*>(controller_);
    if (!ctrl->hasDragSmf())
        return;

    auto tempPath = std::filesystem::temp_directory_path() / "poly-drag-export.mid";
    const auto& data = ctrl->dragSmfData();
    {
        std::ofstream out(tempPath, std::ios::binary);
        if (!out)
            return;
        out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    }

    auto pathStr = tempPath.string();
    auto dropSource = VSTGUI::CDropSource::create(pathStr.c_str(), static_cast<uint32_t>(pathStr.size() + 1),
                                                  VSTGUI::IDataPackage::kFilePath);
    VSTGUI::DragDescription dragDesc(dropSource);
    doDrag(dragDesc);
}

} // namespace poly
