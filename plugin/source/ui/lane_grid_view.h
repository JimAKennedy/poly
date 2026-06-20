#pragma once

#include "vstgui/lib/cview.h"
#include "vstgui/lib/ccolor.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

namespace poly {

class LaneGridView : public VSTGUI::CView {
public:
    LaneGridView(const VSTGUI::CRect& size,
                 Steinberg::Vst::EditController* controller);

    void draw(VSTGUI::CDrawContext* context) override;

private:
    Steinberg::Vst::EditController* controller_;
};

} // namespace poly
