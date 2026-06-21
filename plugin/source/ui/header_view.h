#pragma once

#include "vstgui/lib/cview.h"

namespace poly {

class HeaderView : public VSTGUI::CView {
public:
    explicit HeaderView(const VSTGUI::CRect& size);

    void draw(VSTGUI::CDrawContext* context) override;
};

} // namespace poly
