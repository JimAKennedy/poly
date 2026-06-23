#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cvstguitimer.h"

namespace poly {

class PolyController;

class CellEditorView : public VSTGUI::CView {
public:
    CellEditorView(const VSTGUI::CRect& size, PolyController* controller);
    ~CellEditorView() override;

    void draw(VSTGUI::CDrawContext* context) override;
    bool attached(CView* parent) override;
    bool removed(CView* parent) override;
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;

private:
    static constexpr int kMaxCells = 16;

    VSTGUI::CRect cellRect(int cellIdx, int cellCount, const int* sizes, double totalW, double y, double h) const;
    VSTGUI::CRect addButtonRect() const;
    VSTGUI::CRect removeButtonRect() const;
    int hitTestCell(const VSTGUI::CPoint& where) const;

    PolyController* controller_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> refreshTimer_;
};

} // namespace poly
