# Vendored: Valley Audio's Plateau DSP

- Upstream: https://github.com/ValleyAudio/ValleyRackFree
- Commit: `86f02e431136a7f5c96a872b99b7115b7e133e05` ("Bump version number")
- Licence: GPL-3.0-or-later (`LICENSE.md`, `LICENSE-GPLv3.txt`), Copyright (c) Dale Johnson / Valley Audio Soft.
  `src/Plateau/Plateau.hpp` also carries a BSD-style header; it is not vendored (its control logic is re-written in
  `src/engine.cc`).

## Files (paths as upstream, under `src/`)

| File | What it is |
| --- | --- |
| `Plateau/Dattorro.hpp`, `Plateau/Dattorro.cpp` | The reverb: input chain, Dattorro 1997 tank, freeze crossfade |
| `dsp/delays/InterpDelay.hpp`, `dsp/delays/AllpassFilter.hpp` | Delay line and allpass used by the tank |
| `dsp/filters/OnePoleFilters.hpp`, `dsp/filters/OnePoleFilters.cpp` | Input and tank damping filters, DC blockers |
| `dsp/modulation/LFO.hpp` | The tank's four tri/saw LFOs |
| `dsp/shaping/NonLinear.hpp` | `tanhDriveSignal`, the module's output saturation |
| `utilities/Utilities.hpp` | `linterp`, `scale` |

## Local changes

None. The files are byte-identical to the upstream commit.

`Plateau/Plateau.cpp` (the VCV Rack module) is not vendored. Its `process()` and `getParameters()` are re-written
without Rack in `src/engine.cc`, with these differences, all documented there:
- No CV inputs or attenuverters yet (milestone 2 adds built-in modulation sources for them).
- Filter knobs: upstream adds 5 to the 0..10 knob and clamps at 10, so only the first half of the knob's travel moves
  the filter. Here the whole knob covers that span (440 Hz to 14.1 kHz; low cuts 440 Hz to 13.75 Hz).
- Hold is a latching on/off (upstream: momentary, latching with the "Tog." button).
- Dry, Wet, Size and Pre-delay are smoothed per sample (10 ms), since MPC parameter changes arrive per block.
- Input sensitivity (-18 dB, a context-menu option) is not offered: it equals Wet 18 dB lower.

## Knob art (`art/valley/`)

From the same commit, `res/v2/Med/`: `Rogan1PSMed-bg`, `Rogan1PSMedSmall-bg`, `Rogan1PS{Blue,Green,Red}Med` with
their `-fg` layers, `Rogan1PSWhiteMedSmall` with its `-fg`. Unchanged. The Rogan knob design comes from VCV Rack's
Component Library (graphics by Grayscale), which VCV licenses under CC BY-NC 4.0; Valley's recoloured versions are
distributed in ValleyRackFree. This plugin is free and non-commercial, and credits both.

## Re-vendoring

Copy the files above from a newer upstream commit, update the commit here, run `test/run_tests.sh`, and diff the
behaviour (the tests check tail length, hold, clear, filters and stability).
