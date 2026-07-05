#include <algorithm>
#include <array>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "choc/text/choc_JSON.h"
#include "poly/presets.h"
#include "poly/scene.h"
#include "webui/bridge_params.h"
#include "webui/bridge_serialization.h"

using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static json loadFixture(const char* filename) {
    std::string path = std::string(FIXTURE_DIR) + "/" + filename;
    std::ifstream f(path);
    EXPECT_TRUE(f.good()) << "Cannot open fixture: " << path;
    return json::parse(f);
}

static json loadSchema() {
    std::string path = std::string(SCHEMA_DIR) + "/bridge.schema.json";
    std::ifstream f(path);
    EXPECT_TRUE(f.good()) << "Cannot open schema: " << path;
    return json::parse(f);
}

static std::array<std::string, poly::kMaxLanes> kDefaultLaneNames = {"Lane 0", "Lane 1", "Lane 2", "Lane 3",
                                                                     "Lane 4", "Lane 5", "Lane 6", "Lane 7"};

static const std::string& testLaneName(int lane, void* ctx) {
    auto* names = static_cast<std::array<std::string, poly::kMaxLanes>*>(ctx);
    return (*names)[lane];
}

// ---------------------------------------------------------------------------
// T02: ParamId Resolution Tests
// ---------------------------------------------------------------------------

TEST(BridgeContract, EditFixtureParamIdsAllResolve) {
    auto fixture = loadFixture("edits.json");
    for (const auto& msg : fixture["messages"]) {
        std::string paramId = msg["paramId"];
        auto resolved = poly::webui::resolveParamId(paramId.c_str());
        EXPECT_TRUE(resolved.has_value()) << "Unresolved paramId: " << paramId << " (label: " << msg["label"] << ")";
    }
}

TEST(BridgeContract, MacroParamIdsResolve) {
    const char* macros[] = {"macro.complexity", "macro.density", "macro.syncopation",
                            "macro.swing",      "macro.tension", "macro.humanize"};
    for (auto* name : macros) {
        auto id = poly::webui::resolveParamId(name);
        ASSERT_TRUE(id.has_value()) << "macro param not resolved: " << name;
    }
    EXPECT_EQ(*poly::webui::resolveParamId("macro.complexity"), poly::ParamIDs::kMacroComplexity);
    EXPECT_EQ(*poly::webui::resolveParamId("macro.density"), poly::ParamIDs::kMacroDensity);
    EXPECT_EQ(*poly::webui::resolveParamId("macro.syncopation"), poly::ParamIDs::kMacroSyncopation);
    EXPECT_EQ(*poly::webui::resolveParamId("macro.swing"), poly::ParamIDs::kMacroSwing);
    EXPECT_EQ(*poly::webui::resolveParamId("macro.tension"), poly::ParamIDs::kMacroTension);
    EXPECT_EQ(*poly::webui::resolveParamId("macro.humanize"), poly::ParamIDs::kMacroHumanize);
}

TEST(BridgeContract, LaneExprParamIdsResolve) {
    auto vel0 = poly::webui::resolveParamId("lane.0.velocity");
    ASSERT_TRUE(vel0.has_value());
    EXPECT_EQ(*vel0, poly::ParamIDs::laneParam(0, poly::ParamIDs::kBaseVelocity));

    auto prob2 = poly::webui::resolveParamId("lane.2.probability");
    ASSERT_TRUE(prob2.has_value());
    EXPECT_EQ(*prob2, poly::ParamIDs::laneParam(2, poly::ParamIDs::kProbability));

    auto swing7 = poly::webui::resolveParamId("lane.7.swing");
    ASSERT_TRUE(swing7.has_value());
    EXPECT_EQ(*swing7, poly::ParamIDs::laneParam(7, poly::ParamIDs::kSwingAmount));
}

TEST(BridgeContract, LaneCoreParamIdsResolve) {
    auto steps0 = poly::webui::resolveParamId("lane.0.steps");
    ASSERT_TRUE(steps0.has_value());
    EXPECT_EQ(*steps0, poly::ParamIDs::laneCoreParam(0, poly::ParamIDs::kCoreSteps));

    auto note3 = poly::webui::resolveParamId("lane.3.note");
    ASSERT_TRUE(note3.has_value());
    EXPECT_EQ(*note3, poly::ParamIDs::laneCoreParam(3, poly::ParamIDs::kCoreMidiNote));
}

