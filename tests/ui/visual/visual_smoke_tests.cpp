//------------------------------------------------------------------------
// Visual regression tests for Poly's custom VSTGUI views.
// Renders LaneGridView and VelocityView in multiple parameter states,
// compares against committed baseline PNGs using pixel comparison.
//------------------------------------------------------------------------

#include "image_compare.h"
#include "visual_test_harness.h"

#include <gtest/gtest.h>

#include "controller.h"
#include "plugids.h"
#include "ui/lane_grid_view.h"
#include "ui/velocity_view.h"

#include <filesystem>
#include <fstream>

// vstguieditor.cpp requires this symbol (normally from macmain.cpp)
void* moduleHandle = nullptr;

using namespace JKDigital::VisualTest;
namespace fs = std::filesystem;

//------------------------------------------------------------------------
// Base fixture: manages controller lifecycle and provides render helpers
//------------------------------------------------------------------------
class VisualRegressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        initPlatformOnce();

        controller_ = new poly::PolyController();
        ASSERT_NE(controller_, nullptr);

        auto result = controller_->initialize(nullptr);
        ASSERT_TRUE(result == Steinberg::kResultOk ||
                    result == Steinberg::kResultTrue);
    }

    void TearDown() override {
        if (controller_) {
            controller_->terminate();
            controller_->release();
        }
    }

    void setParam(Steinberg::Vst::ParamID id, double normalized) {
        controller_->setParamNormalized(id, normalized);
    }

    struct RegressionResult {
        bool rendered = false;
        bool baselineExisted = false;
        bool matched = false;
        CompareResult compare;
    };

    RegressionResult checkRegression(VSTGUI::CView* view,
                                     const std::string& name,
                                     double maxDiffPercent = 0.5,
                                     uint8_t tolerance = 2) {
        RegressionResult rr;

        auto bitmap = renderViewToBitmap(view);
        if (!bitmap) return rr;
        rr.rendered = true;

        std::string outPath = getOutputDir() + "/" + name + ".png";
        EXPECT_TRUE(saveBitmapToPNG(bitmap, outPath))
            << "Failed to save output PNG: " << outPath;

        std::string refPath = getReferenceDir() + "/" + name + ".png";

        if (!fs::exists(refPath)) {
            fs::create_directories(getReferenceDir());
            EXPECT_TRUE(saveBitmapToPNG(bitmap, refPath))
                << "Failed to create baseline: " << refPath;
            std::cout << "  [BASELINE CREATED] " << refPath << std::endl;
            rr.baselineExisted = false;
            rr.matched = true;
            return rr;
        }

        rr.baselineExisted = true;

        uint32_t outW = 0, outH = 0, refW = 0, refH = 0;
        std::vector<uint8_t> outPixels, refPixels;

        EXPECT_TRUE(loadPNG(outPath, outW, outH, outPixels))
            << "Failed to load output PNG";
        EXPECT_TRUE(loadPNG(refPath, refW, refH, refPixels))
            << "Failed to load reference PNG";

        EXPECT_EQ(outW, refW) << "Width mismatch for " << name;
        EXPECT_EQ(outH, refH) << "Height mismatch for " << name;

        if (outW != refW || outH != refH) return rr;

        rr.compare =
            compareImages(outPixels, refPixels, outW, outH, tolerance,
                          maxDiffPercent);
        rr.matched = rr.compare.matched;

        if (!rr.matched) {
            std::string diffPath = getOutputDir() + "/" + name + "_diff.png";
            generateDiffImage(outPixels, refPixels, outW, outH, diffPath,
                              tolerance);
            std::cout << "  [DIFF] " << rr.compare.diffPercentage
                      << "% pixels differ (" << rr.compare.diffPixels << "/"
                      << rr.compare.totalPixels << "). Diff: " << diffPath
                      << std::endl;
        }

        return rr;
    }

    Steinberg::Vst::EditController* controller_ = nullptr;
};

//------------------------------------------------------------------------
// LaneGridView regression tests
//------------------------------------------------------------------------

TEST_F(VisualRegressionTest, LaneGridDefault) {
    auto* view =
        new poly::LaneGridView(VSTGUI::CRect(0, 0, 580, 160), controller_);

    auto rr = checkRegression(view, "lane_grid_default");
    EXPECT_TRUE(rr.rendered);
    EXPECT_TRUE(rr.matched)
        << "LaneGridView default rendering differs from baseline";

    delete view;
}

TEST_F(VisualRegressionTest, LaneGridHighComplexity) {
    setParam(poly::ParamIDs::kMacroComplexity, 1.0);

    auto* view =
        new poly::LaneGridView(VSTGUI::CRect(0, 0, 580, 160), controller_);

    auto rr = checkRegression(view, "lane_grid_high_complexity");
    EXPECT_TRUE(rr.rendered);
    EXPECT_TRUE(rr.matched)
        << "LaneGridView high-complexity rendering differs from baseline";

    delete view;
}

TEST_F(VisualRegressionTest, LaneGridMaxLanes) {
    setParam(poly::ParamIDs::kActiveLaneCount, 1.0);

    auto* view =
        new poly::LaneGridView(VSTGUI::CRect(0, 0, 580, 160), controller_);

    auto rr = checkRegression(view, "lane_grid_max_lanes");
    EXPECT_TRUE(rr.rendered);
    EXPECT_TRUE(rr.matched)
        << "LaneGridView max-lanes rendering differs from baseline";

    delete view;
}

//------------------------------------------------------------------------
// VelocityView regression tests
//------------------------------------------------------------------------

TEST_F(VisualRegressionTest, VelocityDefault) {
    auto* view =
        new poly::VelocityView(VSTGUI::CRect(0, 0, 580, 80), controller_);

    auto rr = checkRegression(view, "velocity_default");
    EXPECT_TRUE(rr.rendered);
    EXPECT_TRUE(rr.matched)
        << "VelocityView default rendering differs from baseline";

    delete view;
}

TEST_F(VisualRegressionTest, VelocityHighDensity) {
    setParam(poly::ParamIDs::kMacroDensity, 1.0);

    auto* view =
        new poly::VelocityView(VSTGUI::CRect(0, 0, 580, 80), controller_);

    auto rr = checkRegression(view, "velocity_high_density");
    EXPECT_TRUE(rr.rendered);
    EXPECT_TRUE(rr.matched)
        << "VelocityView high-density rendering differs from baseline";

    delete view;
}

TEST_F(VisualRegressionTest, VelocityHighTension) {
    setParam(poly::ParamIDs::kMacroTension, 1.0);

    auto* view =
        new poly::VelocityView(VSTGUI::CRect(0, 0, 580, 80), controller_);

    auto rr = checkRegression(view, "velocity_high_tension");
    EXPECT_TRUE(rr.rendered);
    EXPECT_TRUE(rr.matched)
        << "VelocityView high-tension rendering differs from baseline";

    delete view;
}
