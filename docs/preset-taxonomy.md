# Poly Preset Taxonomy

The 43 factory presets are grouped into 10 categories. This grouping is the source of truth for both the plugin's nested `Category > Preset` menu (`plugin/source/ui/header_view.cpp`) and the site's Try It category filter row.

The category enum lives in the engine as `kFactoryPresetCategories[]` in `engine/src/presets.cpp`, and each `PresetInfo` entry in `kInfos[]` carries a `category` field that indexes into it. The JSON emitter (`engine/tools/emit_presets.cpp`) exports both the enum and each preset's category as `schemaVersion: 2`.

## Design constraints

- 8-10 categories total
- 2-8 presets per category (relaxed to 2-7 to fit Asian Traditions without an 11th category)
- Each preset in exactly one category
- Categories cover all 43 presets with no gaps

## Ordering rationale

Categories are ordered from foundational/reference material at the top down through Western dance styles, groove traditions, world traditions, and finally experimental / fusion. This is the order they should appear in both the plugin submenu and the site chip row so a user scanning left-to-right / top-to-bottom sees "learn the engine" before "play a style".

## The 10 categories

| # | Category | Count | What lives here |
|---|----------|-------|-----------------|
| 1 | Foundational | 3 | Reference / demo grooves that show the engine's polymetric primitives without leaning on a style. |
| 2 | Minimalist / Compositional | 5 | Process pieces (Reich phasing, Riley layered entry, Nancarrow tempi) plus the Compositional Arc build. |
| 3 | House / Techno | 4 | Straight-four dance grooves — classic house, minimal techno, deep house, afro-house. |
| 4 | Jazz / Funk / Soul | 5 | US-tradition swing and pocket grooves — bop ride, Elvin cascade, JB funk, neo-soul, J Dilla pocket. |
| 5 | Breaks / Drum & Bass | 3 | Chopped-break lineage — classic breakbeat, jungle, liquid DnB. |
| 6 | Latin / Brazilian | 5 | Cuban clave family plus Brazilian bossa/samba. |
| 7 | African | 5 | Sub-Saharan traditions and Afrobeat — Ewe, Manding djembe, Afrobeat 12/8, Afrobeat Lagos. |
| 8 | Asian Traditions | 7 | Indonesian gamelan (Javanese colotomic, Balinese kotekan) plus Indian classical (Carnatic Adi tala, Hindustani Tintal, Rupak Tal). |
| 9 | Balkan / Eastern European | 4 | Additive-cell grooves in 7/8 and 11/8 plus their funk fusion. |
| 10 | Experimental / Fusion | 2 | Genre-crossing hybrids and glitch — IDM Glitch, Afro-Electronic Fusion. |

Totals: 3 + 5 + 4 + 5 + 3 + 5 + 5 + 7 + 4 + 2 = **43**.

## Preset-to-category mapping

Indexes match `kInfos[]` order in `engine/src/presets.cpp`.