TEST(BridgeContract, SceneParamIdsResolve) {
    EXPECT_EQ(*poly::webui::resolveParamId("scene.morph"), poly::ParamIDs::kSceneMorph);
    EXPECT_EQ(*poly::webui::resolveParamId("scene.select"), poly::ParamIDs::kSceneSelect);
}

TEST(BridgeContract, GlobalParamIdsResolve) {
    EXPECT_EQ(*poly::webui::resolveParamId("activeLaneCount"), poly::ParamIDs::kActiveLaneCount);
    EXPECT_EQ(*poly::webui::resolveParamId("seed"), poly::ParamIDs::kSeed);
}

TEST(BridgeContract, ChainParamIdsResolve) {
    EXPECT_EQ(*poly::webui::resolveParamId("chain.enabled"), poly::ParamIDs::kChainEnabled);
    EXPECT_EQ(*poly::webui::resolveParamId("chain.mode"), poly::ParamIDs::kChainMode);

    auto scene0 = poly::webui::resolveParamId("chain.entry.0.scene");
    ASSERT_TRUE(scene0.has_value());
    EXPECT_EQ(*scene0, poly::ParamIDs::chainEntryParam(0, poly::ParamIDs::kChainEntryScene));

    auto bars1 = poly::webui::resolveParamId("chain.entry.1.bars");
    ASSERT_TRUE(bars1.has_value());
    EXPECT_EQ(*bars1, poly::ParamIDs::chainEntryParam(1, poly::ParamIDs::kChainEntryBars));
}

TEST(BridgeContract, InvalidParamIdsReturnNullopt) {
    EXPECT_FALSE(poly::webui::resolveParamId("nonexistent"));
    EXPECT_FALSE(poly::webui::resolveParamId("macro.nonexistent"));
    EXPECT_FALSE(poly::webui::resolveParamId("lane.0.nonexistent"));
    EXPECT_FALSE(poly::webui::resolveParamId("lane.-1.velocity"));
    EXPECT_FALSE(poly::webui::resolveParamId(""));
}

// ---------------------------------------------------------------------------
// T03: State Serialization Tests
// ---------------------------------------------------------------------------

static json serializeInitPreset() {
    poly::GrooveState gs{};
    gs.activeLaneCount = poly::kMaxLanes;
    static constexpr int kSteps[] = {4, 4, 8, 5, 7, 3, 6, 9};
    static constexpr int kSubs[] = {4, 4, 8, 16, 8, 16, 16, 16};
    static constexpr int kHits[] = {4, 2, 8, 3, 4, 2, 4, 5};
    static constexpr int kNotes[] = {36, 38, 42, 45, 46, 39, 43, 50};
    for (int i = 0; i < poly::kMaxLanes; ++i) {
        gs.lanes[i].id = i;
        gs.lanes[i].cycle = {kSteps[i], kSubs[i]};
        gs.lanes[i].hitCount = kHits[i];
        gs.lanes[i].midiNote = kNotes[i];
        gs.lanes[i].baseVelocity = 100;
        gs.lanes[i].probability = 1.0f;
    }

    poly::SceneState ss{};
    auto names = kDefaultLaneNames;
    std::string jsonStr = poly::grooveStateToJson(gs, ss, testLaneName, &names, "Init");
    return json::parse(jsonStr);
}

TEST(BridgeContract, InitPresetSerializesToValidJson) {
    auto j = serializeInitPreset();
    EXPECT_EQ(j["type"], "state");
    EXPECT_TRUE(j.contains("state"));
}

