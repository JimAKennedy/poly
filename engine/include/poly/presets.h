#pragma once

#include "poly/types.h"

namespace poly {

struct PresetInfo {
    const char* name;
    const char* description;
    const char* category;
};

static constexpr int kFactoryPresetCount = 43;
static constexpr int kFactoryPresetCategoryCount = 10;

// Ordered category list — presentation order for plugin submenus and site chips.
// Every PresetInfo::category MUST match one of these strings exactly.
extern const char* const kFactoryPresetCategories[kFactoryPresetCategoryCount];

GrooveState makeFourOnTheFloor();
GrooveState makePolymetricDrift();
GrooveState makeSparsePulse();
GrooveState makeBreakbeat();
GrooveState makeLatinFeel();
GrooveState makeAfroHousePhrases();
GrooveState makeReichPhasing();
GrooveState makeKotekanInterlock();
GrooveState makePocketGroove();
GrooveState makeAfrobeat12_8();
GrooveState makeBalkanAksak();
GrooveState makeBossaNova();
GrooveState makeCarnaticTala();
GrooveState makeIDMGlitch();
GrooveState makeEweAgbekor();
GrooveState makeGamelanColotomic();
GrooveState makePolymetricFoundation();
GrooveState makeEweEnsemble();
GrooveState makeMandingDjembe();
GrooveState makeCubanSon();
GrooveState makeAfrobeatLagos();
GrooveState makeBaliKotekan();
GrooveState makeJavaColotomic();
GrooveState makeTintal();
GrooveState makeRupakTal();
GrooveState makeRachenitsa();
GrooveState makeKopanitsa();
GrooveState makeReichProcess();
GrooveState makeRileyLayers();
GrooveState makeNancarrowTempi();
GrooveState makeMinimalTechno();
GrooveState makeDeepHouse();
GrooveState makeSambaBatucada();
GrooveState makeBossaTrio();
GrooveState makeClassicFunk();
GrooveState makeNeoSoul();
GrooveState makeJazzBop();
GrooveState makeElvinCascade();
GrooveState makeJungleBreak();
GrooveState makeLiquidDnB();
GrooveState makeAfroElectronic();
GrooveState makeBalkanFunk();
GrooveState makeCompositionalArc();

GrooveState makeFactoryPreset(int index);
const PresetInfo& getFactoryPresetInfo(int index);

} // namespace poly
