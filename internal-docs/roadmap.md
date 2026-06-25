# Polymetric Groove Generator for Cubase (VST3)

## Overview

This document outlines a product vision and early-stage roadmap for a Cubase-focused polymetric groove generator plugin implemented as a VST3 instrument with MIDI generation capabilities. The product is conceived as a rhythm design engine rather than a static pattern browser: it should generate evolving, editable, musically legible grooves built from multiple independent rhythmic lanes, each with its own cycle length, subdivision logic, accent structure, and dynamic contour.[cite:16][cite:18][cite:19]

The near-term product scope assumes Cubase as the only supported host and VST3 as the plugin format. That assumption meaningfully simplifies architecture and workflow planning, because Cubase already supports VST3 instruments and offers established routing and recording workflows for MIDI output from instrument-style plugins.[cite:42][cite:44][cite:49]

The initial product should support 4 to 8 lanes, each capable of independent cycle and subdivision behavior, with explicit velocity control, emphasis/accent design, and superimposed envelopes that can evolve dynamics and activity across multiple timescales. These envelope layers are central to the product vision because they make repeated patterns feel alive, directional, and compositionally useful rather than mechanically looped.[cite:18][cite:19]

## Musical Background

### Why polyrhythm matters

Polyrhythm and polymeter are powerful compositional tools because they create perceptual motion from repetition. Instead of relying on constant note-level novelty, a groove can feel complex and evolving when multiple repeating patterns cycle against one another over a common pulse. In many traditions, the listener perceives both a stable time base and a shifting field of accents, densities, and implied groupings.[cite:7][cite:9]

At a practical level, a simple 4-beat pulse can coexist with a 3-step accent cycle, a 5-step percussion loop, and a 7-step velocity contour. None of those layers is especially complex in isolation, but together they generate a long composite cycle that reveals new alignments over time. This principle is one of the most important theoretical foundations for the plugin.[cite:7][cite:10]

### Cross-cultural rhythmic precedents

Many African and Afro-diasporic musics organize rhythmic complexity through interlocking parts around a shared pulse or timeline pattern. Afrobeat, for example, combines West African rhythmic structures with funk and jazz, often using multiple intersecting patterns that are individually simple but collectively dense and highly kinetic.[cite:9]

Latin American traditions frequently layer duple and triple groupings, using sesquialtera or hemiola-like relationships to create tension between stable meter and shifting emphasis. These practices are especially useful for understanding how a modern groove generator can preserve dance-floor clarity while still producing rhythmic ambiguity and forward motion.[cite:3]

More broadly, research on cross-cultural rhythm perception shows that listeners from different rhythmic cultures process complex metric structures differently, which reinforces an important product design insight: the plugin should not assume there is a single universal model of “natural groove.” Instead, it should expose controllable rhythmic logics and accent behaviors that can be tuned to different stylistic expectations.[cite:11]

### Interlocking layers and composite rhythm

A recurring principle across these traditions is that complexity often emerges from distribution rather than density. One part marks pulse, another supplies offbeats, another articulates cross-accents, and another adds ornament or response. The resulting “composite rhythm” is heard as a larger whole than any individual lane.[cite:3][cite:9]

For product design, this suggests that lanes should be role-oriented rather than simply “track 1, track 2, track 3.” Example lane identities might include anchor pulse, backbeat, subdivision shimmer, rotating accent, ghost detail, and long-cycle ornament. Giving each lane a conceptual function will make the generator easier to control and easier to explain in the UI.[cite:18][cite:19]

### Polyrhythm versus polymeter

The plugin should distinguish between polyrhythm and polymeter even if users experience them through similar controls. Polyrhythm refers to simultaneous contrasting subdivisions or groupings over a shared meter, such as 3 against 4. Polymeter refers to layers implying different bar lengths or measure structures while still sharing a common clock.[cite:7]

In implementation terms, the difference matters. A 3-over-4 hi-hat pattern over a standard 4/4 kick can be handled as a subdivision/accent problem, while a 5-step percussion lane cycling across a 16-step bar is better modeled as an independent lane cycle with periodic phase realignment. The product should support both, because the musical payoff comes from allowing short-cycle and long-cycle tension to coexist.[cite:7][cite:18]

### Dynamics as rhythm

