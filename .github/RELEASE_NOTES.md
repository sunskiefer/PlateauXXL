**PlateauXXL — Valley Audio's Plateau reverb, patched with its own LFOs, random sources and sequencers, running natively inside MPC OS on the Akai Force.**

Put it on a track, a submix or the master: huge Dattorro plate tails, Hold to freeze the tank, Clear to empty it, Tuned mode for pitched resonances, and every one of the module's 15 CV jacks patched to a modulation source running inside the plugin. Everything sits in one insert slot, with its own touchscreen pages and Q-Links, follows the MPC tempo and saves with your project.

> ✅ **0.4.1 is tested on an Akai Force** (the latest MPC OS with MockbaMod), and passes the offline tests (x86 with ASan + UBSan, and the ARM build under QEMU). New in 0.4.1: the plugin is now called **PlateauXXL** on the device too. New in 0.4.0: every control on a Q-Link in reading order (buttons included), Panic, 16 user presets, an IN / OUT page for the input and the limiter, a page per sequencer, and the Q-Link feel from RMXXXL 1.2.1 (switches flip once per turn, lists can't race, buttons fire on every tap). As with any third-party plugin, save your projects before installing, and report problems under **Issues**. Full list: [CHANGELOG](https://github.com/sunskiefer/PlateauXXL/blob/main/CHANGELOG.md).

## How you play it
1. **Set the space** with Size, Diffusion and Decay, and colour it with the input and tank filters.
2. **Patch the jacks** on **CV IN**: give any of the 15 inputs (Size, Decay, the filters, Wet, Hold, Clear...) a source and an amount.
3. **Shape the movement** with the sources: four **Bogaudio LFOs**, **Tidal Modulator 2** (Mutable Tides 2), **Random Sampler** (Mutable Marbles) and two step and two gate sequencers, all locked to the MPC tempo if you like.
4. **Play it live**: Hold freezes the tank, Clear empties it, both also from a gate; everything is on the Q-Links.
5. **Stay safe**: a look-ahead brickwall limiter keeps the output under your Ceiling, and **Panic** puts everything back to the defaults in one tap.
6. **Save your patches** in 16 preset slots.

## Pages
**PLATEAU** (reverb, Panic · IN / OUT and SEQ SET · PRESETS) · **CV IN** · **LFO** (1-4) · **TIDAL / RANDOM** · **SEQ** (Seq 1-2, Gate 1-2)

## Requirements
- Akai Force (first generation); other Gen1 MPC OS units should work but are untested
- MPC OS 3.x
- Root SSH access (for example MockbaMod)

## Install
Download **PlateauXXL-0.4.1-mpc-armv7.zip** below, unzip it, then:

    scp -r PlateauXXL-0.4.1 root@<device-ip>:/tmp/
    ssh -t root@<device-ip> sh /tmp/PlateauXXL-0.4.1/install.sh

The installer stops MPC (save first), installs the plugin, and starts MPC again. Then insert **PlateauXXL** (manufacturer ANDREALPHEUS) as an insert effect. Presets go in `/sdcard/PlateauXXL Presets`.

Upgrading from 0.4.0 or earlier: those were installed as **Plateau**. Remove that first with the old zip's uninstaller (`ssh -t root@<device-ip> sh /tmp/Plateau-0.4.0/uninstall.sh`), then install this one.

Full guide (every control, patching, the modulation modules, Q-Links, presets, troubleshooting): [docs/USER_GUIDE.md](https://github.com/sunskiefer/PlateauXXL/blob/main/docs/USER_GUIDE.md)

## Credits
- **Plateau:** Dale Johnson (Valley Audio), after Jon Dattorro's 1997 reverb
- **LFO:** Matt Demanett (Bogaudio)
- **Tidal Modulator 2 and Random Sampler:** Emilie Gillet (Mutable Instruments Tides 2 and Marbles), VCV's port in Audible Instruments
- **Brickwall limiter, Panic and presets:** from [RMXXXL](https://github.com/sunskiefer/RMXXXL)
- **Plugin framework, skin tools and installer:** sd88me ([mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins))
- **Knob artwork:** Valley, VCV Component Library (CC BY-NC 4.0), Bogaudio (CC BY-SA 4.0) · **Interface font:** Titillium Web (SIL Open Font License)

Licensed GPL-3.0-or-later. Free, not for sale.
