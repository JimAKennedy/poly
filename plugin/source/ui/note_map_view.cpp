#include "note_map_view.h"

#include <algorithm>
#include <cstdio>

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/coptionmenu.h"

#include "../controller.h"
#include "../plugids.h"
#include "poly/types.h"

namespace poly {

namespace {
static constexpr const char* kNoteLetters[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

struct DrumName {
    int16_t note;
    const char* name;
};
static constexpr DrumName kDrumNames[] = {
    {35, "Kick 2"},     {36, "Kick 1"},      {37, "Side Stick"},   {38, "Snare"},        {39, "Clap"},
    {40, "Elec Snare"}, {41, "Floor Tom L"}, {42, "Closed HH"},    {43, "Floor Tom H"},  {44, "Pedal HH"},
    {45, "Low Tom"},    {46, "Open HH"},     {47, "Low-Mid Tom"},  {48, "Hi-Mid Tom"},   {49, "Crash 1"},
    {50, "High Tom"},   {51, "Ride 1"},      {52, "China"},        {53, "Ride Bell"},    {54, "Tambourine"},
    {55, "Splash"},     {56, "Cowbell"},     {57, "Crash 2"},      {58, "Vibraslap"},    {59, "Ride 2"},
    {60, "Hi Bongo"},   {61, "Lo Bongo"},    {62, "Mute Conga H"}, {63, "Open Conga H"}, {64, "Low Conga"},
    {65, "Hi Timbale"}, {66, "Lo Timbale"},  {67, "Hi Agogo"},     {68, "Lo Agogo"},     {69, "Cabasa"},
    {70, "Maracas"},    {71, "Whistle S"},   {72, "Whistle L"},    {73, "Guiro S"},      {74, "Guiro L"},
    {75, "Claves"},     {76, "Woodblock H"}, {77, "Woodblock L"},  {78, "Cuica Mute"},   {79, "Cuica Open"},
    {80, "Triangle M"}, {81, "Triangle O"},
};
static constexpr int kDrumNameCount = static_cast<int>(sizeof(kDrumNames) / sizeof(kDrumNames[0]));
} // namespace

NoteMapView::NoteMapView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* controller)
    : CView(size), controller_(controller) {
    setMouseEnabled(true);
    buildRows();
}

const char* NoteMapView::noteLetter(int16_t note) {
    if (note < 0 || note > 127)
        return "?";
    return kNoteLetters[note % 12];
}

int NoteMapView::noteOctave(int16_t note) {
    return (note / 12) - 2;
}

const char* NoteMapView::gmDrumName(int16_t note) {
    for (int i = 0; i < kDrumNameCount; ++i) {
        if (kDrumNames[i].note == note)
            return kDrumNames[i].name;
    }
    return nullptr;
}

void NoteMapView::buildRows() {
    auto* polyCtrl = static_cast<PolyController*>(controller_);
    const auto& gs = polyCtrl->cachedState().sceneA;
    rowCount_ = 0;
    for (int lane = 0; lane < kMaxLanes; ++lane) {
        rows_[static_cast<size_t>(rowCount_)] = {gs.lanes[lane].midiNote, lane};
        ++rowCount_;
    }
}

VSTGUI::CRect NoteMapView::closeButtonRect() const {
    auto b = getViewSize();
    return {b.right - 36, b.top + kTopPadding, b.right - 8, b.top + kTopPadding + 28};
}

VSTGUI::CRect NoteMapView::resetButtonRect() const {
    auto b = getViewSize();
    return {b.right - 110, b.top + kTopPadding, b.right - 44, b.top + kTopPadding + 28};
}

VSTGUI::CRect NoteMapView::destRect(int row) const {
    auto b = getViewSize();
    double y = b.top + kHeaderHeight + 24 + row * kRowHeight;
    return {b.left + 340, y, b.right - 20, y + kRowHeight - 2};
}

int NoteMapView::hitTestRow(const VSTGUI::CPoint& where) const {
    for (int i = 0; i < rowCount_; ++i) {
        if (destRect(i).pointInside(where))
            return i;
    }
    return -1;
}

void NoteMapView::draw(VSTGUI::CDrawContext* context) {
    using namespace VSTGUI;

    buildRows();

    auto bounds = getViewSize();

    context->setFillColor(CColor(0x14, 0x14, 0x1C, 0xF8));
    context->drawRect(bounds, kDrawFilled);

    auto titleFont = makeOwned<CFontDesc>("Arial", 13.0, kBoldFace);
    context->setFont(titleFont);
    context->setFontColor(CColor(0x4A, 0x9E, 0xFF, 0xFF));
    CRect titleRect(bounds.left + 16, bounds.top + kTopPadding, bounds.left + 200, bounds.top + kTopPadding + 28);
    context->drawString("NOTE MAP", titleRect, kLeftText);

    auto btnFont = makeOwned<CFontDesc>("Arial", 10.0);
    context->setFont(btnFont);

    auto resetRect = resetButtonRect();
    context->setFillColor(CColor(0x2A, 0x2A, 0x38, 0xFF));
    context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
    context->setLineWidth(1.0);
    context->drawRect(resetRect, kDrawFilledAndStroked);
    context->setFontColor(CColor(0xCC, 0xCC, 0xD0, 0xFF));
    context->drawString("Reset", resetRect, kCenterText);

    auto closeRect = closeButtonRect();
    context->setFillColor(CColor(0x3A, 0x20, 0x20, 0xFF));
    context->setFrameColor(CColor(0x60, 0x30, 0x30, 0xFF));
    context->drawRect(closeRect, kDrawFilledAndStroked);
    context->setFontColor(CColor(0xE0, 0x80, 0x80, 0xFF));
    context->drawString("\xC3\x97", closeRect, kCenterText);

    CRect divider(bounds.left + 8, bounds.top + kHeaderHeight - 2, bounds.right - 8, bounds.top + kHeaderHeight);
    context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0x40));
    context->drawRect(divider, kDrawFilled);

    auto headerFont = makeOwned<CFontDesc>("Arial", 9.0, kBoldFace);
    context->setFont(headerFont);
    context->setFontColor(CColor(0x66, 0x66, 0x7A, 0xFF));
    double colY = bounds.top + kHeaderHeight + 4;
    context->drawString("LANE", CRect(bounds.left + 16, colY, bounds.left + 70, colY + 16), kLeftText);
    context->drawString("SOURCE", CRect(bounds.left + 140, colY, bounds.left + 280, colY + 16), kLeftText);
    context->drawString("DESTINATION", CRect(bounds.left + 340, colY, bounds.right - 20, colY + 16), kLeftText);

    auto* polyCtrl = static_cast<PolyController*>(controller_);
    const auto& noteMap = polyCtrl->cachedState().noteMap;

    auto rowFont = makeOwned<CFontDesc>("Arial", 11.0);
    context->setFont(rowFont);

    for (int i = 0; i < rowCount_; ++i) {
        double y = bounds.top + kHeaderHeight + 24 + i * kRowHeight;
        const auto& row = rows_[static_cast<size_t>(i)];
        int16_t src = row.sourceNote;
        int16_t dst = noteMap.map[static_cast<size_t>(src)];
        bool isRemapped = (src != dst);

        if (i % 2 == 0) {
            context->setFillColor(CColor(0x1A, 0x1A, 0x24, 0xFF));
            context->drawRect(CRect(bounds.left + 8, y, bounds.right - 8, y + kRowHeight), kDrawFilled);
        }

        if (isRemapped) {
            context->setFillColor(CColor(0x4A, 0x9E, 0xFF, 0x15));
            context->drawRect(CRect(bounds.left + 8, y, bounds.right - 8, y + kRowHeight), kDrawFilled);
        }

        char laneLabel[16];
        std::snprintf(laneLabel, sizeof(laneLabel), "%d", row.laneIndex + 1);
        context->setFontColor(CColor(0x4A, 0x9E, 0xFF, 0xFF));
        context->drawString(laneLabel, CRect(bounds.left + 16, y, bounds.left + 40, y + kRowHeight), kLeftText);

        const char* drumName = gmDrumName(src);
        if (drumName) {
            context->setFontColor(CColor(0xBB, 0xBB, 0xC8, 0xFF));
            context->drawString(drumName, CRect(bounds.left + 44, y, bounds.left + 140, y + kRowHeight), kLeftText);
        }

        char srcLabel[16];
        std::snprintf(srcLabel, sizeof(srcLabel), "%s%d (%d)", noteLetter(src), noteOctave(src), src);
        context->setFontColor(CColor(0xCC, 0xCC, 0xD0, 0xFF));
        context->drawString(srcLabel, CRect(bounds.left + 140, y, bounds.left + 240, y + kRowHeight), kLeftText);

        context->setFontColor(CColor(0x66, 0x66, 0x7A, 0xFF));
        context->drawString("\xE2\x86\x92", CRect(bounds.left + 260, y, bounds.left + 340, y + kRowHeight),
                            kCenterText);

        auto dRect = destRect(i);
        context->setFillColor(isRemapped ? CColor(0x2A, 0x3A, 0x4A, 0xFF) : CColor(0x22, 0x22, 0x30, 0xFF));
        context->setFrameColor(CColor(0x3A, 0x3A, 0x48, 0xFF));
        context->drawRect(dRect, kDrawFilledAndStroked);

        char dstLabel[32];
        const char* dstName = gmDrumName(dst);
        if (dstName) {
            std::snprintf(dstLabel, sizeof(dstLabel), "%s%d (%d) %s", noteLetter(dst), noteOctave(dst), dst, dstName);
        } else {
            std::snprintf(dstLabel, sizeof(dstLabel), "%s%d (%d)", noteLetter(dst), noteOctave(dst), dst);
        }
        context->setFontColor(isRemapped ? CColor(0x4A, 0x9E, 0xFF, 0xFF) : CColor(0xCC, 0xCC, 0xD0, 0xFF));
        CRect textDst = dRect;
        textDst.left += 6;
        textDst.right -= 18;
        context->drawString(dstLabel, textDst, kLeftText);

        context->setFontColor(CColor(0x88, 0x88, 0xA0, 0xFF));
        CRect arrowDst = dRect;
        arrowDst.left = arrowDst.right - 20;
        context->drawString("\xe2\x96\xbe", arrowDst, kCenterText);
    }

    setDirty(false);
}

