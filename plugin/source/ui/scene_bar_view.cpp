#include "scene_bar_view.h"

#include <algorithm>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../plugids.h"

namespace poly {

static constexpr double kButtonW = 36.0;
static constexpr double kMorphButtonW = 50.0;
static constexpr double kButtonH = 28.0;
static constexpr double kButtonGap = 2.0;
static constexpr double kPadX = 10.0;
static constexpr double kSliderW = 280.0;
static constexpr double kSliderGap = 12.0;

SceneBarView::SceneBarView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

VSTGUI::CRect SceneBarView::sceneButtonRect(int index) const {
    auto bounds = getViewSize();
    double y = bounds.top + (bounds.getHeight() - kButtonH) / 2.0;
    double x = bounds.left + kPadX;
    if (index == 1)
        x += kButtonW + kButtonGap;
    else if (index == 2)
        x += (kButtonW + kButtonGap) * 2;
    double w = (index == 2) ? kMorphButtonW : kButtonW;
    return {x, y, x + w, y + kButtonH};
}

VSTGUI::CRect SceneBarView::morphSliderRect() const {
    auto bounds = getViewSize();
    double y = bounds.top + (bounds.getHeight() - kButtonH) / 2.0;
    auto morphBtn = sceneButtonRect(2);
    double x = morphBtn.right + kSliderGap;
    return {x, y, x + kSliderW, y + kButtonH};
}

VSTGUI::CRect SceneBarView::chainButtonRect() const {
    auto bounds = getViewSize();
    double y = bounds.top + (bounds.getHeight() - kButtonH) / 2.0;
    double x = bounds.right - kPadX - kButtonW;
    return {x, y, x + kButtonW, y + kButtonH};
}

void SceneBarView::pushParam(Steinberg::Vst::ParamID id, double value) {
    controller_->beginEdit(id);
    controller_->setParamNormalized(id, value);
    controller_->performEdit(id, value);
    controller_->endEdit(id);
}

void SceneBarView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    int sceneIdx = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSceneSelect) * 2.0));
    double morphVal = controller_->getParamNormalized(ParamIDs::kSceneMorph);
    bool chainOn = controller_->getParamNormalized(ParamIDs::kChainEnabled) > 0.5;

    auto buttonFont = makeOwned<CFontDesc>("Arial", 10.0, kBoldFace);
    context->setFont(buttonFont);

    const char* labels[] = {"A", "B", "Morph"};
    for (int i = 0; i < 3; ++i) {
        auto rect = sceneButtonRect(i);
        bool selected = (sceneIdx == i);

        if (selected) {
            context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0x30));
            context->setFrameColor(CColor(0x4A, 0x9E, 0xFF, 0x80));
        } else {
            context->setFillColor(CColor(0x22, 0x22, 0x30, 0xFF));
            context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
        }
        context->setLineWidth(1.0);
        context->drawRect(rect, kDrawFilledAndStroked);

        context->setFontColor(selected ? CColor(0x4A, 0x9E, 0xFF, 0xFF) : CColor(0x88, 0x88, 0xA0, 0xFF));
        context->drawString(labels[i], rect, kCenterText);
    }

    auto sliderRect = morphSliderRect();
    bool morphActive = (sceneIdx == 2);

    context->setFillColor(
        CColor(0x2A, 0x2A, 0x36, morphActive ? static_cast<uint8_t>(0xFF) : static_cast<uint8_t>(0x60)));
    context->drawRect(sliderRect, kDrawFilled);

    if (morphActive) {
        double fillW = sliderRect.getWidth() * morphVal;
        if (fillW > 0.5) {
            CRect fillRect(sliderRect.left, sliderRect.top, sliderRect.left + fillW, sliderRect.bottom);
            context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0x50));
            context->drawRect(fillRect, kDrawFilled);
        }

        double thumbX = sliderRect.left + fillW;
        CRect thumbRect(thumbX - 2, sliderRect.top, thumbX + 2, sliderRect.bottom);
        context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0xFF));
        context->drawRect(thumbRect, kDrawFilled);
    }

    context->setFrameColor(morphActive ? CColor(0x3A, 0x3A, 0x48, 0xFF) : CColor(0x2A, 0x2A, 0x36, 0x60));
    context->setLineWidth(1.0);
    context->drawRect(sliderRect, kDrawStroked);

    auto labelFont = makeOwned<CFontDesc>("Arial", 8.0);
    context->setFont(labelFont);
    CColor dimColor = morphActive ? CColor(0x88, 0x88, 0xA0, 0xFF) : CColor(0x50, 0x50, 0x60, 0xFF);
    context->setFontColor(dimColor);
    CRect aLabelRect(sliderRect.left + 4, sliderRect.top + 2, sliderRect.left + 20, sliderRect.top + 14);
    context->drawString("A", aLabelRect, kLeftText);
    CRect bLabelRect(sliderRect.right - 20, sliderRect.top + 2, sliderRect.right - 4, sliderRect.top + 14);
    context->drawString("B", bLabelRect, kRightText);

    auto chainRect = chainButtonRect();
    if (chainOn) {
        context->setFillColor(CColor(0x2A, 0x3A, 0x4A, 0xFF));
        context->setFrameColor(CColor(0x4A, 0x9E, 0xFF, 0x80));
    } else {
        context->setFillColor(CColor(0x22, 0x22, 0x30, 0xFF));
        context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
    }
    context->setLineWidth(1.0);
    context->drawRect(chainRect, kDrawFilledAndStroked);

    auto chainFont = makeOwned<CFontDesc>("Arial", 8.0, kBoldFace);
    context->setFont(chainFont);
    context->setFontColor(chainOn ? CColor(0x4A, 0x9E, 0xFF, 0xFF) : CColor(0x88, 0x88, 0xA0, 0xFF));
    context->drawString("CHAIN", chainRect, kCenterText);

    setDirty(false);
}

