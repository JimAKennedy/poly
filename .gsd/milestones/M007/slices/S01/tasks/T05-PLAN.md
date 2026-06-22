---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T05: Phase alignment view shows phrase boundaries

Update phase_alignment_view to draw phrase boundaries (play/gap regions) as colored bands behind the phase indicators. Play regions in lane color, gap regions dimmed/hatched.

## Inputs

- `plugin/source/ui/phase_alignment_view.cpp`

## Expected Output

- `Updated phase alignment view with phrase visualization`

## Verification

Build compiles; visual smoke test passes; Cubase UAT shows phrase boundaries in UI
