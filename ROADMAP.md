# Roadmap

## Done (offline): 0.1.0 the reverb, 0.2.0 the modulation
Plateau's DSP with all its controls; a built-in source for each of its 15 CV inputs (Bogaudio LFO x4, Tidal
Modulator 2, Random Sampler, two step and two gate sequencers), tempo-synced; each page in its module's look.

## Next: on the Force
- 0.2.0 seen on a Force (2026-10-08): pages draw as designed; the LFO FREQ knob jumped (fixed in 0.3.0).
- Install 0.3.0: the FREQ knob, the limiter, and playing every source; popups, Q-Links, project save/reload.
- CPU: `tools/bench.sh build/arm/plateau.so <device-ip> -j` (docs/BENCH.md), once with nothing patched and once with
  every source in use (the heaviest case measured offline costs about twice the reverb alone).
- Record the results in `tested.json`, then release.

## Ideas
- Random Sampler's External mode and X clock input (sampling another source).
- A source meter on the CV IN page (what each input receives).
