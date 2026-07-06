# CC Drum Sample Sourcing Guide — poly.jk.digital

Research date: 2026-07-06. Purpose: pair playable one-shot samples with the
site's pattern examples and presets (Ewe/Afrobeat, Afro-Cuban, Brazilian,
Balkan, Carnatic, Gamelan, Reich phasing, and the electronic/acoustic kits).

**How "verified" is defined here:** the license text/statement was fetched
and read on the actual source page during this research. The research
environment could reach github.com but was blocked from freesound.org,
archive.org, freepats.zenvoid.org, philharmonia.co.uk, and most other
sample hosts — so GitHub-hosted collections are fully verified, and
everything else is an **unverified lead** with the exact URL queued for a
browser check. Nothing in the "verified" tables needs re-checking; every
lead needs one page-load before use.

---

## 1. Executive summary

- **Backbone (adopt now): VCSL — Versilian Community Sample Library**
  (github.com/sgossner/VCSL, **CC0**, verified). One library covers the
  bell/cowbell/shaker/conga/bongo/claves/güiro/darbuka/frame-drum/
  woodblock/gong/balafon/cajón/tambourine roles with velocity layers and
  round-robins. No attribution required.
- **Electronic presets solved, CC0:** the Michael Fischer 1994 TR-808 set
  (github.com/tidalcycles/sounds-tr808-fischer, verified CC0, recorded
  directly from hardware with explicit provenance) + Boochi44's CC0
  processed kits for IDM character.
- **Acoustic kits, verified CC-BY 4.0:** FreePats **MuldjordKit** and
  DrumGizmo **DRSKit** (GitHub mirrors verified; attribution strings below).
- **Gamelan solved outside CC:** the Casa da Música Javanese Gamelan
  (GitHub, verified **Artistic License 2.0**, redistribution permitted)
  covers bonang/gendér/saron/kenong/kethuk/kempyang/gong in both Pelog and
  Slendro — the whole kotekan/colotomy chapter — pending a one-line policy
  call to admit one OSI license alongside CC.
- **Five gaps remain** (no verified CC source yet): djembe/dunun family,
  the Brazilian set (surdo/tamborim/pandeiro/caixa/cuíca), timbales,
  mridangam/tabla under true CC, and a 909/Linn-style kit. All have
  specific leads in §5 — most are one Freesound page-check away.
- **Policy:** allowlist CC0 + CC-BY only; manifest + generated credits
  page; REUSE-style per-file licensing beside the GPLv3 code (§6).

## 2. Verified sources (license read on source page)

