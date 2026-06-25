# Poly — UI Wireframes

## Design Philosophy

The UI must show 4–8 interlocking lanes without overwhelming the user. Three views handle this through progressive disclosure: the overview shows all lanes at once, the lane editor shows one lane in detail, and the envelope editor shows modulation across time.

VSTGUI `.uidesc` files define the layouts. The editor window targets 900×600 minimum, scaling up on larger displays.

---

## View 1: Lane Overview (Default)

The primary view. All lanes visible simultaneously with macro controls.

```
┌─────────────────────────────────────────────────────────────────────┐
│  POLY v0.1                                    [Preset ▾] [≡]      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─ Lane 1: Kick (AnchorPulse) ──────────────────────────────────┐ │
│  │ [●] C1  5/16  E(3,5)  rot:0  prob:0.90  vel:110  ▮▮▯▮▯       │ │
│  └────────────────────────────────────────────────────────────────┘ │
│  ┌─ Lane 2: Snare (Backbeat) ────────────────────────────────────┐ │
│  │ [●] D1  4/8   E(2,4)  rot:0  prob:1.00  vel:100  ▯▮▯▮       │ │
│  └────────────────────────────────────────────────────────────────┘ │
│  ┌─ Lane 3: HH (Shimmer) ───────────────────────────────────────┐ │
│  │ [●] F#1 7/16  E(5,7)  rot:2  prob:0.85  vel: 70  ▮▮▯▮▮▯▮   │ │
│  └────────────────────────────────────────────────────────────────┘ │
│  ┌─ Lane 4: Perc (Ghost) ───────────────────────────────────────┐ │
│  │ [●] A1  3/16  E(2,3)  rot:1  prob:0.60  vel: 45  ▮▯▮       │ │
│  └────────────────────────────────────────────────────────────────┘ │
│  ┌─ Lane 5 ──────────────────────────────────────────────────────┐ │
│  │ [○] —   off                                                    │ │
│  └────────────────────────────────────────────────────────────────┘ │
│                                                                     │
│  ┌─ Macros ──────────────────────────────────────────────────────┐ │
│  │  Complexity  Density  Syncopation  Swing  Tension  Humanize   │ │
│  │  ────●────  ───●────  ──●───────  ●─────  ──●────  ●──────   │ │
│  │     0.50       0.50      0.30      0.00     0.20     0.00     │ │
│  └────────────────────────────────────────────────────────────────┘ │
│                                                                     │
│  Seed: 0x1A2B3C4D    [Randomize]              [4 lanes ▾]          │
└─────────────────────────────────────────────────────────────────────┘
```

### Element Descriptions

| Element | Widget | Behavior |
|---------|--------|----------|
| Lane row | Clickable strip | Single click selects; double click opens lane editor |
| [●]/[○] | Toggle | Lane active/inactive |
| Note label | Text | MIDI note (GM drum name) |
| Cycle display | Text | `steps/subdivision` format |
| E(k,n) | Text | Euclidean notation: k hits in n steps |
| Pattern strip | Dot display | Visual pulse pattern (filled = hit) |
| Macro sliders | Horizontal slider | 0.0–1.0 with value readout |
| Seed | Hex display + button | Current seed; randomize generates new seed |
| Lane count | Dropdown | 4, 5, 6, 7, or 8 active lanes |

---

## View 2: Focused Lane Editor

Opened by double-clicking a lane row. Shows all parameters for one lane.

