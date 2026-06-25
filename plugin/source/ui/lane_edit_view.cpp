#include "lane_edit_view.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"

#include "../plugids.h"

namespace poly {

using namespace VSTGUI;

static const CColor kLaneColors[] = {
    CColor(0x4A, 0x9E, 0xFF, 0xFF), CColor(0xF5, 0xA6, 0x23, 0xFF), CColor(0x27, 0xAE, 0x60, 0xFF),
    CColor(0xE7, 0x4C, 0x3C, 0xFF), CColor(0x9B, 0x59, 0xB6, 0xFF), CColor(0x1A, 0xBC, 0x9C, 0xFF),
    CColor(0xE6, 0x7E, 0x22, 0xFF), CColor(0x34, 0x98, 0xDB, 0xFF),
};

static const char* kNoteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

const LaneEditView::KnobDef LaneEditView::kLaneKnobs[kLaneKnobCount] = {
    {ParamIDs::kCoreSteps, true, "Steps", 64.0, ValueFormat::Integer},
    {ParamIDs::kCoreSubdivision, true, "Subdiv", 4.0, ValueFormat::Subdivision},
    {ParamIDs::kCoreHits, true, "Hits", 64.0, ValueFormat::Integer},
    {ParamIDs::kCoreRotation, true, "Rot", 63.0, ValueFormat::Integer},
    {ParamIDs::kCoreMidiNote, true, "Note", 127.0, ValueFormat::MidiNote},
    {ParamIDs::kBaseVelocity, false, "Vel", 127.0, ValueFormat::Integer},
    {ParamIDs::kGhostFloor, false, "Ghost", 127.0, ValueFormat::Integer},
    {ParamIDs::kVelocitySpread, false, "Spread", 100.0, ValueFormat::Percent},
    {ParamIDs::kSwingAmount, false, "Swing", 100.0, ValueFormat::Percent},
    {ParamIDs::kKotekanSource, false, "Kotek", 8.0, ValueFormat::KotekanSrc},
};

const LaneEditView::KnobDef LaneEditView::kPhraseKnobs[kPhraseKnobCount] = {
    {ParamIDs::kPhraseLength, false, "Len", 64.0, ValueFormat::Beats},
    {ParamIDs::kPhraseGap, false, "Gap", 64.0, ValueFormat::Beats},
    {ParamIDs::kPhraseOffset, false, "Ofs", 64.0, ValueFormat::Beats},
    {ParamIDs::kMutationRate, false, "Mut", 100.0, ValueFormat::Percent},
    {ParamIDs::kDriftRate, false, "Drift", 4.0, ValueFormat::BipolarSteps},
    {ParamIDs::kTimingOffset, false, "Time", 20.0, ValueFormat::BipolarMs},
};

LaneEditView::LaneEditView(const CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
}

LaneEditView::~LaneEditView() {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
}

bool LaneEditView::attached(CView* parent) {
    if (CView::attached(parent)) {
        refreshTimer_ = makeOwned<CVSTGUITimer>([this](CVSTGUITimer*) { invalid(); }, 33);
        return true;
    }
    return false;
}

bool LaneEditView::removed(CView* parent) {
    if (refreshTimer_) {
        refreshTimer_->stop();
        refreshTimer_ = nullptr;
    }
    return CView::removed(parent);
}

CRect LaneEditView::laneTabRect(int lane) const {
    auto bounds = getViewSize();
    double x = bounds.left + 8 + lane * 27.0;
    return CRect(x, bounds.top + 34, x + 24, bounds.top + 48);
}

CRect LaneEditView::laneKnobRect(int knob) const {
    static constexpr double kKnobX[] = {268, 298, 328, 358, 388, 422, 452, 482, 512, 542};
    auto bounds = getViewSize();
    double x = bounds.left + kKnobX[knob];
    return CRect(x, bounds.top + 28, x + 26, bounds.top + 54);
}

CRect LaneEditView::phraseKnobRect(int knob) const {
    static constexpr double kKnobX[] = {20, 53, 86, 128, 161, 194};
    auto bounds = getViewSize();
    double x = bounds.left + kKnobX[knob];
    return CRect(x, bounds.top + 86, x + 26, bounds.top + 112);
}

CRect LaneEditView::schematicRect() const {
    auto bounds = getViewSize();
    return CRect(bounds.left + 232, bounds.top + 93, bounds.left + 564, bounds.top + 105);
}

int LaneEditView::hitTestTab(const CPoint& where) const {
    for (int i = 0; i < kMaxLanes; ++i) {
        if (laneTabRect(i).pointInside(where))
            return i;
    }
    return -1;
}

int LaneEditView::hitTestLaneKnob(const CPoint& where) const {
    for (int i = 0; i < kLaneKnobCount; ++i) {
        auto r = laneKnobRect(i);
        r.extend(4, 4);
        if (r.pointInside(where))
            return i;
    }
    return -1;
}

int LaneEditView::hitTestPhraseKnob(const CPoint& where) const {
    for (int i = 0; i < kPhraseKnobCount; ++i) {
        auto r = phraseKnobRect(i);
        r.extend(4, 4);
        if (r.pointInside(where))
            return i;
    }
    return -1;
}

Steinberg::Vst::ParamID LaneEditView::paramIdForKnob(const KnobDef& def, int lane) const {
    if (def.isCore)
        return ParamIDs::laneCoreParam(lane, def.paramOffset);
    return ParamIDs::laneParam(lane, def.paramOffset);
}

void LaneEditView::drawKnob(CDrawContext* ctx, const CRect& rect, double value, const CColor& color, const KnobDef& def,
                            bool enabled) {
    double cx = rect.left + rect.getWidth() / 2;
    double cy = rect.top + rect.getHeight() / 2;
    double r = std::min(rect.getWidth(), rect.getHeight()) / 2 - 2;

    constexpr double kPi = 3.14159265358979;
    constexpr double kStartAngle = 3.0 * kPi / 4.0;
    constexpr double kSweepAngle = 3.0 * kPi / 2.0;
    constexpr int kSegments = 36;

    uint8_t trackAlpha = enabled ? 0xFF : 0x40;
    ctx->setLineWidth(2.0);
    ctx->setFrameColor(CColor(0x2A, 0x2A, 0x36, trackAlpha));
    for (int i = 0; i < kSegments; ++i) {
        double a0 = kStartAngle + kSweepAngle * i / kSegments;
        double a1 = kStartAngle + kSweepAngle * (i + 1) / kSegments;
        ctx->drawLine(CPoint(cx + r * std::cos(a0), cy + r * std::sin(a0)),
                      CPoint(cx + r * std::cos(a1), cy + r * std::sin(a1)));
    }

    if (enabled && value > 0.005) {
        int valueSegs = std::max(1, static_cast<int>(kSegments * value));
        ctx->setFrameColor(CColor(color.red, color.green, color.blue, 0xCC));
        for (int i = 0; i < valueSegs; ++i) {
            double a0 = kStartAngle + kSweepAngle * i / kSegments;
            double a1 = kStartAngle + kSweepAngle * (i + 1) / kSegments;
            ctx->drawLine(CPoint(cx + r * std::cos(a0), cy + r * std::sin(a0)),
                          CPoint(cx + r * std::cos(a1), cy + r * std::sin(a1)));
        }
    }

    double angle = kStartAngle + kSweepAngle * value;
    double hx = cx + r * std::cos(angle);
    double hy = cy + r * std::sin(angle);
    constexpr double kDotR = 2.5;
    CRect dot(hx - kDotR, hy - kDotR, hx + kDotR, hy + kDotR);
    uint8_t dotAlpha = enabled ? 0xFF : 0x30;
    ctx->setFillColor(CColor(0xE8, 0xE8, 0xEC, dotAlpha));
    ctx->drawEllipse(dot, kDrawFilled);

    auto font = makeOwned<CFontDesc>("Arial", 8.0);
    ctx->setFont(font);
    uint8_t labelAlpha = enabled ? 0xA0 : 0x38;
    ctx->setFontColor(CColor(0x88, 0x88, labelAlpha, 0xFF));
    CRect labelRect(rect.left - 6, rect.bottom + 1, rect.right + 6, rect.bottom + 11);
    ctx->drawString(def.label, labelRect, kCenterText);

    char valStr[16];
    if (!enabled) {
        std::snprintf(valStr, sizeof(valStr), "--");
    } else if (def.format == ValueFormat::Integer) {
        int v = static_cast<int>(std::round(value * def.displayMax));
        std::snprintf(valStr, sizeof(valStr), "%d", v);
    } else if (def.format == ValueFormat::Subdivision) {
        static constexpr int subdivs[] = {1, 2, 4, 8, 16};
        int idx = static_cast<int>(std::round(value * 4.0));
        idx = std::clamp(idx, 0, 4);
        std::snprintf(valStr, sizeof(valStr), "1/%d", subdivs[idx]);
    } else if (def.format == ValueFormat::MidiNote) {
        int note = static_cast<int>(std::round(value * 127.0));
        int octave = (note / 12) - 2;
        const char* name = kNoteNames[note % 12];
        std::snprintf(valStr, sizeof(valStr), "%s%d", name, octave);
    } else if (def.format == ValueFormat::Percent) {
        double pct = value * 100.0;
        if (pct < 0.5)
            std::snprintf(valStr, sizeof(valStr), "off");
        else
            std::snprintf(valStr, sizeof(valStr), "%.0f%%", pct);
    } else if (def.format == ValueFormat::KotekanSrc) {
        int src = static_cast<int>(std::round(value * 8.0)) - 1;
        if (src < 0)
            std::snprintf(valStr, sizeof(valStr), "off");
        else
            std::snprintf(valStr, sizeof(valStr), "L%d", src + 1);
    } else if (def.format == ValueFormat::Beats) {
        double beats = value * def.displayMax;
        if (beats < 0.05)
            std::snprintf(valStr, sizeof(valStr), "off");
        else
            std::snprintf(valStr, sizeof(valStr), "%.1f bt", beats);
    } else if (def.format == ValueFormat::BipolarSteps) {
        double steps = (value - 0.5) * 2.0 * def.displayMax;
        if (std::fabs(steps) < 0.05)
            std::snprintf(valStr, sizeof(valStr), "off");
        else
            std::snprintf(valStr, sizeof(valStr), "%+.1f st", steps);
    } else if (def.format == ValueFormat::BipolarMs) {
        double ms = (value - 0.5) * 2.0 * def.displayMax;
        if (std::fabs(ms) < 0.5)
            std::snprintf(valStr, sizeof(valStr), "off");
        else
            std::snprintf(valStr, sizeof(valStr), "%+.0f ms", ms);
    }

    auto smallFont = makeOwned<CFontDesc>("Arial", 7.0);
    ctx->setFont(smallFont);
    uint8_t valAlpha = enabled ? 0x78 : 0x30;
    ctx->setFontColor(CColor(0x66, 0x66, valAlpha, 0xFF));
    CRect valRect(rect.left - 6, rect.top - 10, rect.right + 6, rect.top - 1);
    ctx->drawString(valStr, valRect, kCenterText);
}

void LaneEditView::drawPhraseSchematic(CDrawContext* ctx, const CColor& color, double lenBeats, double gapBeats,
                                       double ofsBeats) {
    auto bar = schematicRect();
    double barWidth = bar.getWidth();

    ctx->setFillColor(CColor(0x14, 0x14, 0x1C, 0xFF));
    ctx->drawRect(bar, kDrawFilled);

    if (lenBeats < 0.05) {
        ctx->setFillColor(CColor(color.red, color.green, color.blue, 0x30));
        ctx->drawRect(bar, kDrawFilled);
        auto font = makeOwned<CFontDesc>("Arial", 8.0);
        ctx->setFont(font);
        ctx->setFontColor(CColor(0x50, 0x50, 0x60, 0xFF));
        ctx->drawString("continuous", bar, kCenterText);
        return;
    }

    double cycleBeats = lenBeats + gapBeats;
    if (cycleBeats < 0.01)
        return;

    double pxPerBeat = barWidth / cycleBeats;

    double ofsWrapped = std::fmod(ofsBeats, cycleBeats);
    if (ofsWrapped < 0)
        ofsWrapped += cycleBeats;

    double playStart = ofsWrapped;
    double playEnd = ofsWrapped + lenBeats;

    ctx->setFillColor(CColor(color.red, color.green, color.blue, 0x70));
    if (playEnd <= cycleBeats) {
        CRect playRect(bar.left + playStart * pxPerBeat, bar.top, bar.left + playEnd * pxPerBeat, bar.bottom);
        ctx->drawRect(playRect, kDrawFilled);
    } else {
        CRect r1(bar.left + playStart * pxPerBeat, bar.top, bar.right, bar.bottom);
        ctx->drawRect(r1, kDrawFilled);
        CRect r2(bar.left, bar.top, bar.left + (playEnd - cycleBeats) * pxPerBeat, bar.bottom);
        ctx->drawRect(r2, kDrawFilled);
    }

    ctx->setLineWidth(0.5);
    double tickBase = bar.bottom + 1;
    for (double beat = 0; beat <= cycleBeats + 0.01; beat += 1.0) {
        double px = bar.left + beat * pxPerBeat;
        if (px > bar.right + 0.5)
            break;
        if (pxPerBeat < 3.0 && std::fmod(beat, 4.0) > 0.01)
            continue;
        bool major = (std::fmod(beat, 4.0) < 0.01);
        double tickH = major ? 3.0 : 2.0;
        ctx->setFrameColor(CColor(0x50, 0x50, 0x60, major ? (uint8_t)0x80 : (uint8_t)0x40));
        ctx->drawLine(CPoint(px, tickBase), CPoint(px, tickBase + tickH));
    }

    ctx->setFrameColor(CColor(0x30, 0x30, 0x40, 0x60));
    ctx->setLineWidth(0.5);
    ctx->drawRect(bar, kDrawStroked);

    auto formatBeat = [](char* buf, size_t sz, double b) {
        if (std::fabs(b - std::round(b)) < 0.05)
            std::snprintf(buf, sz, "%.0f", b);
        else
            std::snprintf(buf, sz, "%.1f", b);
    };

    struct BeatLabel {
        double px;
        double beat;
    };
    BeatLabel labels[3];
    int labelCount = 0;

    labels[labelCount++] = {bar.left, 0.0};
    if (gapBeats >= 0.05) {
        double gapStartPx = bar.left + (lenBeats / cycleBeats) * barWidth;
        labels[labelCount++] = {gapStartPx, lenBeats};
    }
    if (labelCount == 0 || std::fabs(cycleBeats - labels[labelCount - 1].beat) >= 0.05)
        labels[labelCount++] = {bar.right, cycleBeats};

    constexpr double kMinLabelSpacing = 24.0;
    if (labelCount == 3 &&
        (labels[1].px - labels[0].px < kMinLabelSpacing || labels[2].px - labels[1].px < kMinLabelSpacing)) {
        labels[1] = labels[2];
        labelCount = 2;
    }

    double labelY = bar.bottom + 4;
    auto labelFont = makeOwned<CFontDesc>("Arial", 8.0);
    ctx->setFont(labelFont);
    ctx->setFontColor(CColor(0x88, 0x88, 0xA0, 0xFF));

    for (int i = 0; i < labelCount; ++i) {
        char text[16];
        formatBeat(text, sizeof(text), labels[i].beat);
        CRect lr;
        CHoriTxtAlign align;
        if (i == 0) {
            lr = CRect(labels[i].px, labelY, labels[i].px + 30, labelY + 10);
            align = kLeftText;
        } else if (i == labelCount - 1) {
            lr = CRect(labels[i].px - 30, labelY, labels[i].px, labelY + 10);
            align = kRightText;
        } else {
            lr = CRect(labels[i].px - 15, labelY, labels[i].px + 15, labelY + 10);
            align = kCenterText;
        }
        ctx->drawString(text, lr, align);
    }
}

void LaneEditView::draw(CDrawContext* context) {
    auto bounds = getViewSize();

    selectedLane_ = static_cast<int>(std::round(controller_->getParamNormalized(ParamIDs::kSelectedLane) * 7.0));

    context->setFillColor(CColor(0x1E, 0x1E, 0x26, 0xFF));
    context->drawRect(bounds, kDrawFilled);

    auto tabFont = makeOwned<CFontDesc>("Arial", 9.0);
    context->setFont(tabFont);

    for (int i = 0; i < kMaxLanes; ++i) {
        auto r = laneTabRect(i);
        auto color = kLaneColors[i];

        if (i == selectedLane_) {
            context->setFillColor(color);
            context->drawRect(r, kDrawFilled);
            context->setFontColor(CColor(0xFF, 0xFF, 0xFF, 0xFF));
        } else {
            context->setFillColor(CColor(color.red, color.green, color.blue, 0x28));
            context->drawRect(r, kDrawFilled);
            context->setFrameColor(CColor(color.red, color.green, color.blue, 0x50));
            context->setLineWidth(1.0);
            context->drawRect(r, kDrawStroked);
            context->setFontColor(CColor(0x80, 0x80, 0x90, 0xFF));
        }

        char lbl[4];
        std::snprintf(lbl, sizeof(lbl), "%d", i + 1);
        context->drawString(lbl, r, kCenterText);
    }

    auto laneColor = kLaneColors[selectedLane_];

    auto groupFont = makeOwned<CFontDesc>("Arial", 7.0);
    context->setFont(groupFont);
    context->setFontColor(CColor(0x50, 0x50, 0x60, 0xFF));
    CRect patternLabel(bounds.left + 268, bounds.top + 4, bounds.left + 420, bounds.top + 14);
    context->drawString("Pattern", patternLabel, kLeftText);
    CRect voiceLabel(bounds.left + 422, bounds.top + 4, bounds.left + 575, bounds.top + 14);
    context->drawString("Voice", voiceLabel, kLeftText);

    context->setLineWidth(0.5);
    context->setFrameColor(CColor(0x30, 0x30, 0x40, 0x60));
    double divX = bounds.left + 416;
    context->drawLine(CPoint(divX, bounds.top + 14), CPoint(divX, bounds.top + 65));

    for (int k = 0; k < kLaneKnobCount; ++k) {
        auto r = laneKnobRect(k);
        auto paramId = paramIdForKnob(kLaneKnobs[k], selectedLane_);
        double value = controller_->getParamNormalized(paramId);
        drawKnob(context, r, value, laneColor, kLaneKnobs[k]);
    }

    auto phraseFont = makeOwned<CFontDesc>("Arial", 8.0);
    context->setFont(phraseFont);
    context->setFontColor(CColor(0x58, 0x58, 0x70, 0xFF));
    CRect phraseLabel(bounds.left + 10, bounds.top + 69, bounds.left + 90, bounds.top + 79);
    context->drawString("Phrase", phraseLabel, kLeftText);

    auto lenParamId = paramIdForKnob(kPhraseKnobs[0], selectedLane_);
    double lenValue = controller_->getParamNormalized(lenParamId);
    bool lenActive = lenValue > 0.005;

    double gapValue = controller_->getParamNormalized(paramIdForKnob(kPhraseKnobs[1], selectedLane_));
    double ofsValue = controller_->getParamNormalized(paramIdForKnob(kPhraseKnobs[2], selectedLane_));
    drawPhraseSchematic(context, laneColor, lenValue * kPhraseKnobs[0].displayMax,
                        gapValue * kPhraseKnobs[1].displayMax, ofsValue * kPhraseKnobs[2].displayMax);

    for (int k = 0; k < kPhraseKnobCount; ++k) {
        auto r = phraseKnobRect(k);
        auto paramId = paramIdForKnob(kPhraseKnobs[k], selectedLane_);
        double value = controller_->getParamNormalized(paramId);
        bool enabled = (k == 0) || (k >= 1 && k <= 2 && lenActive) || (k >= 3);
        drawKnob(context, r, value, laneColor, kPhraseKnobs[k], enabled);
    }

    setDirty(false);
}

CMouseEventResult LaneEditView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    if (!(buttons & kLButton))
        return kMouseEventNotHandled;

