# Security Policy

## Supported versions

Only the latest release on the `main` branch is supported with security fixes.

## Reporting a vulnerability

If you find a security issue in Poly, please report it privately:

**Email:** the.jim.kennedy@gmail.com

Include:
- Description of the vulnerability
- Steps to reproduce
- Affected component (engine, plugin, state serialization, MIDI processing)

## Response timeline

- **Acknowledgement:** within 48 hours
- **Fix timeline:** within 1 week of acknowledgement
- **Disclosure:** coordinated after a fix is available

## Scope

Security-relevant areas include:
- VST3 state deserialization (`setState`)
- MIDI file parsing and export
- Plugin parameter handling
- Any code running in the audio thread (real-time safety)
