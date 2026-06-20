# SEED-MEMORIES.md — Poly Initial Memory Seeding

**Instructions for the agent:** Read each memory below and persist it using `capture_thought`.
After all memories are saved, delete this file (`rm SEED-MEMORIES.md`).

---

## Memory 1: RT Safety — No allocation in process()/renderRange()
- **Category:** `convention`
- **Content:** `poly_engine::renderRange() and the VST3 process() call must never allocate heap memory, acquire locks, throw exceptions, or perform I/O. Pre-allocate all buffers in initialize(). This is a portfolio-wide convention — violations found in M004 audit across DrumGenerator, DrumPostProcessor, MorphForge, and TremoloGlitch.`
- **Confidence:** 0.95

## Memory 2: State Serialization Versioning
- **Category:** `convention`
- **Content:** `Always write kStateVersion as the first int32 in getState(). Branch on version in setState(). Omitting a version number makes preset compatibility a time bomb — every new field requires another fallback layer. Learned from M004 audit of existing jk.digital plugins.`
- **Confidence:** 0.95

## Memory 3: VST3 allocateMessage/sendMessage Not RT-Safe
- **Category:** `gotcha`
- **Content:** `VST3 SDK allocateMessage()/sendMessage() in process() is NOT guaranteed lock-free. Found as a systematic violation in DrumPostProcessor and MorphForge during M004 audit. For Poly: if you need to send messages from the audio thread, use atomic flags or a lock-free queue to defer to a timer/non-RT thread.`
- **Confidence:** 0.95

## Memory 4: setActive() May Be Called From Audio Thread
- **Category:** `gotcha`
- **Content:** `Some DAW hosts (including certain Cubase configurations) call VST3 setActive() from the audio thread. Any allocation (vector::resize, new, malloc) in setActive() is a potential RT safety violation. Pre-allocate in initialize() and only clear/reset in setActive().`
- **Confidence:** 0.9

## Memory 5: Absolute PPQ Timing — No Accumulation
- **Category:** `architecture`
- **Content:** `Poly's envelope and cycle phases must be derived from absolute ProcessContext PPQ position (projectTimeMusic), never accumulated across process blocks. Accumulator-based sequencers drift on loop restarts, tempo changes, and transport jumps. Golden tests enforce byte-identical output under these conditions.`
- **Confidence:** 0.95

## Memory 6: C++20 Trial Project
- **Category:** `architecture`
- **Content:** `Poly is the first jk.digital project using C++20 (rest of portfolio is C++17). This is intentional — proving C++20 features (std::span, designated initializers, concepts) before wider adoption. If C++20 causes friction with VST3 SDK or CI toolchains, document the issues for the portfolio decision.`
- **Confidence:** 0.9

## Memory 7: Lock-Free Cross-Thread Communication Pattern
- **Category:** `pattern`
- **Content:** `Reference pattern from DrumGenerator: SPSC ring buffers (RTMessageQueue) for audio-to-UI, atomic flag ownership ping-pong for audio-to-generation, double buffering with atomic index swap for generation-to-audio. All use acquire/release memory ordering. Adapt for Poly's parameter snapshot delivery to the engine.`
- **Confidence:** 0.85