TEST(BridgeContract, StateHasRequiredTopLevelFields) {
    auto j = serializeInitPreset();
    const auto& s = j["state"];

    EXPECT_TRUE(s.contains("preset"));
    EXPECT_TRUE(s.contains("seed"));
    EXPECT_TRUE(s.contains("scene"));
    EXPECT_TRUE(s.contains("macros"));
    EXPECT_TRUE(s.contains("lanes"));
    EXPECT_TRUE(s.contains("presets"));
    EXPECT_TRUE(s.contains("chain"));
    EXPECT_TRUE(s.contains("noteMap"));

    EXPECT_TRUE(s["preset"].is_string());
    EXPECT_TRUE(s["seed"].is_number_integer());
    EXPECT_TRUE(s["scene"].is_string());
    EXPECT_TRUE(s["macros"].is_object());
    EXPECT_TRUE(s["lanes"].is_array());
    EXPECT_TRUE(s["presets"].is_array());
    EXPECT_TRUE(s["chain"].is_object());
    EXPECT_TRUE(s["noteMap"].is_array());
}

TEST(BridgeContract, StatePresetNameMatches) {
    auto j = serializeInitPreset();
    EXPECT_EQ(j["state"]["preset"], "Init");
}

TEST(BridgeContract, StateLaneCountMatchesActiveLanes) {
    auto j = serializeInitPreset();
    EXPECT_EQ(j["state"]["lanes"].size(), static_cast<size_t>(poly::kMaxLanes));
}

TEST(BridgeContract, StateSceneIsValidEnum) {
    auto j = serializeInitPreset();
    std::string scene = j["state"]["scene"];
    EXPECT_TRUE(scene == "A" || scene == "B" || scene == "Morph") << "Invalid scene: " << scene;
}

TEST(BridgeContract, StateMacrosHaveAllFields) {
    auto j = serializeInitPreset();
    const auto& m = j["state"]["macros"];
    for (auto* key : {"complexity", "density", "syncopation", "swing", "tension", "humanize"}) {
        EXPECT_TRUE(m.contains(key)) << "Missing macro field: " << key;
        EXPECT_TRUE(m[key].is_number()) << "Macro field not a number: " << key;
    }
}

TEST(BridgeContract, StateLaneHasRequiredFields) {
    auto j = serializeInitPreset();
    auto schema = loadSchema();
    auto requiredFields = schema["definitions"]["lane"]["required"];

    for (size_t i = 0; i < j["state"]["lanes"].size(); ++i) {
        const auto& lane = j["state"]["lanes"][i];
        for (const auto& field : requiredFields) {
            std::string key = field;
            EXPECT_TRUE(lane.contains(key)) << "Lane " << i << " missing required field: " << key;
        }
    }
}

TEST(BridgeContract, StateLanePatternLengthMatchesSteps) {
    auto j = serializeInitPreset();
    for (size_t i = 0; i < j["state"]["lanes"].size(); ++i) {
        const auto& lane = j["state"]["lanes"][i];
        int steps = lane["steps"];
        EXPECT_EQ(lane["pattern"].size(), static_cast<size_t>(steps))
            << "Lane " << i << " pattern length mismatch: expected " << steps;
        EXPECT_EQ(lane["mt"].size(), static_cast<size_t>(steps))
            << "Lane " << i << " micro-timing array length mismatch";
        EXPECT_EQ(lane["accents"].size(), static_cast<size_t>(steps))
            << "Lane " << i << " accents array length mismatch";
    }
}

TEST(BridgeContract, StateNoteMapHas128Entries) {
    auto j = serializeInitPreset();
    EXPECT_EQ(j["state"]["noteMap"].size(), 128u);
}

TEST(BridgeContract, StateNoteMapDefaultIsIdentity) {
    auto j = serializeInitPreset();
    for (int i = 0; i < 128; ++i) {
        EXPECT_EQ(j["state"]["noteMap"][i].get<int>(), i) << "NoteMap[" << i << "] should be identity";
    }
}

TEST(BridgeContract, StatePresetsArrayHasFactoryPresets) {
    auto j = serializeInitPreset();
    EXPECT_EQ(j["state"]["presets"].size(), static_cast<size_t>(poly::kFactoryPresetCount));
    for (const auto& p : j["state"]["presets"]) {
        EXPECT_TRUE(p.contains("name"));
        EXPECT_TRUE(p.contains("description"));
        EXPECT_TRUE(p["name"].is_string());
        EXPECT_FALSE(p["name"].get<std::string>().empty());
    }
}

