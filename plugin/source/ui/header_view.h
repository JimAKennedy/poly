#pragma once

#include "vstgui/lib/cview.h"

namespace Steinberg {
namespace Vst {
class EditController;
}
} // namespace Steinberg

namespace poly {

class HeaderView : public VSTGUI::CView {
public:
    HeaderView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller);

    void draw(VSTGUI::CDrawContext* context) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    static constexpr int kInitPreset = -2;

    void applyPreset(int index);
    void resetToInit();
    void toggleNoteMap();
    VSTGUI::CRect noteMapButtonRect() const;

    Steinberg::Vst::EditController* controller_;
    int selectedPreset_ = -1;
};

} // namespace poly
