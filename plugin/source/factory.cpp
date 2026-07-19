#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/main/pluginfactory.h"

#include "controller.h"
#include "plugids.h"
#include "processor.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

BEGIN_FACTORY_DEF("jk.digital", "https://jk.digital", "mailto:contact@jk.digital")

// Non-distributable: processor sends `&uiSnapshot_` as a raw pointer via IMessage
// (see PolyProcessor::sendSnapshotPointer). In a bridged/distributed host the pointer
// would be interpreted in the wrong address space. Revisit when moving to a proper
// cross-process UI surface (shared memory, id-based lookup, or message-only UI state).
// M046 S02 P3 mitigation — review 2026-07-16.
DEF_CLASS2(INLINE_UID_FROM_FUID(poly::kPolyProcessorUID), PClassInfo::kManyInstances, kVstAudioEffectClass,
           poly::kPolyPluginName, 0, Vst::PlugType::kInstrumentSynth, poly::kPolyVersionString, kVstVersionString,
           poly::PolyProcessor::createInstance)

DEF_CLASS2(INLINE_UID_FROM_FUID(poly::kPolyControllerUID), PClassInfo::kManyInstances, kVstComponentControllerClass,
           "Poly Controller", 0, "", poly::kPolyVersionString, kVstVersionString, poly::PolyController::createInstance)

END_FACTORY