    int tab = hitTestTab(where);
    if (tab >= 0) {
        selectedLane_ = tab;
        double norm = tab / 7.0;
        controller_->beginEdit(ParamIDs::kSelectedLane);
        controller_->setParamNormalized(ParamIDs::kSelectedLane, norm);
        controller_->performEdit(ParamIDs::kSelectedLane, norm);
        controller_->endEdit(ParamIDs::kSelectedLane);
        invalid();
        return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    int laneKnob = hitTestLaneKnob(where);
    if (laneKnob >= 0) {
        dragTarget_ = DragTarget::LaneKnob;
        dragKnob_ = laneKnob;
        dragStartY_ = where.y;
        auto paramId = paramIdForKnob(kLaneKnobs[laneKnob], selectedLane_);
        dragStartValue_ = controller_->getParamNormalized(paramId);
        controller_->beginEdit(paramId);
        return kMouseEventHandled;
    }

    int phraseKnob = hitTestPhraseKnob(where);
    if (phraseKnob >= 0) {
        if (phraseKnob >= 1 && phraseKnob <= 2) {
            auto lenId = paramIdForKnob(kPhraseKnobs[0], selectedLane_);
            if (controller_->getParamNormalized(lenId) < 0.005)
                return kMouseEventNotHandled;
        }
        dragTarget_ = DragTarget::PhraseKnob;
        dragKnob_ = phraseKnob;
        dragStartY_ = where.y;
        auto paramId = paramIdForKnob(kPhraseKnobs[phraseKnob], selectedLane_);
        dragStartValue_ = controller_->getParamNormalized(paramId);
        controller_->beginEdit(paramId);
        return kMouseEventHandled;
    }

    return kMouseEventNotHandled;
}

CMouseEventResult LaneEditView::onMouseMoved(CPoint& where, const CButtonState& buttons) {
    if (dragTarget_ == DragTarget::None)
        return kMouseEventNotHandled;

    double delta = (dragStartY_ - where.y) / kDragSensitivity;
    double newValue = dragStartValue_ + delta;
    if (newValue < 0.0)
        newValue = 0.0;
    if (newValue > 1.0)
        newValue = 1.0;

    const auto& knobs = (dragTarget_ == DragTarget::LaneKnob) ? kLaneKnobs[dragKnob_] : kPhraseKnobs[dragKnob_];
    auto paramId = paramIdForKnob(knobs, selectedLane_);
    controller_->setParamNormalized(paramId, newValue);
    controller_->performEdit(paramId, newValue);
    invalid();
    return kMouseEventHandled;
}

CMouseEventResult LaneEditView::onMouseUp(CPoint& where, const CButtonState& buttons) {
    if (dragTarget_ == DragTarget::None)
        return kMouseEventNotHandled;

    const auto& knobs = (dragTarget_ == DragTarget::LaneKnob) ? kLaneKnobs[dragKnob_] : kPhraseKnobs[dragKnob_];
    auto paramId = paramIdForKnob(knobs, selectedLane_);
    controller_->endEdit(paramId);
    dragTarget_ = DragTarget::None;
    dragKnob_ = -1;
    return kMouseEventHandled;
}

} // namespace poly
