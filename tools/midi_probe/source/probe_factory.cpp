#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/main/pluginfactory.h"

#include "probe_ids.h"
#include "probe_processor.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

BEGIN_FACTORY_DEF("jk.digital", "https://jk.digital", "mailto:contact@jk.digital")

DEF_CLASS2(INLINE_UID_FROM_FUID(probe::kProbeProcessorUID), PClassInfo::kManyInstances, kVstAudioEffectClass,
           probe::kProbePluginName, 0, Vst::PlugType::kFxAnalyzer, probe::kProbeVersionString, kVstVersionString,
           probe::ProbeProcessor::createInstance)

END_FACTORY