void NoteMapView::showNoteMenu(int row, const VSTGUI::CPoint& where) {
    auto menu = VSTGUI::makeOwned<VSTGUI::COptionMenu>(VSTGUI::CRect(0, 0, 0, 0), nullptr, -1);

    for (int i = 0; i < kDrumNameCount; ++i) {
        char label[48];
        int16_t n = kDrumNames[i].note;
        std::snprintf(label, sizeof(label), "%s%d (%d) - %s", noteLetter(n), noteOctave(n), n, kDrumNames[i].name);
        menu->addEntry(label);
    }

    VSTGUI::CPoint framePos = where;
    localToFrame(framePos);

    if (menu->popup(getFrame(), framePos)) {
        auto result = menu->getLastResult();
        if (result >= 0 && result < kDrumNameCount) {
            int16_t srcNote = rows_[static_cast<size_t>(row)].sourceNote;
            int16_t newDst = kDrumNames[result].note;

            auto* polyCtrl = static_cast<PolyController*>(controller_);
            polyCtrl->mutableCachedState().noteMap.map[static_cast<size_t>(srcNote)] = newDst;
            polyCtrl->sendNoteMap();
            invalid();
        }
    }
}

VSTGUI::CMouseEventResult NoteMapView::onMouseDown(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {
    if (!(buttons & VSTGUI::kLButton))
        return VSTGUI::kMouseEventNotHandled;

    if (closeButtonRect().pointInside(where)) {
        if (auto* frame = getFrame()) {
            frame->removeView(this, true);
        }
        return VSTGUI::kMouseEventHandled;
    }

    if (resetButtonRect().pointInside(where)) {
        auto* polyCtrl = static_cast<PolyController*>(controller_);
        polyCtrl->mutableCachedState().noteMap.reset();
        polyCtrl->sendNoteMap();
        invalid();
        return VSTGUI::kMouseEventHandled;
    }

    int row = hitTestRow(where);
    if (row >= 0) {
        showNoteMenu(row, where);
        return VSTGUI::kMouseEventHandled;
    }

    return VSTGUI::kMouseEventHandled;
}

} // namespace poly