TEST(BridgeContract, StateChainHasRequiredFields) {
    auto j = serializeInitPreset();
    const auto& chain = j["state"]["chain"];
    EXPECT_TRUE(chain.contains("enabled"));
    EXPECT_TRUE(chain.contains("mode"));
    EXPECT_TRUE(chain.contains("entryCount"));
    EXPECT_TRUE(chain.contains("entries"));
    EXPECT_TRUE(chain["enabled"].is_boolean());
    EXPECT_TRUE(chain["entries"].is_array());
}

TEST(BridgeContract, FactoryPresetsSerialization) {
    for (int pi = 0; pi < poly::kFactoryPresetCount; ++pi) {
        auto gs = poly::makeFactoryPreset(pi);
        poly::SceneState ss{};
        auto names = kDefaultLaneNames;
        std::string presetName = poly::getFactoryPresetInfo(pi).name;
        std::string jsonStr = poly::grooveStateToJson(gs, ss, testLaneName, &names, presetName);

        json j;
        ASSERT_NO_THROW(j = json::parse(jsonStr)) << "Invalid JSON for preset " << pi << " (" << presetName << ")";
        EXPECT_EQ(j["type"], "state");
        EXPECT_EQ(j["state"]["preset"], presetName);

        int laneCount = j["state"]["lanes"].size();
        EXPECT_GE(laneCount, 1);
        EXPECT_LE(laneCount, poly::kMaxLanes);

        for (int li = 0; li < laneCount; ++li) {
            const auto& lane = j["state"]["lanes"][li];
            int steps = lane["steps"];
            EXPECT_EQ(lane["pattern"].size(), static_cast<size_t>(steps))
                << "Preset " << presetName << " lane " << li << " pattern/steps mismatch";
        }
    }
}

TEST(BridgeContract, LaneHueIsValidHexColor) {
    auto j = serializeInitPreset();
    for (size_t i = 0; i < j["state"]["lanes"].size(); ++i) {
        std::string hue = j["state"]["lanes"][i]["hue"];
        EXPECT_EQ(hue.size(), 7u) << "Lane " << i << " hue wrong length";
        EXPECT_EQ(hue[0], '#') << "Lane " << i << " hue missing # prefix";
    }
}

