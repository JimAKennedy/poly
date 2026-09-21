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

_Nothing queued._

## Resolved

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
