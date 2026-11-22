# Changelog

## 0.5.0

PlateauXXL for the 64-bit MPCs and for Akai's plugin folder format (MPC OS 3.10.1). Same sound, pages and presets as
0.4.3. **Not tried on a device yet**: checked offline only (2026-11-22), see the README's Status.

- **A 64-bit build** for the 64-bit MPCs (MPC Live III, XL, One G2, Key G2 and later). The 32-bit build (MPC X, Live,
  Live II, One, Key 61 / 37, Force) is the same code.
- **Three zips per release**, made with the current Open MPC catalog release tools (`third_party/open-mpc-release`, from
  open-mpc `main`): `PlateauXXL-0.5.0-mpc-armv7.zip` (Force and the 32-bit MPCs, with `install.sh`, as before),
  `PlateauXXL-0.5.0-mpc-aarch64.zip` (the 64-bit MPCs, with `install.sh`) and `PlateauXXL-0.5.0-mpc-sdk.zip` (one folder
  with `plugin.json` and both binaries for every MPC on MPC OS 3.10.1 or later: copy it into `VST2 Plugins`, no mod needed).
- The installers are the current ones: on MPC OS 3.10.1+ `install.sh` and `uninstall.sh` find the `VST2 Plugins` drive
  themselves when `-t` is left out, and they handle case-insensitive volumes. The catalog check reports 0 warnings (the `'` of
  L'Cronx is shell-escaped in both scripts, checked with `sh -n`).
- The 0.4.3 release stays available as it is. Installs over 0.4.x in place.
- Nothing else changed: pages, Q-Links, presets and the Q-Link / automation fix (#486-#489) are as in 0.4.3.

## 0.4.3
- Built on the latest wrapper and includes the Q-Link and automation fix (mpc-vst-plugins #486-#489): Q-Links now
  move this plugin's parameters on the track's automation screen (MPC no longer rebuilds its parameter screen on every
  write, at most one refresh per 50 ms), and the build keeps every parameter's index fixed (`params.lock`), so saved
  projects, automation and Q-Link maps stay on the right parameters in later versions. Nothing else changed.
- Packaged with the fixed L'Cronx release tools (the ' in the plugin folder name is escaped in install.sh and
  uninstall.sh, and the catalog check no longer flags it). Installs over 0.4.2 in place: no need to uninstall first.

## 0.4.2
- Packed under **L'Cronx**: the plugin folder is now `L'Cronx - VST - PlateauXXL` and MPC lists the maker as L'Cronx. Nothing else
  changed from 0.4.1. If you installed 0.4.1, run its `uninstall.sh` first (copy out anything you saved inside the plugin
  folder), then install 0.4.2. The installer handles the ' in the folder name.

## 0.4.1
- The plugin is called **PlateauXXL** on the device (it was Plateau): plugin list, page titles, the plugin folder
  (`ANDREALPHEUS - VST - PlateauXXL`), `plateauxxl.so`, the presets folder (`/sdcard/PlateauXXL Presets`) and the
  catalog id `plateauxxl`. The uid stays the same. Remove an earlier Plateau install with its own uninstaller first.

## 0.4.0
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
