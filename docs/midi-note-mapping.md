# MIDI Note Mapping Reference

All Poly presets use General MIDI drum map assignments, compatible with Cubase Drum Maps, Native Instruments Battery/Kontakt, and any GM-compliant kit.

## Note Inventory

| Note | GM Standard Name   | Used By                                                                 |
|------|--------------------|-------------------------------------------------------------------------|
| 36   | Bass Drum 1        | Kick in most presets, Surdo (Bossa), Gong (Kotekan), Mridangam bass (Carnatic) |
| 37   | Side Stick         | Rim (Poly Drift, Sparse Pulse, Balkan)                                  |
| 38   | Acoustic Snare     | Snare in standard presets                                               |
| 39   | Hand Clap          | Ghost (Sparse Pulse, Pocket Groove), Kanjira (Carnatic), Init lane 5   |
| 42   | Closed Hi-Hat      | Hi-hat in standard presets, Ghatam (Carnatic)                           |
| 43   | High Floor Tom     | Djembe (Afro-House), Darbuka (Balkan), Mridangam treble (Carnatic), Init lane 6 |
| 45   | Low Tom            | Tom/Perc ghost (Poly Drift, Breakbeat, IDM), Init lane 3               |
| 46   | Open Hi-Hat        | Perc (Four on Floor), Init lane 4                                       |
| 50   | High Tom           | Init lane 7                                                            |
| 56   | Cowbell            | Bell (Afrobeat), Cowbell (Latin), Perc (Afro-House), Glitch (IDM)      |
| 63   | Open Hi Conga      | Conga (Latin, Afro-House, Afrobeat)                                     |
| 67   | High Agogo         | Agogo (Bossa Nova)                                                      |
| 70   | Maracas            | Shaker (Afro-House, Afrobeat), Pandeiro (Bossa)                         |
| 75   | Claves             | Clave (Latin)                                                           |
| 76   | Hi Wood Block      | Reich fixed/drifting, Polos (Kotekan), Zurna (Balkan), Tamborim (Bossa) |
| 77   | Low Wood Block     | Sangsih (Kotekan)                                                       |

## Init Preset Lane Assignments

| Lane | Note | GM Name         |
|------|------|-----------------|
| 0    | 36   | Bass Drum 1     |
| 1    | 38   | Acoustic Snare  |
| 2    | 42   | Closed Hi-Hat   |
| 3    | 45   | Low Tom         |
| 4    | 46   | Open Hi-Hat     |
| 5    | 39   | Hand Clap       |
| 6    | 43   | High Floor Tom  |
| 7    | 50   | High Tom        |

## Design Notes

Non-Western instruments are mapped to the closest GM equivalent by role:

- **Kanjira (39 / Hand Clap)** -- 54 (Tambourine) would be slightly more accurate for a frame drum, but hand clap works as a snappy ghost layer.
- **Pandeiro (70 / Maracas)** -- 54 (Tambourine) is arguably closer, but maracas gives the right shaker-shimmer role.
- **Ghatam (42 / Closed Hi-Hat)** -- no GM equivalent exists; hi-hat fills the rhythmic shimmer role well.
- **Afrobeat bell (56 / Cowbell)** -- ride bell (53) is an alternative for timeline bells, but cowbell is valid and more universally mapped.