Velocity is not merely an expressive afterthought; in groove-based music it is a structural component of rhythm. A repeated note pattern with varying velocity often reads as a different groove than the same pattern played at uniform intensity. Accent placement, ghost-note contrast, and dynamic swells can all redefine the listener’s sense of hierarchy within a bar or phrase.[cite:6][cite:10]

That is why velocity control should be first-class in the product. Users should be able to define per-lane base velocity, emphasis probability, accent masks, and dynamic envelopes. A lane should not just answer “when does a note occur?” but also “how strongly is it articulated, how does that strength change over time, and how does it interact with other lanes?”[cite:18][cite:19]

### Multiphase interest through superimposed envelopes

One of the most important differentiators for the plugin should be its ability to superimpose multiple envelopes on top of a lane or on the groove globally. These envelopes should modulate parameters such as velocity, density, probability, accent bias, microtiming looseness, and timbral role selection over different phase lengths.[cite:10][cite:12][cite:18]

For example, a lane could have:

- A short 1-bar velocity envelope shaping local contour.
- A 4-bar emphasis envelope that strengthens every third subdivision over time.
- A 7-bar activity envelope that gradually introduces and removes ghost notes.
- A 16-bar macro envelope that increases tension toward a section boundary.

Because these envelopes do not have to share the same period, they create multiphase interest: the groove changes in ways that feel organic and emergent, even when the underlying note-generation rules remain relatively constrained. This is the product feature that best translates ethnomusicological layering principles into a modern DAW tool.[cite:7][cite:10][cite:12]

## Product Vision

### Vision statement

The plugin should become a Cubase-native groove design instrument for producers who want structured rhythmic complexity, not just random variation. Its core value is the generation of musically coherent, evolving rhythmic layers that remain editable, explainable, and tightly integrated with composition workflows.[cite:18][cite:19]

Rather than replacing the producer, the product should act as a controllable rhythmic engine. It should help users sketch, mutate, and arrange polymetric grooves that would be time-consuming to program manually, while still preserving fine-grained authorial control over timing, density, accent, and dynamic behavior.[cite:17][cite:18]

### Product position

The most defensible market position is not “AI drum generator” in the generic sense. The more precise position is a polymetric and multi-timescale groove generator that combines explicit musical logic, lane-based architecture, and optional adaptive features later in the roadmap.[cite:19][cite:20]

That framing matters because many current tools already offer pattern libraries, Euclidean sequencing, or basic probability. The product becomes more distinctive when it emphasizes independent lane cycles, composite rhythm design, velocity/emphasis modeling, and envelope superposition for long-form motion.[cite:18][cite:19][cite:20]

### Intended users

The initial target users are producers working in Cubase who want to generate drum and percussion material for electronic, hybrid, and groove-oriented music. The product is especially well suited to users who care about evolving arrangement behavior, interlocking parts, and the ability to render or record generated MIDI for downstream editing.[cite:39][cite:40][cite:49]

Secondary users include composers and sound designers who want to drive percussion instruments, sampled ensembles, or modular rhythmic textures from a lane-based generator rather than a fixed grid editor. Because Cubase is the initial target, the plugin should feel at home inside Cubase’s instrument-track, routing, automation, and MIDI capture model.[cite:40][cite:45][cite:49]

## Product Principles

The early product should follow a small set of principles:

- **Musical legibility:** every generated result should be understandable in terms of lanes, cycles, accents, and envelopes rather than opaque black-box output.[cite:17][cite:19]
- **Editable output:** users must be able to capture, drag, or otherwise render MIDI into ordinary Cubase workflows for further editing.[cite:39][cite:40]
- **Deterministic behavior:** transport restarts, loop points, and tempo changes must not produce unstable or surprising results unless variation is explicitly requested.[cite:24][cite:29]
- **Dynamics-first design:** velocity and emphasis are part of the composition model, not cosmetic modifiers.[cite:18][cite:19]
- **Long-form motion:** the product should produce phrase- and section-level development, not just bar-level pattern mutation.[cite:18][cite:19]
- **Host focus:** optimize for Cubase instead of abstract cross-host support in the early phases.[cite:42][cite:44]

## Cubase and VST3 Assumptions

