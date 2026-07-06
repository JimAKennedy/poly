#pragma once

#include "poly/types.h"

namespace poly {

struct PresetInfo {
    const char* name;
    const char* description;
};

static constexpr int kFactoryPresetCount = 16;

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

GrooveState makeFactoryPreset(int index);
const PresetInfo& getFactoryPresetInfo(int index);

} // namespace poly