| Source | URL | License | Attribution | Covers |
|---|---|---|---|---|
| VCSL (Versilian Community Sample Library) | github.com/sgossner/VCSL | CC0-1.0 | none (courtesy: "VCSL — Versilian Studios LLC, CC0") | Agogô bells (hi/lo), 2 cowbells, cabasa, shakers, claves, güiro, woodblock (pp–ff), congas (conga/quinto/tumba, 48 WAV, v1–v4 × rr), bongos (48 WAV, open/muted), **darbuka**, frame drum, 2 gongs, balafon, cajón, 3 tambourines, finger cymbals, slit drum, bass drums, snares, toms |
| VSCO-2 CE | github.com/sgossner/VSCO-2-CE | CC0-1.0 | none | Second timbre for conga/quinto, claves, cowbell, güiro; orchestral perc (concert BD, snares, timpani, mallets) |
| FreePats World Percussion | github.com/freepats/world-percussion | CC0-1.0 | none (credits courtesy: Xavimart/FreePats) | Cajón flamenco, bongos, darbuka, conga, claves, castanets, maracas, egg shaker, tambourine, hand clap (24 one-shots, SFZ-mapped) |
| Fischer TR-808 set (1994) | github.com/tidalcycles/sounds-tr808-fischer | CC0-1.0 (LICENSE fetched) | none (courtesy: "808 samples by Michael Fischer, 1994") | 116 WAV: 25 kicks, 25 snares, toms/congas, rimshot, claves, clap, maracas, cowbell, cymbals, open/closed hats. Direct-from-hardware provenance stated |
| Boochi44 free-drum-samples | github.com/Boochi44/free-drum-samples | CC0-1.0 (README; standalone LICENSE not confirmed — minor asterisk) | none | 3 processed kits (kicks/subs/snares/claps/hats/perc/FX) — IDM glitch character |
| FreePats MuldjordKit | github.com/freepats/MuldjordKit | CC-BY 4.0 | **"Drum samples provided by DrumGizmo.org"** (repo-specified) | Full rock kit: 2 kicks, snare, 4 toms, hi-hat, 2 crashes, 2 rides, china (SFZ+WAV) |
| DrumGizmo DRSKit | github.com/sfzinstruments/DrumGizmo.DRSKit | CC-BY 4.0 (license sentence read on mirror) | "DRSKit by Jes Eiler (DRSDrums) & the DrumGizmo team — CC BY 4.0" | Kick, snare, 3 toms, hi-hat, 2 crashes, ride; brushes variant; graded-strength hits (ghost notes) |
| Dim Cabasa | github.com/sfzinstruments/kinwie.dim-cabasa | CC-BY 4.0 | "Dim Cabasa by Kinwie — CC BY 4.0" | 250 cabasa samples, 24-bit/48k FLAC |
| Mailbox Badger PD Drum Samples Vol. 1–2 | archive.org/details/HeatDish + …volume2 | Public Domain Mark (uploader = creator; corroborated via artist's Bandcamp) | none required; description requests courtesy credit — honor it | Acoustic drum one-shots (lo-fi/character) |
| **CDM Javanese Gamelan** *(non-CC, policy call)* | github.com/Digitopia/CDM-GAMELAN-SAMPLE-LIBRARY | **Artistic License 2.0** (verified; verbatim redistribution explicitly permitted, keep copyright notice) | retain "Copyright 2014 Digitopia - Casa da Música" notice | Folder-verified: **bonang, gendér, gambang, saron, kenong, kethuk, kempyang, gong, drums** — Pelog AND Slendro. Covers the full colotomic + balungan set for the kotekan/colotomy chapters |
| naad tabla composer *(MIT, provenance caveat)* | github.com/oormicreations/naad | MIT (verified) | per MIT | ~120 real tabla stroke WAVs organized by bol (Dha, Dhin, Ge, Na, Ta, Tin, Tun…). **Caveat:** sample provenance undocumented — email author (oormicreations@gmail.com) to confirm recordings are original before shipping |
| Berklee Sampling Archive *(flagged: license per item page, host blocked)* | archive.org/details/Berklee44v1 (…v1–v13) | CC-BY 3.0 (per item pages + creativecommons.org announcement — re-confirm) | TASL per volume | 6,500+ world samples incl. **tabla**, congas; mono 16-bit, utilitarian |

## 3. Site-chapter / preset mapping

| Chapter / preset | Instruments | Status |
|---|---|---|
| Reich phasing | woodblock, claves | ✅ VCSL (CC0) |
| Afro-Cuban / Latin Feel | claves, congas, bongos, güiro, cencerro | ✅ VCSL (+FreePats); ⚠ timbales/cascara gap; conga strokes labeled Hit/Tap not open/slap/bass — audition |
| Ewe / Afrobeat 12/8 | gankogui→VCSL agogô bells; axatse→VCSL cabasa/shakers or Dim Cabasa; kick/snare/shaker per kit | ✅ roles covered by substitutes; ⚠ true djembe/dunun gap |
| Balkan aksak | davul, darbuka, riq | ✅ darbuka (VCSL/FreePats); frame drum + tambourine as riq substitute; ⚠ davul gap (leads) |
| Bossa Nova | surdo, tamborim, agogô, pandeiro | agogô ✅ VCSL; ⚠ surdo/tamborim/pandeiro/caixa/cuíca gap (leads) |
| Carnatic tala | mridangam, tabla | ⚠ mridangam: CompMusic stroke dataset lead (Freesound pack 14157/akshaylaya + Zenodo mirror, historically CC-BY); tabla: naad (MIT, confirm provenance) or Berklee (CC-BY 3.0 flag) |
| Kotekan / gamelan | gangsa/bonang, kenong/kethuk, gongs | ✅ CDM Gamelan covers bonang/gendér/saron/kenong/kethuk/kempyang/gong in both tunings (Artistic-2.0 policy call); VCSL gongs as CC0 fallback for gong role only |
| Four-on-floor / DnB / Breakbeat | 808-style kit | ✅ Fischer set (CC0); ⚠ 909-style gap (leads) |
| IDM glitch | processed kit | ✅ Boochi44 (CC0) + own mangles of Fischer (CC0 allows) |
| Pocket groove (Dilla) | acoustic kit w/ ghost notes | ✅ MuldjordKit or DRSKit (CC-BY); best velocity layering (AVL) is CC-BY-**SA** — excluded by default policy |

## 4. Rejected sources (verified or strongly corroborated reasons)

- **Philharmonia Orchestra** — custom terms forbid making samples available
  "as is"; third-party mirrors claiming CC-BY-SA contradict the orchestra's
  own license. (Has djembe/dunun/surdo — tempting, unusable.)
- **Goldbaby** — terms explicitly forbid redistribution on a website.
- **Hydrogen drumkits** — per-kit licenses; the two checked (GMRockKit,
  TR808EmulationKit) are GPL-on-audio: avoid.
- **Salamander Drumkit / AVL Drumkits** — CC-BY-**SA** (not BY as widely
  assumed); excluded by the SA-free policy (revisit only if policy changes).
- **KB6 / Reverb / MusicRadar mirrors on Archive.org** — license laundering:
  third-party CC stamps on uncleared or custom-licensed content.
- **Looperman / SampleFocus / 99Sounds / SampleRadar / BPB packs** — all
  "royalty-free for your music" EULAs that expressly prohibit redistribution;
  none can feed a public repo/playground.
- **tidalcycles/Dirt-Samples** — no LICENSE, no provenance.
- **Sonatina (CC Sampling+, retired license), Virtual Playing Orchestra
  (mixed terms)** — skip.

## 5. Verification queue (unverified leads — one browser check each)

Freesound is per-SOUND licensed (CC0 / CC-BY / CC-BY-NC; 4.0 since 2023,
older files 3.0) — check every file, or use the API which returns license +
attribution machine-readably: `filter=license:"Creative Commons 0"`.
Handy search URL pattern:
`https://freesound.org/search/?q=INSTRUMENT&f=license:%22Creative+Commons+0%22`

| Gap | Lead | URL |
|---|---|---|
| Timbales + cascara | Sassaby "Percussion Samples" (LP Matador per snippet) | freesound.org/people/Sassaby/packs/28352/ |
| Congas w/ open/slap labels; bongos | MrRentAPercussionist | freesound.org/people/MrRentAPercussionist/packs/25693/ |
| Djembe | domingus pack; Infinita08 pack (AKG P220) | freesound.org/people/domingus/packs/3456/ · …/Infinita08/packs/26031/ |
| Dunun family | eboxweb singles (dundunba/sangban/kenkeni) | freesound.org/people/eboxweb/downloaded_sounds/ |
| Agogô alt | pjcohen "Agogo Bells Percussion Set" | freesound.org/people/pjcohen/packs/23370/ |
| Davul | Panotsi pack; Despoina_kalantzi pack | …/Panotsi/packs/19730/ · …/Despoina_kalantzi/packs/19731/ |
| Doumbek strokes | hollandm (dum/tek tags) | freesound.org/people/hollandm/packs/38695/ |
| Surdo | plotzki "Samba Percussion" | freesound.org/people/plotzki/packs/2301/ |
| Pandeiro | reinsamba pack + pgonsilva/CWBudde singles | …/reinsamba/packs/1298/ |
| Cuíca | reinsamba (may be loops) | …/reinsamba/packs/1339/ |
| Mridangam | CompMusic/IITM stroke dataset (~10 stroke classes: tha/thi/thom/num/chapu/dheem…, reportedly CC-BY 3.0) | freesound.org/people/akshaylaya/packs/14157/ (+ sibling tonic packs) · zenodo.org/record/4068196 |
| Tabla bols alt | mmiron "tabla bols" pack | freesound.org/people/mmiron/packs/8162/ |
| Balinese gamelan | Akito van Troyer sf2 (snippet: CC-BY 3.0) | musical-artifacts.com/artifacts/3063 |
| Tabla | Berklee volumes (CC-BY 3.0 flag) | archive.org/details/Berklee44v1 |
| 909/LM-1-style | PJCmusic LM-1; rossf LM-1; oceansonmars CR-78; sakebeats 909 snares; deadrobotmusic; RokZRooM CC0 pack | freesound URLs in report; scrutinize 909 provenance (ROM-based hats) |
| Gamelan (CC option) | UC Davis/Elisa Hough set — permission required (samples excluded from the MIT-licensed sequencer that uses them) | elisahough.com/sounds/sampler.html |
| Quality contributors to sweep | InspectorJ (CC-BY, has credit format), waveplaySFX (CC0?), menegass, DWSD | freesound.org/people/… |

Also verify if ever needed: FreePats site SF2 variants, DrumGizmo
CrocellKit/Aasimonster (CC-BY per snippets), U. Iowa terms.

## 6. Licensing policy & workflow

- **Allowlist:** `CC0-1.0`, `CC-BY-4.0` (+ `CC-BY-3.0` legacy). Exclude
  NC (public repo → downstream commercial reuse you can't police), ND, SA
  (SA note: playback of an unmodified sample = "collection", fine; mixing
  into rendered demo audio = adaptation → the demo becomes SA — simplest to
  exclude SA entirely). Decide separately whether Artistic-2.0 (CDM
  Gamelan) joins the allowlist.
- **GPLv3 repo compatibility:** side-by-side assets are "mere aggregation"
  — no conflict. Use the REUSE pattern: `LICENSES/` dir with the license
  texts, per-file `.license` sidecars or `REUSE.toml`, SPDX IDs.
- **Manifest:** one entry per file — `file, title, author, author_url,
  source_url, license (SPDX), license_url, modifications, retrieved,
  attribution` — CI fails on missing entries or non-allowlisted IDs.
  Snapshot each source page at download time (uploader can change license
  later; your grant stands, the snapshot is evidence).
- **Credits, generated from the manifest (3 outputs):** site `/credits`
  page (TASL format; linked from footer + a "Sounds" info control in the
  playground — CC-BY 4.0 §3(a)(2) explicitly blesses a linked credits
  page); repo `CREDITS.md` + README pointer; credits text alongside any
  shipped demo content. Credit CC0 authors too (etiquette).
- **Attribution string format:** `"Title" by Author (source URL), CC BY
  4.0 (license URL). Modifications: trimmed, normalized.` Freesound also
  auto-generates your attribution list at freesound.org/home/attribution/.
- **Trademark naming:** "808-style / 909-style / LM-1-style"; never "TR-808
  Kit"; no Roland trade-dress colors in UI; footer disclaimer ("Not
  affiliated with or endorsed by Roland Corporation…"). Roland registered
  TR-808/TB-303 trade dress in 2018-19; no known action against sample
  packs, but recordings-of-hardware provenance is the real risk — prefer
  sets with explicit provenance (Fischer) or synthesized recreations.
- **Archive.org rule:** `licenseurl` is uploader-declared — embed only when
  (a) uploader is identifiably the creator, (b) independently corroborated,
  (c) not a ROM rip.

## 7. Suggested next steps

1. Pull VCSL + Fischer 808 + FreePats World Percussion + MuldjordKit;
   audition against the presets; build `samples/manifest.json` as you go.
2. From an unrestricted browser (or the server-side session if its network
   allows): run the §5 verification queue — ~15 minutes with the Freesound
   license filter; the API (free key) automates it.
3. Policy call: Artistic-2.0 for CDM Gamelan (recommend yes — OSI-approved,
   redistribution permitted, and it's the only real gamelan found).
4. Wire the manifest → credits generator + CI allowlist check.
5. Re-audition gaps after verification; anything still missing (likely
   riq, cuíca one-shots, true shekere) → record or synthesize in-house and
   release CC0, closing the loop.
