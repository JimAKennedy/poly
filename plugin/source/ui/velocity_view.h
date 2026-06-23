#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"

namespace poly {

class VelocityView : public VSTGUI::CView {
public:
    VelocityView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller);
    ~VelocityView() override;

    void draw(VSTGUI::CDrawContext* context) override;
    bool attached(CView* parent) override;
    bool removed(CView* parent) override;

private:
    Steinberg::Vst::EditController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
};

} // namespace poly
