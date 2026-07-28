#include "controller.h"

#include <cstring>

#include "ui/cell_editor_view.h"
#include "ui/cross_rhythm_view.h"
#include "ui/envelope_curve_view.h"
#include "ui/export_controls_view.h"
#include "ui/header_view.h"
#include "ui/lane_edit_view.h"
#include "ui/lane_grid_view.h"
#include "ui/micro_timing_editor_view.h"
#include "ui/note_map_view.h"
#include "ui/phase_alignment_view.h"
#include "ui/scene_bar_view.h"
#include "ui/timeline_step_editor_view.h"
#include "ui/velocity_view.h"
// The shipping macOS/Windows plugin uses the choc-webview editor; the platform
// predicate is the selector (see plugin/CMakeLists.txt). POLY_UI_TEST_NATIVE_EDITOR
// is a test-only escape hatch that lets the headless UI test harness compile the
// native VSTGUI editor path on macOS/Windows without pulling in the choc/webview
// stack — it is never defined for the shipping poly_plugin target, so it cannot
// cause the local/CI editor divergence M053 S04 removed.
#if (defined(__APPLE__) || defined(_WIN32)) && !defined(POLY_UI_TEST_NATIVE_EDITOR)
#include "webui/web_ui_view.h"
#endif

namespace poly {

Steinberg::IPlugView* PLUGIN_API PolyController::createView(Steinberg::FIDString name) {
    if (Steinberg::FIDStringsEqual(name, Steinberg::Vst::ViewType::kEditor)) {
#if (defined(__APPLE__) || defined(_WIN32)) && !defined(POLY_UI_TEST_NATIVE_EDITOR)
        return new WebUIView(this); // ownership-transfer
#else
        auto* view = new VSTGUI::VST3Editor(this, "view", "poly.uidesc"); // ownership-transfer
        view->setDelegate(this);
        return view;
#endif
    }
    return nullptr;
}

VSTGUI::CView* PolyController::createCustomView(VSTGUI::UTF8StringPtr name, const VSTGUI::UIAttributes& /*attributes*/,
                                                const VSTGUI::IUIDescription* /*description*/,
                                                VSTGUI::VST3Editor* /*editor*/) {
    if (std::strcmp(name, "HeaderView") == 0) {
        return new HeaderView(VSTGUI::CRect(0, 0, 600, 32), this); // ownership-transfer
    }
    if (std::strcmp(name, "LaneEditView") == 0) {
        return new LaneEditView(VSTGUI::CRect(0, 0, 580, 126), this); // ownership-transfer
    }
    if (std::strcmp(name, "LaneGridView") == 0) {
        return new LaneGridView(VSTGUI::CRect(0, 0, 580, 156), this); // ownership-transfer
    }
    if (std::strcmp(name, "VelocityView") == 0) {
        return new VelocityView(VSTGUI::CRect(0, 0, 580, 40), this); // ownership-transfer
    }
    if (std::strcmp(name, "EnvelopeCurveView") == 0) {
        return new EnvelopeCurveView(VSTGUI::CRect(0, 0, 380, 146), this); // ownership-transfer
    }
    if (std::strcmp(name, "PhaseAlignmentView") == 0) {
        return new PhaseAlignmentView(VSTGUI::CRect(0, 0, 190, 146), this); // ownership-transfer
    }
    if (std::strcmp(name, "CellEditorView") == 0) {
        return new CellEditorView(VSTGUI::CRect(0, 0, 580, 60), this); // ownership-transfer
    }
    if (std::strcmp(name, "TimelineStepEditorView") == 0) {
        return new TimelineStepEditorView(VSTGUI::CRect(0, 0, 580, 60), this); // ownership-transfer
    }
    if (std::strcmp(name, "MicroTimingEditorView") == 0) {
        return new MicroTimingEditorView(VSTGUI::CRect(0, 0, 580, 60), this); // ownership-transfer
    }
    if (std::strcmp(name, "SceneBarView") == 0) {
        return new SceneBarView(VSTGUI::CRect(0, 0, 580, 46), this); // ownership-transfer
    }
    if (std::strcmp(name, "CrossRhythmView") == 0) {
        return new CrossRhythmView(VSTGUI::CRect(0, 0, 580, 146), this); // ownership-transfer
    }
    if (std::strcmp(name, "NoteMapView") == 0) {
        return new NoteMapView(VSTGUI::CRect(0, 0, 600, 838), this); // ownership-transfer
    }
    if (std::strcmp(name, "ExportControlsView") == 0) {
        return new ExportControlsView(VSTGUI::CRect(0, 0, 580, 46), this); // ownership-transfer
    }
    return nullptr;
}

} // namespace poly