The initial architecture assumes a VST3 plugin intended for Cubase only. Cubase supports VST3 instruments and effects, and Steinberg positions VST3 as the modern plugin standard in Cubase and Nuendo.[cite:42][cite:44]

Because VST3 does not provide a universally consistent “MIDI effect” model across hosts, the most practical product shape is a VST3 instrument with MIDI generation/output behavior, designed around Cubase routing conventions. This avoids spending early roadmap capacity on cross-host compatibility abstraction and allows the team to optimize one canonical user workflow.[cite:30][cite:39][cite:43]

A working v1 user path should be:

1. Load the groove generator on an instrument track in Cubase.[cite:49]
2. Route or capture its MIDI output into the target drum instrument or MIDI part workflow supported by Cubase.[cite:39][cite:40]
3. Shape the groove using lane controls, macro controls, velocity/emphasis design, and envelope layers.
4. Record, freeze, or export MIDI for downstream editing and arrangement.[cite:40]

## Core Product Model

### Lane architecture

The generator should expose 4 to 8 lanes. This range is large enough to support rich interlocking textures while remaining cognitively manageable for v1. Fewer than four lanes reduces the musical payoff of composite rhythm; more than eight lanes risks UI overload before the interaction model is proven.[cite:18][cite:19]

Each lane should own the following properties:

- Role or target voice.
- Cycle length.
- Subdivision grid.
- Hit or pulse count.
- Rotation/offset.
- Probability.
- Base velocity.
- Accent/emphasis map.
- Humanization or timing looseness amount.
- Envelope assignments.

This design makes each lane a self-contained rhythmic agent while still allowing global macros and inter-lane relationships.[cite:18][cite:19]

### Velocity, emphasis, and accent structure

Velocity should be modeled at several levels rather than as a single knob:

- **Base velocity:** nominal intensity of the lane.
- **Accent mask:** defines stronger beat positions or selected steps.
- **Emphasis probability:** determines how often strong accents are actually expressed.
- **Ghost-note floor:** minimum low-velocity articulations for supporting texture.
- **Velocity spread:** controlled randomization or contour deviation.
- **Macro envelope modulation:** phrase-scale changes in intensity over time.

This allows the system to produce grooves that “breathe.” For example, a lane may preserve the same onset pattern while the emphasis envelope shifts the listener’s perceived phrasing every few bars. That is especially valuable in electronic and hybrid percussion contexts where repeated MIDI note positions alone are not enough to sustain interest.[cite:6][cite:10][cite:18]

### Superimposed envelopes

Envelope superposition should be one of the signature features in the product vision. Instead of giving each lane one static modulation curve, the plugin should support multiple envelopes per lane and optionally a small number of global envelopes affecting all lanes or selected lane groups.[cite:10][cite:12][cite:18]

Potential envelope targets include:

- Velocity.
- Density.
- Probability.
- Accent bias.
- Note length.
- Timing looseness.
- Lane mute/activation weight.
- Fill likelihood.

Potential envelope timescales include one bar, multiple bars, phrase lengths, and custom lane-relative cycle lengths. The key capability is that these envelopes can be superimposed rather than replaced, producing multi-timescale modulation and phase interaction. This makes the groove feel composed and evolving rather than periodically “randomized.”[cite:10][cite:12][cite:18]

### Macro controls

The plugin should also expose a small set of high-value macro controls. These should alter multiple underlying parameters coherently rather than simply scaling a single value.

Recommended early macros:

- Complexity.
- Density.
- Syncopation.
- Swing.
- Tension.
- Humanize.
- Repetition versus variation.
- Tight versus loose feel.

These macros are important because they let users shape the generator at musical intent level while preserving access to lane-level editing for expert control.[cite:18][cite:19]

## Early-Stage Roadmap

The roadmap below focuses on the initial phases only: architecture, MVP engine, and early workflow/musical refinement.

### Phase 0: Product Definition and Architecture

The goal of Phase 0 is to lock the Cubase-first operating model and de-risk the core engine architecture before deep feature work begins. Since Cubase-only support is now assumed, this phase should not spend effort on broad DAW compatibility. Instead, it should define one canonical Cubase routing and recording workflow and make that the contract for the product.[cite:39][cite:40][cite:44]

#### Objectives

