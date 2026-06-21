//------------------------------------------------------------------------
// Headless UI interaction tests for Poly.
// Tests controller lifecycle, macro knob interaction, and view tree.
//------------------------------------------------------------------------

#include "headless_ui_host.h"

#include <gtest/gtest.h>

#include "controller.h"
#include "plugids.h"
#include "ui/lane_grid_view.h"
#include "ui/velocity_view.h"

#include "vstgui/lib/cviewcontainer.h"

using namespace JKDigital::InteractionTest;

namespace {

auto makeControllerFactory() {
    return []() -> Steinberg::Vst::IEditController* {
        return static_cast<Steinberg::Vst::IEditController*>(
            poly::PolyController::createInstance(nullptr));
    };
}

}  // namespace

// -----------------------------------------------------------------------
// Smoke tests
// -----------------------------------------------------------------------

class InteractionSmokeTest : public ::testing::Test {
protected:
    void SetUp() override { initPlatformOnce(); }
};

TEST_F(InteractionSmokeTest, ControllerLifecycle) {
    HeadlessUIHost host{makeControllerFactory(), UIDESC_RESOURCE_DIR};

#ifndef __APPLE__
    GTEST_SKIP() << "Headless UI host requires macOS";
#endif

    ASSERT_TRUE(host.open()) << "Failed to open headless UI host";
    EXPECT_TRUE(host.isOpen());

    double complexity =
        host.getParameterValue(poly::ParamIDs::kMacroComplexity);
    EXPECT_GE(complexity, 0.0);
    EXPECT_LE(complexity, 1.0);

    EXPECT_NE(host.getFrame(), nullptr) << "CFrame should be available";

    host.close();
    EXPECT_FALSE(host.isOpen());
}

TEST_F(InteractionSmokeTest, MacroKnobExists) {
    HeadlessUIHost host{makeControllerFactory(), UIDESC_RESOURCE_DIR};

#ifndef __APPLE__
    GTEST_SKIP() << "Headless UI host requires macOS";
#endif

    ASSERT_TRUE(host.open());

    auto center =
        host.getControlCenter(poly::ParamIDs::kMacroComplexity);
    EXPECT_NE(center, VSTGUI::CPoint(-1, -1))
        << "Complexity knob should be findable by tag";

    host.close();
}

// -----------------------------------------------------------------------
// Macro knob interaction tests
// -----------------------------------------------------------------------

class MacroKnobTest : public ::testing::Test {
protected:
    void SetUp() override {
        initPlatformOnce();
#ifndef __APPLE__
        GTEST_SKIP() << "Headless UI host requires macOS";
#endif
        host_ = std::make_unique<HeadlessUIHost>(
            makeControllerFactory(), UIDESC_RESOURCE_DIR);
        ASSERT_TRUE(host_->open());
    }

    void TearDown() override {
        if (host_ && host_->isOpen())
            host_->close();
    }

    std::unique_ptr<HeadlessUIHost> host_;
};

TEST_F(MacroKnobTest, ScrollComplexityChangesValue) {
    auto center = host_->getControlCenter(poly::ParamIDs::kMacroComplexity);
    ASSERT_NE(center, VSTGUI::CPoint(-1, -1));

    double before = host_->getParameterValue(poly::ParamIDs::kMacroComplexity);
    host_->simulateScroll(center.x, center.y, 5.0f);
    double after = host_->getParameterValue(poly::ParamIDs::kMacroComplexity);

    EXPECT_NE(before, after)
        << "Scrolling on Complexity knob should change parameter";
}

TEST_F(MacroKnobTest, ScrollDensityChangesValue) {
    auto center = host_->getControlCenter(poly::ParamIDs::kMacroDensity);
    ASSERT_NE(center, VSTGUI::CPoint(-1, -1));

    double before = host_->getParameterValue(poly::ParamIDs::kMacroDensity);
    host_->simulateScroll(center.x, center.y, 5.0f);
    double after = host_->getParameterValue(poly::ParamIDs::kMacroDensity);

    EXPECT_NE(before, after)
        << "Scrolling on Density knob should change parameter";
}

TEST_F(MacroKnobTest, AllMacroKnobsDiscoverable) {
    const Steinberg::Vst::ParamID macroIds[] = {
        poly::ParamIDs::kMacroComplexity,
        poly::ParamIDs::kMacroDensity,
        poly::ParamIDs::kMacroSyncopation,
        poly::ParamIDs::kMacroSwing,
        poly::ParamIDs::kMacroTension,
        poly::ParamIDs::kMacroHumanize,
    };

    for (auto id : macroIds) {
        auto center = host_->getControlCenter(id);
        EXPECT_NE(center, VSTGUI::CPoint(-1, -1))
            << "Macro knob with tag " << id << " should be discoverable";

        auto rect = host_->getControlRect(id);
        EXPECT_GT(rect.getWidth(), 0) << "Knob " << id << " should have width";
        EXPECT_GT(rect.getHeight(), 0) << "Knob " << id << " should have height";
    }
}

TEST_F(MacroKnobTest, ScrollGeneratesEditLog) {
    host_->clearEditLog();

    auto center = host_->getControlCenter(poly::ParamIDs::kMacroComplexity);
    ASSERT_NE(center, VSTGUI::CPoint(-1, -1));

    host_->simulateScroll(center.x, center.y, 3.0f);

    const auto& edits = host_->getEditLog();
    EXPECT_FALSE(edits.empty())
        << "Scrolling should generate parameter edits in the log";

    bool foundComplexity = false;
    for (const auto& edit : edits) {
        if (edit.paramId == poly::ParamIDs::kMacroComplexity) {
            foundComplexity = true;
            EXPECT_GE(edit.value, 0.0);
            EXPECT_LE(edit.value, 1.0);
        }
    }
    EXPECT_TRUE(foundComplexity)
        << "Edit log should contain Complexity parameter edits";
}

