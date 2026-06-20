#include "public.sdk/source/main/pluginfactory.h"

#include "plugids.h"
#include "processor.h"
#include "controller.h"

#include "pluginterfaces/vst/ivstaudioprocessor.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

BEGIN_FACTORY_DEF("jk.digital",
                  "https://jk.digital",
                  "mailto:contact@jk.digital")

    DEF_CLASS2(INLINE_UID_FROM_FUID(poly::kPolyProcessorUID),
               PClassInfo::kManyInstances,
               kVstAudioEffectClass,
               poly::kPolyPluginName,
               Vst::kDistributable,
               Vst::PlugType::kInstrumentSynth,
               poly::kPolyVersionString,
               kVstVersionString,
               poly::PolyProcessor::createInstance)

    DEF_CLASS2(INLINE_UID_FROM_FUID(poly::kPolyControllerUID),
               PClassInfo::kManyInstances,
               kVstComponentControllerClass,
               "Poly Controller",
               0,
               "",
               poly::kPolyVersionString,
               kVstVersionString,
               poly::PolyController::createInstance)

END_FACTORY