| # | Preset name | Category |
|---|-------------|----------|
| 0 | Four on the Floor | House / Techno |
| 1 | Polymetric Drift | Foundational |
| 2 | Sparse Pulse | Foundational |
| 3 | Breakbeat | Breaks / Drum & Bass |
| 4 | Latin Feel | Latin / Brazilian |
| 5 | Afro-House Phrases | House / Techno |
| 6 | Reich Phasing | Minimalist / Compositional |
| 7 | Kotekan Interlock | Asian Traditions |
| 8 | Pocket Groove | Jazz / Funk / Soul |
| 9 | Afrobeat 12/8 | African |
| 10 | Balkan Aksak | Balkan / Eastern European |
| 11 | Bossa Nova | Latin / Brazilian |
| 12 | Carnatic Tala | Asian Traditions |
| 13 | IDM Glitch | Experimental / Fusion |
| 14 | Sub-Saharan: Agbekor | African |
| 15 | Gamelan: Colotomic | Asian Traditions |
| 16 | Polymetric Foundation | Foundational |
| 17 | Ewe Polymetric Ensemble | African |
| 18 | Manding Djembe | African |
| 19 | Cuban Son Montuno | Latin / Brazilian |
| 20 | Afrobeat Lagos | African |
| 21 | Balinese Kotekan | Asian Traditions |
| 22 | Javanese Colotomic | Asian Traditions |
| 23 | Tintal Groove | Asian Traditions |
| 24 | Rupak Tal | Asian Traditions |
| 25 | Rachenitsa 7/8 | Balkan / Eastern European |
| 26 | Kopanitsa 11/8 | Balkan / Eastern European |
| 27 | Reich Phase Process | Minimalist / Compositional |
| 28 | Riley Layered Entry | Minimalist / Compositional |
| 29 | Nancarrow Tempi | Minimalist / Compositional |
| 30 | Minimal Techno | House / Techno |
| 31 | Deep House | House / Techno |
| 32 | Samba Batucada | Latin / Brazilian |
| 33 | Bossa Nova Trio | Latin / Brazilian |
| 34 | Classic Funk | Jazz / Funk / Soul |
| 35 | Neo-Soul Pocket | Jazz / Funk / Soul |
| 36 | Jazz Bop Ride | Jazz / Funk / Soul |
| 37 | Elvin Jones Cascade | Jazz / Funk / Soul |
| 38 | Jungle Break | Breaks / Drum & Bass |
| 39 | Liquid Drum and Bass | Breaks / Drum & Bass |
| 40 | Afro-Electronic Fusion | Experimental / Fusion |
| 41 | Balkan Funk | Balkan / Eastern European |
| 42 | Compositional Arc | Minimalist / Compositional |

## Judgement calls worth flagging

Three assignments merited a real decision rather than an obvious placement:

- **Polymetric Drift** and **Sparse Pulse** sit with Foundational rather than Minimalist. Both are style-agnostic engine demos (Drift shows prime-number cycles; Sparse Pulse shows wide spacing + ghost mutation). Minimalist is reserved for presets that reference a specific compositional practice (Reich, Riley, Nancarrow, and the Compositional Arc build). This keeps "Foundational" as the first-thing-to-try tutorial bucket.
- **Balkan Funk** goes to Balkan / Eastern European rather than Experimental / Fusion. The description is `7/8 aksak with funk ghost notes and micro-timing on the hi-hat` — an aksak groove with funk seasoning, not a genre-crossing hybrid. The metric structure dominates.
- **Asian Traditions** intentionally spans Indonesian gamelan and Indian classical (Carnatic + Hindustani) rather than splitting into two categories. Splitting would put us at 11 categories (over the cap) and give Indian only 3 entries; the shared ground — cyclical / nested / non-Western classical tradition — is defensible for a browsing menu. If the count grows past ~10 in either family, this should split into Gamelan / Southeast Asian and Indian in a future round.

## Adding a new preset

1. Add the `PresetInfo` entry to `kInfos[]` in `engine/src/presets.cpp`, including a `category` field pointing at one of the ten strings in `kFactoryPresetCategories`.
2. Add a row to the preset-to-category table above. If the assignment isn't obvious, add a paragraph to "Judgement calls worth flagging" explaining why.
3. Rerun `npm --prefix site run generate-presets` and commit the regenerated `site/src/generated/presets.json` and `site/public/webui/presets.json`.
4. If total preset count in any category crosses 8, revisit the category structure rather than growing it further.

## Adding a new category

1. Append a new string to `kFactoryPresetCategories[]` in `engine/src/presets.cpp` (order matters — this is menu order).
2. Update the count in the header comment and in this doc.
3. Bump `schemaVersion` in `engine/tools/emit_presets.cpp` only if the JSON shape itself changes; adding a category string does not require a bump.
4. Regenerate the JSON as above.