TEST_F(MacroKnobTest, DragSyncopationChangesValue) {
    auto center = host_->getControlCenter(poly::ParamIDs::kMacroSyncopation);
    ASSERT_NE(center, VSTGUI::CPoint(-1, -1));

    double before = host_->getParameterValue(poly::ParamIDs::kMacroSyncopation);
    host_->simulateDrag(center.x, center.y, center.x, center.y - 30);
    double after = host_->getParameterValue(poly::ParamIDs::kMacroSyncopation);

    EXPECT_NE(before, after)
        << "Dragging Syncopation knob upward should change parameter";
}

// -----------------------------------------------------------------------
// Controller lifecycle edge cases
// -----------------------------------------------------------------------

class LifecycleTest : public ::testing::Test {
protected:
    void SetUp() override {
        initPlatformOnce();
#ifndef __APPLE__
        GTEST_SKIP() << "Headless UI host requires macOS";
#endif
    }
};

TEST_F(LifecycleTest, DoubleOpenPrevented) {
    HeadlessUIHost host{makeControllerFactory(), UIDESC_RESOURCE_DIR};
    ASSERT_TRUE(host.open());
    EXPECT_FALSE(host.open()) << "Second open() should return false";
    host.close();
}

TEST_F(LifecycleTest, OpenCloseReopenCycle) {
    HeadlessUIHost host{makeControllerFactory(), UIDESC_RESOURCE_DIR};

    ASSERT_TRUE(host.open());
    EXPECT_TRUE(host.isOpen());

    host.close();
    EXPECT_FALSE(host.isOpen());

    EXPECT_TRUE(host.open()) << "Reopen after close should succeed";
    EXPECT_TRUE(host.isOpen());

    host.close();
}

TEST_F(LifecycleTest, ParameterReadBeforeOpen) {
    HeadlessUIHost host{makeControllerFactory(), UIDESC_RESOURCE_DIR};
    double val = host.getParameterValue(poly::ParamIDs::kMacroComplexity);
    EXPECT_EQ(val, 0.0) << "Parameter read before open should return 0";
}

TEST_F(LifecycleTest, EditLogClearWorks) {
    HeadlessUIHost host{makeControllerFactory(), UIDESC_RESOURCE_DIR};
    ASSERT_TRUE(host.open());

    auto center = host.getControlCenter(poly::ParamIDs::kMacroComplexity);
    if (center != VSTGUI::CPoint(-1, -1)) {
        host.simulateScroll(center.x, center.y, 3.0f);
        EXPECT_FALSE(host.getEditLog().empty());
    }

    host.clearEditLog();
    EXPECT_TRUE(host.getEditLog().empty())
        << "Edit log should be empty after clearing";

    host.close();
}

// -----------------------------------------------------------------------
// View tree inspection
// -----------------------------------------------------------------------

class ViewTreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        initPlatformOnce();
#ifndef __APPLE__
        GTEST_SKIP() << "Headless UI host requires macOS";
#endif
        host_ = std::make_unique<HeadlessUIHost>(
            makeControllerFactory(), UIDESC_RESOURCE_DIR);
        ASSERT_TRUE(host_->open());
    }

    void TearDown() override {
        if (host_ && host_->isOpen())
            host_->close();
    }

    std::unique_ptr<HeadlessUIHost> host_;
};

TEST_F(ViewTreeTest, FrameHasChildren) {
    auto* frame = host_->getFrame();
    ASSERT_NE(frame, nullptr);
    EXPECT_GT(frame->getNbViews(), 0u)
        << "CFrame should have child views from uidesc";
}

TEST_F(ViewTreeTest, MacroKnobBoundsAreSensible) {
    auto rect = host_->getControlRect(poly::ParamIDs::kMacroComplexity);
    EXPECT_EQ(rect.getWidth(), 50) << "Complexity knob should be 50px wide";
    EXPECT_EQ(rect.getHeight(), 50) << "Complexity knob should be 50px tall";
    EXPECT_GT(rect.left, 0) << "Knob should not be at origin";
}

TEST_F(ViewTreeTest, CustomViewsInTree) {
    auto* frame = host_->getFrame();
    ASSERT_NE(frame, nullptr);

    bool foundLaneGrid = false;
    bool foundVelocity = false;

    std::function<void(VSTGUI::CViewContainer*)> walk =
        [&](VSTGUI::CViewContainer* container) {
            for (uint32_t i = 0; i < container->getNbViews(); ++i) {
                auto* child = container->getView(i);
                if (!child)
                    continue;
                if (dynamic_cast<poly::LaneGridView*>(child))
                    foundLaneGrid = true;
                if (dynamic_cast<poly::VelocityView*>(child))
                    foundVelocity = true;
                auto* sub = dynamic_cast<VSTGUI::CViewContainer*>(child);
                if (sub)
                    walk(sub);
            }
        };

    walk(frame);

    EXPECT_TRUE(foundLaneGrid) << "LaneGridView should be in the view tree";
    EXPECT_TRUE(foundVelocity) << "VelocityView should be in the view tree";
}
