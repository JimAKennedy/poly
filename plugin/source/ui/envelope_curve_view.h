#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cview.h"

namespace poly {

class EnvelopeCurveView : public VSTGUI::CView {
public:
    EnvelopeCurveView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller);

    void draw(VSTGUI::CDrawContext* context) override;

private:
    Steinberg::Vst::EditController* controller_;
};

} // namespace poly