VSTGUI::CMouseEventResult SceneBarView::onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (!(buttons & VSTGUI::kLButton))
        return VSTGUI::kMouseEventNotHandled;

    for (int i = 0; i < 3; ++i) {
        if (sceneButtonRect(i).pointInside(where)) {
            pushParam(ParamIDs::kSceneSelect, i / 2.0);
            invalid();
            return VSTGUI::kMouseEventHandled;
        }
    }

    int sceneIdx = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSceneSelect) * 2.0));
    if (sceneIdx == 2 && morphSliderRect().pointInside(where)) {
        draggingMorph_ = true;
        auto sliderRect = morphSliderRect();
        double norm = std::clamp((where.x - sliderRect.left) / sliderRect.getWidth(), 0.0, 1.0);
        pushParam(ParamIDs::kSceneMorph, norm);
        invalid();
        return VSTGUI::kMouseEventHandled;
    }

    if (chainButtonRect().pointInside(where)) {
        bool current = controller_->getParamNormalized(ParamIDs::kChainEnabled) > 0.5;
        pushParam(ParamIDs::kChainEnabled, current ? 0.0 : 1.0);
        invalid();
        return VSTGUI::kMouseEventHandled;
    }

    return VSTGUI::kMouseEventNotHandled;
}

VSTGUI::CMouseEventResult SceneBarView::onMouseMoved(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (!draggingMorph_)
        return VSTGUI::kMouseEventNotHandled;

    auto sliderRect = morphSliderRect();
    double norm = std::clamp((where.x - sliderRect.left) / sliderRect.getWidth(), 0.0, 1.0);
    pushParam(ParamIDs::kSceneMorph, norm);
    invalid();
    return VSTGUI::kMouseEventHandled;
}

VSTGUI::CMouseEventResult SceneBarView::onMouseUp(VSTGUI::CPoint& where, const VSTGUI::CButtonState& /*buttons*/) {
    if (draggingMorph_) {
        draggingMorph_ = false;
        auto sliderRect = morphSliderRect();
        double norm = std::clamp((where.x - sliderRect.left) / sliderRect.getWidth(), 0.0, 1.0);
        pushParam(ParamIDs::kSceneMorph, norm);
        invalid();
        return VSTGUI::kMouseEventHandled;
    }
    return VSTGUI::kMouseEventNotHandled;
}

} // namespace poly
