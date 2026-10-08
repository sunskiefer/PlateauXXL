# Changelog

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
