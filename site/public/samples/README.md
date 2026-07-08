# Poly Site Samples

Drum samples used by the interactive previews on [poly.jk.digital](https://poly.jk.digital).

## Directory layout

```
samples/
  manifest.json          # every sample → license, attribution, MIDI mapping
  schema.json            # JSON Schema for manifest validation
  LICENSES/              # full license texts (REUSE-style)
    CC0-1.0.txt
    CC-BY-4.0.txt
    CC-BY-3.0.txt
    Artistic-2.0.txt
  kick/                  # samples grouped by instrument role
  snare/
  ...
```

## Adding samples

1. Place the `.ogg` file under the appropriate role directory (create one if needed).
2. Add an entry to `manifest.json` with `file`, `spdx`, `source`, `midiNotes`, and `role`.
   - CC-BY / Artistic-2.0 / MIT samples **must** include an `attribution` string.
   - CC0 samples may omit `attribution` (courtesy credit is optional).
3. Run `bash scripts/check-sample-manifest.sh` to validate.
4. CI enforces: every file has a manifest entry, every entry points to an existing file, and only allowlisted SPDX IDs are used.

## License policy

| SPDX ID | Allowed | Notes |
|---|---|---|
| CC0-1.0 | Yes | No attribution required |
| CC-BY-4.0 | Yes | Attribution on /credits page |
| CC-BY-3.0 | Yes | Attribution on /credits page |
| Artistic-2.0 | Yes | Retain copyright notice |
| MIT | Yes | Retain copyright notice |
| CC-BY-SA-* | **No** | ShareAlike incompatible with site license |
| GPL / custom | **No** | Redistribution restrictions |

## Format requirements

- **OGG Vorbis** (`.ogg`) — primary format for Web Audio
- 44.1 kHz or 48 kHz, mono or stereo
- Trimmed silence, normalized to -1 dBFS peak
- One-shots only (no loops)
