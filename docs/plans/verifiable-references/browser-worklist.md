# Browser worklist

URLs that automation cannot settle and a human can. M003/S02 extends this file;
M001/S01 created it with the first entry.

Some scholarly hosts refuse scripts as a matter of policy — five entries in the
bibliography return 403 and one returns 406 while being perfectly reachable in a
browser. The D-Scholarship@Pitt record behind `ref-9` is the proven case: it
403s every automated fetch tried, and the owner loaded it and downloaded the PDF
without difficulty. Treating "a script cannot fetch it" as "unobtainable" would
condemn good sources, so those entries come here instead of being replaced.

An entry leaves this queue by **moving to `## Resolved`**, never by being
deleted. The record of what was checked and when is the point of the file, and a
deleted row is indistinguishable from a row that was never added.

Downloaded documents go to the Dropbox research archive at
`~/Library/CloudStorage/Dropbox/Research/drum generator/References`, or to a
`.gitignore`d directory in the project root.

Point the repo at whichever you use by exporting **`POLY_REFERENCES_ARCHIVE`**.
Unset, it falls back to `.references/` in the project root, which is
`.gitignore`d. The manifest at `site/src/data/references.json` records a bare
filename and never a path, so no machine-specific root can reach a tracked file
— `check-personal-paths` rejected the absolute form in the vision's own first
draft, and this makes that failure unreachable rather than merely reviewed.

## Pending

### Queued by M002/S02 — 19 entries that are browser-only and unverified

Every entry below is obtainable by a person and was not obtained by automation.
Grouped by host, because hosts fail as a class: one session on a host settles
every entry on it. The manifest at `site/src/data/references.json` carries the
measured evidence for each.

#### Academia.edu

| Entry | URL | What to check |
|---|---|---|
| `ref-30` | `https://www.academia.edu/12360331/Structures_of_Rhythm_in_Mevlevi_Music_A_Cyclical_Analysis_Model` | You have an account. Confirm the paper’s title and author, and save the PDF. |

#### IFTAWM / AAWM

| Entry | URL | What to check |
|---|---|---|
| `ref-23` | `https://journal.iftawm.org/wp-content/uploads/2023/12/Reindl_AAWM_Vol_11_2.pdf` | Read the PDF’s title page and record the exact article title and journal name. This settles VR17. |

#### No URL recorded

| Entry | URL | What to check |
|---|---|---|
| `fr-collins-2001` | — none in the entry — | The entry carries no URL. Find the work, then record a route to it. |
| `fr-novotney-1998` | — none in the entry — | The entry carries no URL. Find the work, then record a route to it. |
| `ref-39` | `https://www.cambridge.org/core/services/aop-cambridge-core/content/view/BBC410F9849DB982AEBFACEA14D38F32/S0261143023000041a.pdf/shaping_rhythm_timing_and_sound_in_five_groovebased_genres.pdf` | The entry carries no URL. Find the work, then record a route to it. |
| `ref-8` | `https://www.scribd.com/document/218700737/unit-6-62-mustapha-tettey-addy-ghana-agbekor-dance` | The entry carries no URL. Find the work, then record a route to it. |

#### Oxford Academic

| Entry | URL | What to check |
|---|---|---|
| `ref-18` | `https://academic.oup.com/edited-volume/28278/chapter/214416541` | Record whether the chapter is readable, purchasable or institution-only, and the price if one is shown. |
| `ref-19` | `https://academic.oup.com/book/6495/chapter/150390558` | Record whether the chapter is readable, purchasable or institution-only, and the price if one is shown. |

#### PubPub

| Entry | URL | What to check |
|---|---|---|
| `ref-24` | `https://alpaca.pubpub.org/pub/ofzrb3a6/release/12` | Open access behind a bot block. Confirm the title and save the PDF. |

#### Semantic Scholar

| Entry | URL | What to check |
|---|---|---|
| `ref-46` | `https://www.semanticscholar.org/paper/The-Theory-of-Rep-Rate-Pattern-Generation-in-the-Bjorklund/c652d0a32895afc5d50b6527447824c31a553659` | Find and save Bjorklund (2003); it underpins the engine’s Euclidean generator. |

#### YouTube

| Entry | URL | What to check |
|---|---|---|
| `ref-10` | `https://www.youtube.com/watch?v=T-bXVeAmGiM` | Confirm the video exists and note its actual title and uploader. There is no text to review, so this settles existence only — removal is M004/VR12. |
| `ref-11` | `https://www.youtube.com/watch?v=OE7X1PgmF54` | Confirm the video exists and note its actual title and uploader. There is no text to review, so this settles existence only — removal is M004/VR12. |
| `ref-14` | `https://www.youtube.com/watch?v=OoyGKEqfyZw` | Confirm the video exists and note its actual title and uploader. There is no text to review, so this settles existence only — removal is M004/VR12. |
| `ref-15` | `https://www.youtube.com/watch?v=ryTTHmUYc2o` | Confirm the video exists and note its actual title and uploader. There is no text to review, so this settles existence only — removal is M004/VR12. |
| `ref-25` | `https://www.youtube.com/watch?v=uephXrkxH1E` | Confirm the video exists and note its actual title and uploader. There is no text to review, so this settles existence only — removal is M004/VR12. |

#### lianproductions.com

| Entry | URL | What to check |
|---|---|---|
| `ref-17` | `https://lianproductions.com/afro-house/` | Answers a 202 challenge stub. Confirm the article exists and note its title. |

#### martinscherzinger.org

