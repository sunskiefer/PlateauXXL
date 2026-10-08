# Roadmap

## Done (offline): 0.1.0 the reverb, 0.2.0 the modulation
Plateau's DSP with all its controls; a built-in source for each of its 15 CV inputs (Bogaudio LFO x4, Tidal
Modulator 2, Random Sampler, two step and two gate sequencers), tempo-synced; each page in its module's look.

## Next: on the Force
- Install 0.2.0, check it loads, every page, the popups, Q-Links and project save/reload.
- CPU: `tools/bench.sh build/arm/plateau.so <device-ip> -j` (docs/BENCH.md), once with nothing patched and once with
  every source in use (the heaviest case measured offline costs about twice the reverb alone).
- Record the results in `tested.json`, then release.

## Ideas
- Random Sampler's External mode and X clock input (sampling another source).
- A source meter on the CV IN page (what each input receives).
