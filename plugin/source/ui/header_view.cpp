#include "header_view.h"

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../plugids.h"

namespace poly {

HeaderView::HeaderView(const VSTGUI::CRect& size) : CView(size) {
    setWantsFocus(false);
}

void HeaderView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    auto bounds = getViewSize();

    context->setFillColor(CColor(0x18, 0x18, 0x22, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    auto titleFont = makeOwned<CFontDesc>("Arial", 15.0, kBoldFace);
    context->setFont(titleFont);
    context->setFontColor(CColor(0x4A, 0x9E, 0xFF, 0xFF));
    CRect titleRect(bounds.left + 14, bounds.top, bounds.left + 120, bounds.bottom);
    context->drawString("POLY", titleRect, kLeftText);

    auto versionFont = makeOwned<CFontDesc>("Arial", 9.0);
    context->setFont(versionFont);
    context->setFontColor(CColor(0x60, 0x60, 0x6A, 0xFF));
    CRect versionRect(bounds.right - 80, bounds.top, bounds.right - 10, bounds.bottom);
    context->drawString(kPolyVersionString, versionRect, kRightText);

    CRect accentLine(bounds.left, bounds.bottom - 2, bounds.right, bounds.bottom);
    context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0x40));
    context->drawRect(accentLine, kDrawFilled);

    setDirty(false);
}

} // namespace poly