```
┌─────────────────────────────────────────────────────────────────────┐
│  POLY v0.1  ◄ Lane 3: HH (Shimmer)                  [← Overview] │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─ Identity ──────────┐  ┌─ Cycle ────────────────────────────┐  │
│  │ Role:  [Shimmer ▾]  │  │ Steps: [7 ▾]   Subdivision: [16 ▾]│  │
│  │ Note:  [F#1    ▾]   │  │ Hits:  [5 ▾]   Rotation:    [2 ▾] │  │
│  │ Active: [●]         │  │                                     │  │
│  └─────────────────────┘  │ Pattern: ▮ ▮ ▯ ▮ ▮ ▯ ▮             │  │
│                            └────────────────────────────────────┘  │
│                                                                     │
│  ┌─ Dynamics ──────────────────────────────────────────────────┐   │
│  │                                                              │   │
│  │  Probability     ─────────────●──  0.85                      │   │
│  │  Base Velocity   ──────────●─────  70                        │   │
│  │  Velocity Spread ──●─────────────  0.10                      │   │
│  │  Emphasis Prob   ───────●────────  0.50                      │   │
│  │  Ghost Floor     ───●────────────  30                        │   │
│  │  Humanize        ●──────────────   0.0 ms                    │   │
│  │                                                              │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  ┌─ Accent Mask ───────────────────────────────────────────────┐   │
│  │  Step:   1   2   3   4   5   6   7                           │   │
│  │  Accent: [●] [○] [○] [●] [○] [○] [●]                        │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  ┌─ Envelopes (0/4 active) ────────── [+ Add Envelope] ───────┐   │
│  │  (no envelopes assigned)                                     │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Element Descriptions

| Element | Widget | Behavior |
|---------|--------|----------|
| Role dropdown | Select | Sets lane semantic role |
| Note dropdown | Select | GM drum map with note names |
| Steps/Subdivision | Spinbox or dropdown | Defines cycle length |
| Hits/Rotation | Spinbox or dropdown | Euclidean parameters |
| Pattern display | Read-only dots | Updates live as hits/rotation change |
| Dynamic sliders | Horizontal sliders | Real-time automatable parameters |
| Accent mask | Toggle grid | Click steps to set accent positions |
| Envelope list | Expandable rows | Shows assigned envelopes; [+] adds new |

---

## View 3: Envelope Editor

Opened by clicking an envelope row in the lane editor, or via a dedicated tab.

```
┌─────────────────────────────────────────────────────────────────────┐
│  POLY v0.1  ◄ Lane 3: HH — Envelope 1                [← Lane]    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─ Settings ──────────────────────────────────────────────────┐   │
│  │  Target: [Velocity ▾]    Shape: [Sine ▾]                    │   │
│  │  Period: [4.0 ▾] bars    Depth: ───────●──── 0.80           │   │
│  │  Phase:  ──●──────────── 0.25                                │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  ┌─ Waveform Preview ─────────────────────────────────────────┐   │
│  │  1.0 ┤                                                      │   │
│  │      │      ╭──╮                         ╭──╮               │   │
│  │  0.5 ┤    ╭╯    ╰╮                     ╭╯    ╰╮             │   │
│  │      │  ╭╯        ╰╮                 ╭╯        ╰╮           │   │
│  │  0.0 ┤╶╯            ╰╮             ╭╯            ╰╮         │   │
│  │      │                ╰╮         ╭╯                ╰╮       │   │
│  │ -0.5 ┤                  ╰──╮ ╭──╯                    ╰──    │   │
│  │      ├───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┤     │   │
│  │      1       2       3       4       5       6       7  bars │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  ┌─ All Envelopes on This Lane ────────────────────────────────┐   │
│  │  [1] Velocity   Sine   4 bars   depth:0.80   phase:0.25    │   │
│  │  [+] Add envelope (3 slots remaining)                        │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  ┌─ Global Envelopes ─────────────────────────────────────────┐   │
│  │  (no global envelopes assigned)                              │   │
│  │  [+ Add Global Envelope] (8 slots available)                 │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Element Descriptions

| Element | Widget | Behavior |
|---------|--------|----------|
| Target | Dropdown | Velocity, Density, Probability, AccentBias, etc. |
| Shape | Dropdown | Sine, Ramp, Triangle, Curve, StepList |
| Period | Numeric with suffix | Bars (float, supports non-integer like 3.5, 7.0) |
| Depth | Slider | 0.0–1.0 modulation amount |
| Phase | Slider | 0.0–1.0 phase offset |
| Waveform preview | Canvas | Real-time rendering of shape × depth × phase |
| Envelope list | Rows | All envelopes on this lane with summary |
| Global envelopes | Rows | Envelopes affecting all lanes |

---

## Navigation Flow

```
Lane Overview ──(double-click lane)──► Focused Lane Editor
     ▲                                        │
     │                              (click envelope row)
     │                                        ▼
     └───────(← Overview button)───── Envelope Editor
                                              │
                              (← Lane button)─┘
```

## Responsive Layout Notes

- **Minimum size**: 900×600 for overview with 4 lanes + macros
- **8 lanes**: Lane rows compress vertically; pattern strip shortens
- **Lane editor**: Fixed height — scrolls if window is too short
- **Envelope waveform**: Scales horizontally with window width; minimum 400px
- **VSTGUI constraints**: All layouts defined in `.uidesc` XML; no runtime HTML/CSS

## Color and Typography (Deferred)

Color palette, font choices, and visual styling will be defined in Phase 1 when the VSTGUI editor is implemented. These wireframes define layout and information hierarchy only. See `~/dev/audio-meta/design/` for jk.digital brand tokens when styling begins.
