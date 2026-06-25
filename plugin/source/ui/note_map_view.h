#pragma once

#include <array>

#include "vstgui/lib/cview.h"

namespace Steinberg {
namespace Vst {
class EditController;
}
} // namespace Steinberg

namespace poly {

class NoteMapView : public VSTGUI::CView {
public:
    NoteMapView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller);

    void draw(VSTGUI::CDrawContext* context) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    static constexpr int kRowHeight = 30;
    static constexpr int kHeaderHeight = 44;
    static constexpr int kTopPadding = 8;
    static constexpr int kDrumRangeStart = 35;
    static constexpr int kDrumRangeEnd = 82;

    struct RowInfo {
        int16_t sourceNote;
        int laneIndex;
    };

    void buildRows();
    VSTGUI::CRect closeButtonRect() const;
    VSTGUI::CRect resetButtonRect() const;
    VSTGUI::CRect destRect(int row) const;
    int hitTestRow(const VSTGUI::CPoint& where) const;
    void showNoteMenu(int row, const VSTGUI::CPoint& where);

    static const char* noteLetter(int16_t note);
    static int noteOctave(int16_t note);
    static const char* gmDrumName(int16_t note);

    Steinberg::Vst::EditController* controller_;
    std::array<RowInfo, 8> rows_{};
    int rowCount_ = 0;
};

} // namespace poly
