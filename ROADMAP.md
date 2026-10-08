# Roadmap

## Milestone 1: the reverb (0.1.0)
Plateau's DSP with all its front-panel controls, the VCV dark-panel look and Valley's knobs, tests on x86 and ARM.
Next step: install on a Force, check it loads, sounds right and the CPU bench passes (`tools/bench.sh`).

## Milestone 2: a built-in source for every CV input
On VCV Rack, Plateau's character comes from patching LFOs, sequencers and triggers into its jacks. On the Force nothing
can be patched, so each of its 15 CV inputs gets its own control here:

- **Per input:** a source select and the module's own attenuverter (CV depth). Inputs: Dry, Wet, Pre-Delay, In Low,
  In High, Size, Diffusion, Decay, Rev Low, Rev High, Mod Rate, Mod Shape, Mod Depth, Hold, Clear.
- **A shared pool of sources**, synced to the MPC tempo, that any input can pick:
  - LFOs after Bogaudio's LFO (GPL-3.0).
  - Tides (Mutable Instruments, MIT) for ramps, envelopes and complex cycles.
  - Marbles (Mutable Instruments, MIT) for random voltages and random gates.
  - Step and trigger sequencers (written here): 16 steps, per-step value or gate, length, division.
- **A MOD page** in the skin.

The new parameters are appended after the current 18, so projects saved with 0.1.0 keep working.
