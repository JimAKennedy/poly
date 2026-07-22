---
class: gated
---

# MIDI Note Mapping Reference

All Poly presets use General MIDI drum map assignments, compatible with Cubase Drum Maps, Native Instruments Battery/Kontakt, and any GM-compliant kit.

## Note Inventory

Note names use the Cubase default convention: Middle C (MIDI 60) = C3, MIDI 0 = C-2.

| Note | Name | GM Standard Name   | Used By                                                                 |
|------|------|--------------------|-------------------------------------------------------------------------|
| 36   | C1   | Bass Drum 1        | Kick in most presets, Surdo (Bossa), Gong (Kotekan), Mridangam bass (Carnatic) |
| 37   | C#1  | Side Stick         | Rim (Poly Drift, Sparse Pulse, Balkan)                                  |
| 38   | D1   | Acoustic Snare     | Snare in standard presets                                               |
| 39   | D#1  | Hand Clap          | Ghost (Sparse Pulse, Pocket Groove), Kanjira (Carnatic), Init lane 5   |
| 42   | F#1  | Closed Hi-Hat      | Hi-hat in standard presets, Ghatam (Carnatic)                           |
| 43   | G1   | High Floor Tom     | Djembe (Afro-House), Darbuka (Balkan), Mridangam treble (Carnatic), Init lane 6 |
| 45   | A1   | Low Tom            | Tom/Perc ghost (Poly Drift, Breakbeat, IDM), Init lane 3               |
| 46   | A#1  | Open Hi-Hat        | Perc (Four on Floor), Init lane 4                                       |
| 50   | D2   | High Tom           | Init lane 7                                                            |
| 56   | G#2  | Cowbell            | Bell (Afrobeat), Cowbell (Latin), Perc (Afro-House), Glitch (IDM)      |
| 63   | D#3  | Open Hi Conga      | Conga (Latin, Afro-House, Afrobeat)                                     |
| 67   | G3   | High Agogo         | Agogo (Bossa Nova)                                                      |
| 70   | A#3  | Maracas            | Shaker (Afro-House, Afrobeat), Pandeiro (Bossa)                         |
| 75   | D#4  | Claves             | Clave (Latin)                                                           |
| 76   | E4   | Hi Wood Block      | Reich fixed/drifting, Polos (Kotekan), Zurna (Balkan), Tamborim (Bossa) |
| 77   | F4   | Low Wood Block     | Sangsih (Kotekan)                                                       |

## Init Preset Lane Assignments

| Lane | Note | Name | GM Name         |
|------|------|------|-----------------|
| 0    | 36   | C1   | Bass Drum 1     |
| 1    | 38   | D1   | Acoustic Snare  |
| 2    | 42   | F#1  | Closed Hi-Hat   |
| 3    | 45   | A1   | Low Tom         |
| 4    | 46   | A#1  | Open Hi-Hat     |
| 5    | 39   | D#1  | Hand Clap       |
| 6    | 43   | G1   | High Floor Tom  |
| 7    | 50   | D2   | High Tom        |

## Design Notes

Non-Western instruments are mapped to the closest GM equivalent by role:

- **Kanjira (39 / Hand Clap)** -- 54 (Tambourine) would be slightly more accurate for a frame drum, but hand clap works as a snappy ghost layer.
- **Pandeiro (70 / Maracas)** -- 54 (Tambourine) is arguably closer, but maracas gives the right shaker-shimmer role.
- **Ghatam (42 / Closed Hi-Hat)** -- no GM equivalent exists; hi-hat fills the rhythmic shimmer role well.
- **Afrobeat bell (56 / Cowbell)** -- ride bell (53) is an alternative for timeline bells, but cowbell is valid and more universally mapped.
