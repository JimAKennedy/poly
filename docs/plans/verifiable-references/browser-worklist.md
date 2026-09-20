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

## Pending

### `ref-22` — NIOS Hindustani Music (242), theory book 1, chapter 3

| | |
|---|---|
| **URL** | `https://nios.ac.in/media/documents/Hindustani_Music_242/hindustanimusictheorybook1/HMB1Ch3.pdf` |
| **Check** | Does the PDF load in a browser? |
| **Save** | If it loads, the PDF, named `ref-22-nios-hindustani-music-ch3.pdf` |
| **Queued** | 2026-09-20 |

**Why it is here rather than being replaced.** The ledger row VR02 says the link
"does not respond" and that "the PDF moved rather than the source being bad".
Re-measurement supports neither claim:

- the PDF times out — 60s, no response, not a 404
- `HMB1Ch1.pdf`, a sibling chapter in the same directory that a search engine
  lists as live, times out identically
- `https://nios.ac.in/` — the site root — times out too
- DNS resolves to a single A record, so this is not a name-resolution failure

Nothing on the host answers from this network. The path may be perfectly good.
Replacing India's national open-schooling board as a source on evidence that
only proves one network cannot reach it is the failure this programme exists to
end, so the question goes to a browser.

**Outcomes and what each means.** The PDF loads → the entry is sound and only
unreachable from automation; save it and record the hand-verification. It 404s →
the path really has rotted and VR02's original framing was right. The host is
unreachable for the owner too → same conclusion, better evidence.

## Resolved

_None yet._
