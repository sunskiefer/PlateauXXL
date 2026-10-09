# Changelog

## 0.4.0 (unreleased)
- Every control is on a Q-Link, buttons and switches included, in reading order on every page: the top row left
  to right, then the next row (Q-Link 1, 2, 3, ...). `tools/qlinks.py` sets them from the screen positions and the
  build fails if one is missing or out of order. A screen with more than 16 controls has two Q-Link sub-pages.
- New PLATEAU sub-pages: IN / OUT (Dry, Wet, Pre-Delay, the limiter, Saturate) with SEQ SET (Rate, Length and
  Slew / Width of the four sequencers) on the same screen, and PRESETS. The main page keeps the reverb.
- PANIC (on the main page, like RMXXXL's): every setting back to its default, the tank emptied, the sources
  restarted. The preset slot stays.
- 16 user presets (RMXXXL's): pick a slot, SAVE or LOAD; files in /sdcard/Plateau Presets, written and read off the
  audio thread.
- Q-Link and tap behaviour from RMXXXL 1.2.1's Force test: triggers fire on every tap and once per Q-Link turn to
  the right; on/off switches flip once per turn either way; option lists step at most once per 0.2 s.
- SEQ: one page per sequencer (SEQ 1, SEQ 2, GATE 1, GATE 2), 16 steps each, wider.
- RANDOM: two Q-Link sub-pages on one screen (17 controls).
- Fixed: Random Sampler's Y output read past its scale table once Steps quantized it (also in the firmware and VCV).

## 0.3.0 (unreleased)
- Brickwall limiter at the output, RMXXXL's (Drive in, Ceiling out, Release), in the OUTPUT / LIMITER frame on the
  PLATEAU page. Same output as RMXXXL's sample for sample, at a fraction of the CPU (sliding-window minimum).
- Fixed: the LFO pages' FREQ knob jumped up and down as it turned on the Force (its filmstrip was exactly MPC's
  16384 px image limit; now 15872 px).
- The PLATEAU page's lower row is a little narrower to make room; the name plate moved to the TANK frame.

## 0.2.0 (unreleased)
- Every CV input of the module (15) picks a built-in source and has the module's attenuverter (CV IN page), with
  the module's own CV math.
- Sources: four Bogaudio LFOs, Tidal Modulator 2 (Tides 2), Random Sampler (Marbles), two 16-step CV sequencers and
  two 16-step gate sequencers, following the MPC tempo (play restarts them).
- Pages drawn like the modules: Bogaudio's LFO panel and knobs, the Mutable panels' colours with VCV's Rogan knobs.
- Fixed: option parameters started at the wrong default when they had more than two options.
- Decay now matches the module exactly (the module's knob sits 0.1 above its value).

## 0.1.0 (unreleased)
- First port of Valley Audio's Plateau reverb to MPC OS: all front-panel controls, Hold, Clear, Tuned, Diffuse In,
  output saturation, the VCV dark-panel colours and Valley's Rogan knobs.