| Entry | URL | What to check |
|---|---|---|
| `fr-scherzinger-2010` | — none in the entry — | The host did not respond at all. Check whether the PDF is still there; if not, find the ICTM proceedings version. |
| `ref-35` | `http://martinscherzinger.org/wp-content/uploads/Piano-Phase-in-Global-Perspective-.pdf` | The host did not respond at all. Check whether the PDF is still there; if not, find the ICTM proceedings version. |

#### Érudit

| Entry | URL | What to check |
|---|---|---|
| `ref-41` | `https://www.erudit.org/en/journals/sqrm/2015-v16-n1-2-sqrm03043/1039619ar.pdf` | Behind an Anubis challenge that returns HTTP 200. Confirm the article title and save the PDF. |


## Resolved

### The six library-only entries queued by first-release M003/S02 — **all settled, 2026-09-22**

The owner worked the queue, reproduced below as it was written, in one session. Every entry was verified
against its own title page after download, and each manifest record in
`site/src/data/references.json` carries the evidence. Two findings came out of
the check that no listing could have produced:

- The file first saved as Cohn was byte-identical to the Vitale download; the
  owner re-fetched Cohn and the second file was verified before anything was
  recorded.
- The Grove article a subscriber reaches is the **2001** second-edition rewrite
  by Qureshi, Powers, Katz, Widdess and sixteen others, not Powers's 1980
  article the entry named. Same title, different edition and authorship. The
  entry now cites the 2001 article; the anchor `fr-powers-1980` keeps its name
  because anchors are never reassigned.

| Entry | Outcome | Route | Verdict |
|---|---|---|---|
| `fr-cohn-1992` | found | ResearchGate repost of the JSTOR export, account needed; 403 to scripts | browser-only, verified |
| `fr-locke-1982` | found | JSTOR under the owner's JPASS personal subscription | purchasable, verified |
| `fr-brailoiu-1951` | found | JSTOR under the owner's JPASS personal subscription | purchasable, verified |
| `fr-harrison-2025` | not found outside the book | owner's decision: keep the citation, point it at the Routledge book | purchasable, unverified |
| `fr-vitale-1990` | found | `gamelan.org` back-issue PDF, open to scripts and browsers alike | open-access, verified |
| `fr-powers-1980` | found, as the 2001 edition | Grove Music Online, subscription login; the "Rhythm and tāla" section is the one chapter 6 leans on | purchasable, mismatch recorded and the entry corrected |


<details><summary>The queue as written</summary>

### Queued by first-release M003/S02 — six library-only entries

These six are cited from shipping chapters and are the last entries in the
appendix that need an institution. Automation found no obtainable copy for any
of them; four are paywalled by publication model rather than by bot-blocking, so
a browser alone will not change those — but an **Academia.edu or Scribd** copy
would, and that is where reposted copies of canonical articles tend to sit.

Found → the citation is kept and repointed. Not found → the citing chapter's
claim is reworded to stop depending on it. Until then FR18 is paused, and with
it milestone M003.

| Entry | The work | Where to look | If found |
|---|---|---|---|
| `fr-cohn-1992` | Cohn, "Transpositional Combination of Beat-Class Sets in Steve Reich's Phase-Shifting Music", *Perspectives of New Music* 30(2) | Academia.edu, ResearchGate — it won SMT's Outstanding Publication Award, so copies circulate | save the PDF, note the URL |
| `fr-locke-1982` | Locke, "Principles of Offbeat Timing and Cross-Rhythm in Southern Ewe Dance Drumming", *Ethnomusicology* 26(2) | Academia.edu; Tufts course pages under `sites.tufts.edu/davidlocke/` | save the PDF, note the URL |
| `fr-brailoiu-1951` | Brăiloiu, "Le rythme aksak", *Revue de Musicologie* 33(99/100) | Academia.edu; French musicology scans circulate. **The origin of the term "aksak"** — chapter 7 rests on it | save the PDF, note the URL |
| `fr-harrison-2025` | Harrison, "Reflections on the Amen Break", *The Routledge Companion to Remix Studies* 2nd ed. | Academia.edu — Nate Harrison self-archives; his 2004 "Can I Get an Amen?" is widely posted | save the PDF, note the URL |
| `fr-vitale-1990` | Vitale, "Kotekan: The Technique of Interlocking Parts in Balinese Music", *Balungan* 4(2) | `gamelan.org` — the American Gamelan Institute publishes *Balungan*; check for a back-issue archive behind the front page | save the PDF, note the URL |
| `fr-powers-1980` | Powers, "India, subcontinent of", *The New Grove Dictionary of Music and Musicians* vol. 9 | Grove Music Online through any public library card — many give free Grove access | note the access route |

</details>


### `ref-22` — NIOS Hindustani Music (242) — **replaced, 2026-09-20**

Settled editorially, not by the browser check this row was raised for. The
owner identified the entry as course material: the National Institute of Open
Schooling's teaching text, the same class as the Fiveable study guide removed
from `ref-26`. Whether a teaching PDF loads does not make it a citable source,
so the liveness question was never worth asking and the check was not spent.

Replaced by Clayton, M. (2020), "Theory and Practice of Long-form
Non-isochronous Meters: The Case of the North Indian *rūpak tāl*", *Music
Theory Online* 26(1), DOI `10.30535/mto.26.1.2` — peer-reviewed, platinum open
access, readable without login, and on North Indian tāl.

The measurement that raised the row is retained below, because it remains true
and because the host is likely to recur in this bibliography.
