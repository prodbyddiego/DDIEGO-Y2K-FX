# DDIEGO Y2K FX editor

## Surface and intent

The editor is an operational sample instrument for a DAW: the loaded sound and
the current processing mode should be understandable at a glance. The waveform
is a real min/max preview of the decoded sample, not decorative artwork.

## Visual system

- **Canvas:** ink navy (`#080D18` to `#101529`) with subtle 48 px grid lines.
- **Panels:** deep blue-black (`#111A29`) with a 1 px slate-cyan border
  (`#24384A`) and rounded 12–15 px corners.
- **Signal:** bright cyan (`#53F2E5`) for waveform, active controls, and primary
  signal details.
- **Slices and interaction:** neon magenta (`#FF4FC8`) for slice divisions,
  active drop targets, and reverse accents.
- **Text:** cool white (`#E6F8FB`) for names and values; muted blue-gray
  (`#8DAAB7`) for secondary labels.
- **Typography:** JUCE's system sans-serif; bold uppercase for the product,
  mode, and control labels; compact uppercase for supporting readouts.

## Layout and behavior

The resizable 1120 × 820 editor keeps mode selection and sample loading at the
top, the waveform below, and the active engine's controls in the lower deck.
Granular controls use a four-column, two-row grid. Chopper controls use a
two-row grid with slice division, reverse, and random-map actions above a
two-octave C1–B2 MIDI keyboard. The minimum editor size is 920 × 740.

Empty waveform state explains loading and drag-and-drop. Dragging a file over
the waveform highlights the target; decode failures appear in the status line.
The waveform's chopper markers reflect the selected division, start offset, and
slice length. Controls retain native JUCE interaction and expose their values
through host-automatable parameters.