- Finalize the plugin form as a VST3 instrument with MIDI generation/output behavior for Cubase workflows.[cite:30][cite:43]
- Define the lane engine data model and event scheduling model.
- Decide how cycle math, bar alignment, and phrase alignment are represented internally.
- Define the initial UI model for 4 to 8 lanes without overloading the screen.
- Establish the preset/state model for generator intent, snapshots, and rendered output.

#### Key technical decisions

- **Clock model:** use transport-anchored musical time rather than free-running time, with explicit handling for loop restarts and song-position jumps.[cite:24][cite:29]
- **Event scheduler:** compute MIDI events predictably for the current processing window using deterministic cycle state.
- **Lane state model:** separate static lane settings from evolving envelope phase state.
- **Snapshot model:** allow storing a full groove state, including envelope phase settings where appropriate, for pattern recall and A/B testing.
- **Automation contract:** decide which parameters are exposed to Cubase automation in v1 versus which remain internal compound controls.[cite:44][cite:45]

#### Deliverables

- Product requirements document.
- Technical architecture note.
- Cubase workflow spec.
- UI wireframes for lane editor, macro section, and envelope editor.
- Prototype of timing engine behavior in a test harness.

### Phase 1: Core Groove Engine MVP

The goal of Phase 1 is to deliver a musically useful generator before adaptive or ML features are introduced. The MVP should prove that the lane architecture, cycle interaction, and velocity/emphasis system are already valuable on their own.[cite:17][cite:18]

#### MVP scope

- Support 4 to 8 lanes.
- Independent cycle length per lane.
- Independent subdivision grid per lane.
- Pulse count or hit count controls.
- Rotation or offset controls.
- Per-lane probability.
- Per-lane base velocity.
- Accent/emphasis masks.
- Velocity spread and ghost-note behavior.
- A first version of superimposed envelopes, at minimum for velocity and density.
- Core macro controls for complexity, density, syncopation, swing, and tension.
- MIDI generation that can be captured or recorded in Cubase workflows.[cite:39][cite:40]

#### MVP user outcomes

The user should be able to instantiate the plugin, set up four or more interlocking lanes, hear evolving composite grooves immediately, and render or record the result into ordinary MIDI for editing. The plugin should already make static beat programming feel slower by comparison.[cite:39][cite:40][cite:49]

#### MVP quality criteria

- Stable under transport start/stop.
- Stable under loop playback.
- Stable under tempo changes.
- Repeatable output when randomness is disabled.
- Clear visual correspondence between lane settings and audible output.
- Velocity/emphasis changes are musically audible and not buried in the UI.

### Phase 2: Musical Refinement and Cubase Workflow Polish

The goal of Phase 2 is to turn the MVP from a compelling generator into a composition tool. This phase should improve phrase-scale motion, recall, and Cubase workflow polish rather than jump immediately to machine learning.[cite:18][cite:19]

#### Scope

- More sophisticated envelope layering, including multiple simultaneous envelopes per lane.
- Phrase- and section-length modulation lengths such as 4, 8, 16, and custom bars.[cite:18]
- Scene or snapshot switching for moving between groove states.[cite:17][cite:18]
- Constraint options such as anchor kick, preserve backbeat, and protect lane density ranges.[cite:19]
- Better automation mapping for high-value Cubase controls.[cite:45]
- Freeze, drag, or pattern export workflows where feasible in the Cubase-centered interaction model.[cite:40]
- Improved visualizations for phase relationship and envelope interaction.

#### Desired outcomes

By the end of this phase, the product should support writing an entire rhythmic section or draft track, not just generating interesting loops. The user should be able to evolve grooves over time while keeping core structural anchors intact.[cite:18][cite:19]

## Suggested Functional Breakdown

| Area | Phase 0 | Phase 1 | Phase 2 |
|---|---|---|---|
| Cubase workflow model | Define | Implement | Polish |
| VST3 instrument packaging | Define | Implement | Stabilize |
| Lane engine | Design | Implement | Optimize |
| 4–8 lane UI | Wireframe | Implement | Refine |
| Velocity/emphasis engine | Design | Implement | Expand |
| Envelope superposition | Design | Initial implementation | Advanced layering |
| Macro controls | Define | Implement | Tune |
| MIDI capture/export workflow | Define | Basic support | Better UX |
| Scene/snapshot system | Define | Basic support if feasible | Expand |
| Adaptive/ML features | Deferred | Deferred | Still deferred |