TEST(BridgeContract, LaneRoleIsValidEnum) {
    auto j = serializeInitPreset();
    auto schema = loadSchema();
    auto validRoles = schema["definitions"]["lane"]["properties"]["role"]["enum"];

    for (size_t i = 0; i < j["state"]["lanes"].size(); ++i) {
        std::string role = j["state"]["lanes"][i]["role"];
        bool found = false;
        for (const auto& r : validRoles) {
            if (r == role) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Lane " << i << " has invalid role: " << role;
    }
}

TEST(BridgeContract, LaneSubdivisionIsValidEnum) {
    auto j = serializeInitPreset();
    auto schema = loadSchema();
    auto validSubs = schema["definitions"]["lane"]["properties"]["subdivision"]["enum"];

    for (size_t i = 0; i < j["state"]["lanes"].size(); ++i) {
        int sub = j["state"]["lanes"][i]["subdivision"];
        bool found = false;
        for (const auto& s : validSubs) {
            if (s == sub) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Lane " << i << " has invalid subdivision: " << sub;
    }
}

// Verify escapeJsonString handles special characters
TEST(BridgeContract, EscapeJsonStringSpecialChars) {
    EXPECT_EQ(poly::escapeJsonString("hello"), "\"hello\"");
    EXPECT_EQ(poly::escapeJsonString("a\"b"), "\"a\\\"b\"");
    EXPECT_EQ(poly::escapeJsonString("a\\b"), "\"a\\\\b\"");
    EXPECT_EQ(poly::escapeJsonString("a\nb"), "\"a\\nb\"");
    EXPECT_EQ(poly::escapeJsonString(""), "\"\"");
}

// ---------------------------------------------------------------------------
// T04: Action Payload Roundtrip Tests (choc JSON parser → value extraction)
// ---------------------------------------------------------------------------
// These verify that JSON payloads produced by JavaScript's JSON.stringify
// round-trip correctly through choc's parser using the same accessor pattern
// as handleAction(). Catches the bug class where choc stores integers as
// int64 but C++ extraction used strict type accessors that throw on mismatch.

TEST(BridgeActionContract, ChocInt64ToInt32Conversion) {
    auto v = choc::json::parse(R"({"x": 42})");
    EXPECT_EQ(v["x"].get<int32_t>(), 42);
}

TEST(BridgeActionContract, ChocInt64ToDoubleConversion) {
    auto v = choc::json::parse(R"({"x": 0})");
    EXPECT_DOUBLE_EQ(v["x"].get<double>(), 0.0);
}

TEST(BridgeActionContract, ToggleStepPayload) {
    auto msg = choc::json::parse(R"({"type":"action","v":1,"name":"toggleStep","payload":{"lane":2,"step":7}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["lane"].get<int32_t>(), 2);
    EXPECT_EQ(p["step"].get<int32_t>(), 7);
}

TEST(BridgeActionContract, SetEuclidPayloadAllFields) {
    auto msg = choc::json::parse(
        R"({"type":"action","v":1,"name":"setEuclid","payload":{"lane":1,"steps":8,"hits":5,"rotation":2}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["lane"].get<int32_t>(), 1);
    EXPECT_EQ(p["steps"].get<int32_t>(), 8);
    EXPECT_EQ(p["hits"].get<int32_t>(), 5);
    EXPECT_EQ(p["rotation"].get<int32_t>(), 2);
}

TEST(BridgeActionContract, SetEuclidPayloadPartial) {
    auto msg = choc::json::parse(R"({"type":"action","v":1,"name":"setEuclid","payload":{"lane":0,"steps":16}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["lane"].get<int32_t>(), 0);
    EXPECT_TRUE(p.hasObjectMember("steps"));
    EXPECT_FALSE(p.hasObjectMember("hits"));
    EXPECT_FALSE(p.hasObjectMember("rotation"));
    EXPECT_EQ(p["steps"].get<int32_t>(), 16);
}

TEST(BridgeActionContract, SetCellsPayloadWithArray) {
    auto msg = choc::json::parse(R"({"type":"action","v":1,"name":"setCells","payload":{"lane":3,"cells":[2,3,2]}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["lane"].get<int32_t>(), 3);
    EXPECT_TRUE(p.hasObjectMember("cells"));
    EXPECT_FALSE(p["cells"].isVoid());
    EXPECT_EQ(static_cast<int>(p["cells"].size()), 3);
    EXPECT_EQ(p["cells"][static_cast<uint32_t>(0)].get<int32_t>(), 2);
    EXPECT_EQ(p["cells"][static_cast<uint32_t>(1)].get<int32_t>(), 3);
    EXPECT_EQ(p["cells"][static_cast<uint32_t>(2)].get<int32_t>(), 2);
}

TEST(BridgeActionContract, SetCellsPayloadNull) {
    auto msg = choc::json::parse(R"({"type":"action","v":1,"name":"setCells","payload":{"lane":0,"cells":null}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["lane"].get<int32_t>(), 0);
    EXPECT_TRUE(p["cells"].isVoid());
}

TEST(BridgeActionContract, SetFixedStepPayload) {
    auto msg =
        choc::json::parse(R"({"type":"action","v":1,"name":"setFixedStep","payload":{"lane":1,"step":3,"on":true}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["lane"].get<int32_t>(), 1);
    EXPECT_EQ(p["step"].get<int32_t>(), 3);
    EXPECT_TRUE(p["on"].getBool());
}

TEST(BridgeActionContract, SetMicroTimingPayload) {
    auto msg =
        choc::json::parse(R"({"type":"action","v":1,"name":"setMicroTiming","payload":{"lane":0,"step":5,"ms":-10}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["lane"].get<int32_t>(), 0);
    EXPECT_EQ(p["step"].get<int32_t>(), 5);
    EXPECT_DOUBLE_EQ(p["ms"].get<double>(), -10.0);
}

TEST(BridgeActionContract, SetEnvelopePayload) {
    auto msg = choc::json::parse(
        R"({"type":"action","v":1,"name":"setEnvelope","payload":{"lane":2,"index":0,"envelope":{"target":"Velocity","period":4.0,"depth":0.3,"on":true}}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["lane"].get<int32_t>(), 2);
    EXPECT_EQ(p["index"].get<int32_t>(), 0);
    auto env = p["envelope"];
    EXPECT_FALSE(env.isVoid());
    EXPECT_EQ(std::string(env["target"].toString()), "Velocity");
    EXPECT_DOUBLE_EQ(env["period"].get<double>(), 4.0);
    EXPECT_DOUBLE_EQ(env["depth"].get<double>(), 0.3);
    EXPECT_TRUE(env["on"].getBool());
}

TEST(BridgeActionContract, SetEnvelopePayloadNull) {
    auto msg = choc::json::parse(
        R"({"type":"action","v":1,"name":"setEnvelope","payload":{"lane":1,"index":0,"envelope":null}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["lane"].get<int32_t>(), 1);
    EXPECT_EQ(p["index"].get<int32_t>(), 0);
    EXPECT_TRUE(p["envelope"].isVoid());
}

TEST(BridgeActionContract, SelectScenePayload) {
    auto msg = choc::json::parse(R"({"type":"action","v":1,"name":"selectScene","payload":{"scene":"B"}})");
    EXPECT_EQ(std::string(msg["payload"]["scene"].toString()), "B");
}

TEST(BridgeActionContract, ApplyPresetPayload) {
    auto msg = choc::json::parse(R"({"type":"action","v":1,"name":"applyPreset","payload":{"index":3}})");
    EXPECT_EQ(msg["payload"]["index"].get<int32_t>(), 3);
}

TEST(BridgeActionContract, ApplyPresetPayloadInit) {
    auto msg = choc::json::parse(R"({"type":"action","v":1,"name":"applyPreset","payload":{"index":-1}})");
    EXPECT_EQ(msg["payload"]["index"].get<int32_t>(), -1);
}

TEST(BridgeActionContract, ChainRemoveEntryPayload) {
    auto msg = choc::json::parse(R"({"type":"action","v":1,"name":"chainRemoveEntry","payload":{"index":2}})");
    EXPECT_EQ(msg["payload"]["index"].get<int32_t>(), 2);
}

TEST(BridgeActionContract, SetNoteMapPayload) {
    auto msg = choc::json::parse(R"({"type":"action","v":1,"name":"setNoteMap","payload":{"note":60,"output":62}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["note"].get<int32_t>(), 60);
    EXPECT_EQ(p["output"].get<int32_t>(), 62);
}

TEST(BridgeActionContract, SetAccentPayload) {
    auto msg =
        choc::json::parse(R"({"type":"action","v":1,"name":"setAccent","payload":{"lane":0,"step":3,"value":1}})");
    auto p = msg["payload"];
    EXPECT_EQ(p["lane"].get<int32_t>(), 0);
    EXPECT_EQ(p["step"].get<int32_t>(), 3);
    EXPECT_DOUBLE_EQ(p["value"].get<double>(), 1.0);
}

TEST(BridgeActionContract, EmptyPayloadActions) {
    for (const char* action : {"togglePlay", "exportRequest", "chainAddEntry", "resetNoteMap"}) {
        std::string j = std::string(R"({"type":"action","v":1,"name":")") + action + R"(","payload":{}})";
        auto msg = choc::json::parse(j);
        EXPECT_EQ(std::string(msg["type"].toString()), "action");
        EXPECT_EQ(std::string(msg["name"].toString()), action);
    }
}

TEST(BridgeActionContract, AllSchemaActionsHaveCppHandler) {
    auto schema = loadSchema();
    auto actionNames = schema["definitions"]["msgAction"]["properties"]["name"]["enum"];

    std::set<std::string> cppHandlers = {
        "toggleStep",  "setEuclid",     "setCells",  "setFixedStep",  "setMicroTiming",   "setEnvelope",  "selectScene",
        "applyPreset", "exportRequest", "setAccent", "chainAddEntry", "chainRemoveEntry", "resetNoteMap", "setNoteMap",
    };
    std::set<std::string> mockOnly = {"togglePlay"};

    for (const auto& name : actionNames) {
        std::string n = name.get<std::string>();
        if (mockOnly.count(n))
            continue;
        EXPECT_TRUE(cppHandlers.count(n) > 0) << "Schema action '" << n << "' has no C++ handler in handleAction()";
    }
}