## Risks and Design Considerations

### UI complexity

A lane-based polymetric tool can become visually dense very quickly. Supporting up to eight lanes is musically attractive, but the UI must avoid presenting every parameter at once. The interaction model should likely separate overview, focused lane editing, and envelope editing into distinct panes or modes.[cite:18][cite:19]

### Determinism versus surprise

Users want variation, but they also need trust. If the plugin changes too much between playback passes, it becomes hard to compose with. The roadmap should therefore bias toward deterministic systems with explicit randomness controls, seed management, and repeatability toggles.[cite:17][cite:18]

### Envelope design usability

Superimposed envelopes are potentially a signature differentiator, but they also introduce conceptual complexity. The product should expose them progressively: perhaps one visible “motion” layer in the simplest workflow, with optional advanced envelope stacks for expert users. This is a product design problem as much as an algorithmic one.[cite:18][cite:19]

### Cubase workflow friction

Even with Cubase-only targeting, MIDI-generation workflows need to feel straightforward. Routing, monitoring, and recording generated output must be documented and validated carefully so the product does not feel fragile or “special-case.” Steinberg and user-community material indicate that Cubase users can record MIDI output from VST instruments, but the exact interaction flow should be made part of the product onboarding and QA plan.[cite:40][cite:49]

## Implementation Notes for Early Engineering

The early implementation should treat the groove engine as a pure musical model separate from UI and separate from any later adaptive intelligence. That engine should accept transport position and lane state as inputs and emit deterministic note events plus dynamic values as outputs. This separation will make the system easier to test and easier to evolve.[cite:24][cite:29][cite:18]

A useful internal decomposition is:

- **Transport/time layer:** converts host timing into bar, beat, subdivision, and lane-phase context.
- **Lane generator layer:** computes hit candidates for each lane.
- **Dynamic shaping layer:** applies velocity, emphasis masks, ghost-note floors, and envelope superposition.
- **Constraint layer:** enforces product-level rules such as anchor beats or maximum density.
- **Output layer:** schedules MIDI events for the host and optionally prepares a renderable clip representation.

This architecture will also help when later adaptive features are introduced, because learning systems can propose parameter changes or target curves without owning the entire note-generation path.[cite:19][cite:20]

## Further Reading

### VST3 and Cubase

- Steinberg VST 3 SDK introduction and API documentation for plugin architecture, processing, and host integration.[cite:21][cite:24]
- Steinberg VST 3 Developer Portal for current documentation entry points.[cite:31]
- Steinberg technical background on VST3 in Cubase and Nuendo.[cite:44]
- Steinberg Cubase help pages on installing and using VST plugins and instruments.[cite:42][cite:49]

### JUCE and plugin development

- JUCE platform documentation and framework overview for cross-platform plugin development with VST3 targets.[cite:32]
- JUCE community discussions around VST3, MIDI-output plugin behavior, and host workflow expectations.[cite:28][cite:29][cite:30][cite:39][cite:43]

### Rhythm and layering

- Material on musical rhythm concepts, especially distinctions between polyrhythm and polymeter.[cite:7]
- Discussions of Latin American rhythmic layering and sesquialtera/hemiola traditions.[cite:3]
- Overviews of Afrobeat and related African-derived approaches to intersecting rhythmic layers.[cite:9]
- Research on cross-cultural rhythm processing and perception.[cite:11]
- Writing on vertical layering and texture-building as soundscape practice.[cite:10][cite:12]

## Immediate Next Steps

The most effective next deliverables for implementation are:

1. A formal product requirements document derived from this vision.
2. A lane-engine technical specification covering timing, cycle math, velocity/emphasis logic, and envelope superposition.
3. A Cubase user workflow note showing exactly how the VST3 instrument is routed, monitored, and recorded.
4. A UI concept for 4-lane and 8-lane operation modes.
5. A small timing-engine prototype proving deterministic cycle alignment and envelope phase behavior under loop restarts.

These steps keep the project focused on its strongest early differentiators: multi-lane polymeter, dynamics-first groove generation, and long-form rhythmic motion through superimposed envelopes.[cite:18][cite:19]
