# PlateauXXL v0.4.1

**Valley Audio's Plateau reverb, patched with its own LFOs, random sources and sequencers, running natively inside
MPC OS on the Akai Force.**

PlateauXXL is a VST2 insert effect for MPC OS's built-in plugin host, made by L'Cronx (shown on the device as
**PlateauXXL** by **ANDREALPHEUS**). It is a port of **Plateau**, Dale Johnson's plate reverb for VCV Rack: huge,
smooth tails from Jon Dattorro's 1997 algorithm, a modulated tank, Hold to freeze it, Clear to empty it, and a Tuned
mode that turns tiny tank sizes into pitched, resonant tones. On VCV Rack, Plateau comes alive through what is patched
into its fifteen CV jacks. A Force has no cables, so PlateauXXL builds the patch in: every jack picks a source, and the
sources are VCV favourites running inside the plugin, each on a page drawn like the module it comes from. Put it on a
track, a submix or the master; everything sits in one insert slot, follows the MPC tempo and saves with your project.

> [!NOTE]
> **Status: 0.4.1, tested on an Akai Force** (MPC OS 3.9.1 with MockbaMod). It also passes the full test suite
> on x86 (ASan + UBSan) and on the ARM build under QEMU. Report anything odd under [Issues](../../issues).

![The PLATEAU page](docs/img/plateau.png)

*The PLATEAU page, rendered offline from the skin (on the device MPC fills in the values).*

## Highlights

- **Plateau, the reverb:** Valley's own Dattorro DSP, unchanged: Size, Diffusion, Decay, input and tank filters,
  pre-delay up to 500 ms, Hold (freeze), Clear, Tuned mode, Diffuse In and the module's output saturation.
- **Every CV jack patched:** each of the module's 15 CV inputs (Dry, Wet, Pre-Delay, In Low / High, Size,
  Diffusion, Decay, Rev Low / High, Mod Rate / Shape / Depth, Hold, Clear) picks a source and has the module's own
  attenuverter, with Plateau's own CV math, as if a cable were plugged in.
- **Bogaudio LFO** (four of them): frequency or locked to the MPC tempo (8 bars to 1/32, triplets, dotted), Slow,
  sample and hold, pulse width, smoothing, offset and scale; sine, triangle, ramps, square or stepped random.
- **Tidal Modulator 2** (Mutable Instruments Tides 2): four related outputs of ramps, envelopes and cycles, in Cycle,
  AD or AR, locked to the tempo or free.
- **Random Sampler** (Mutable Instruments Marbles): random voltages X1-X3 and Y, random gates T1-T3, Deja Vu loops,
  scales, clocked by the tempo or by itself.
- **Step and gate sequencers:** two 16-step CV sequencers (±5 V, slew) and two 16-step gate sequencers, on the MPC
  tempo; press play and they start from step 1.
- **Brickwall limiter:** RMXXXL's look-ahead limiter, Drive in, Ceiling out.
- **Panic:** one tap back to the factory settings, the tank emptied, every source restarted.
- **Presets:** 16 slots of your own, saved on the device.
- **Full Q-Link control:** every control on every page is on a Q-Link, buttons included, in reading order.
- **Skin:** each page drawn like its module: Plateau's dark panel and Valley's knobs, Bogaudio's light LFO panel and
  knobs, the Mutable panels' grey with VCV's Rogan knobs.

| | | |
| --- | --- | --- |
| ![LFO](docs/img/lfo.png) | ![TIDAL](docs/img/tidal.png) | ![RANDOM](docs/img/random.png) |
| ![IN / OUT](docs/img/in-out.png) | ![CV IN](docs/img/cv-in.png) | ![PRESETS](docs/img/presets.png) |

## Patching: what a CV jack does here

On the module each CV input adds a voltage to its knob through a small attenuverter. In PlateauXXL each input has a
**source** (a list) and that same **attenuverter** (a knob, -100 % to +100 %), on the **CV IN** page:

```
 source (LFO 1-4, Tidal 1-4, X1-X3, Y, T1-T3, Seq 1-2, Gate 1-2)  --volts x attenuverter-->  Plateau's CV input
```

The voltages are the modules' own: an LFO swings ±5 V, Random Sampler's X outputs 0 to 5 V (or ±5 V), a gate is
10 V. Plateau's CV math is the module's: Size moves by 0.1 of its range per volt, Dry and Wet by their whole range,
the filters by one octave per volt, and **Hold** and **Clear** react above 0.5 V (Hold holds while the gate is high,
Clear empties the tank on each rising edge).

**Three patches to start with**

| Patch | Set | Result |
| --- | --- | --- |
| Breathing tank | Size source **LFO 1** (1 Bar), Size CV +20 % | The space grows and shrinks with the bar |
| Rhythmic freeze | Hold source **Gate 1**, a few steps on at 1/16 | The tail freezes on the gates and moves on between them |
| Random colour | Rev High source **X1**, Rev High CV +50 %, Random clock 1/8 | The tank's brightness jumps on the eighths |

## Documentation

| Document | What's in it |
| --- | --- |
| [User guide](docs/USER_GUIDE.md) | Install, every page and control, patching, the modulation modules, tempo, Q-Links, Panic, presets, troubleshooting |
| [Changelog](CHANGELOG.md) | What changed in each version |
| [Roadmap](ROADMAP.md) | Bugs and changes planned for the next versions |

## Requirements

- An **Akai Force** (first generation). Other first-generation MPC OS units (MPC Live / Live II, One, X,
  Key 61) should work but are untested.
- **Root SSH access** to the device, for example through MockbaMod. Stock MPC OS can't install third-party
  plugins.
- **MPC OS 3.x.** The skin uses the 3.x format.

## Installation

Download `PlateauXXL-<version>-mpc-armv7.zip` from [Releases](../../releases), or build it (below). Unzip it and
follow the `INSTALL.md` inside. In short:

```
scp -r PlateauXXL-<version> root@<device-ip>:/tmp/
ssh -t root@<device-ip> sh /tmp/PlateauXXL-<version>/install.sh
```

The installer asks for confirmation (`-y` skips it), **stops MPC** (save your project first), copies the plugin
to `/sdcard/Synths/ANDREALPHEUS - VST - PlateauXXL/`, backs up and edits `MPC.settings`, and starts MPC again.
Running it again upgrades in place. Then insert **PlateauXXL** (manufacturer ANDREALPHEUS) on a track, submix or the
master. `uninstall.sh` removes it the same way.

Upgrading from 0.4.0 or earlier: those were installed as **Plateau**. Remove that first with the old zip's
uninstaller (`ssh -t root@<device-ip> sh /tmp/Plateau-0.4.0/uninstall.sh`), then install this one.

Presets go in `/sdcard/PlateauXXL Presets` (created on the first save), outside the plugin folder, so they survive
updates.

## Building

Linux or WSL with Python 3 (+ Pillow, cairosvg), gcc, the DejaVu fonts and [Zig](https://ziglang.org/)
(`pip install ziglang pillow cairosvg`). No Docker.

```
git clone --recursive https://github.com/sunskiefer/PlateauXXL
cd PlateauXXL
./build.sh                # build/arm/plateauxxl.so + the skin -> build/package/
test/run_tests.sh         # the framework's host test + PlateauXXL's own suites, under ASan/UBSan
```

`tools/gen_params.py` is the single source of the parameter list (it writes `params.json` and `src/param_ids.h`).
MPC stores automation by parameter index, so parameters are appended only. `tools/qlinks.py` sets every page's
Q-Links from the layout, and the build checks them.

## Related projects

- [mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins) by sd88me: the framework PlateauXXL is built on, and
  the plugin catalog.
- [RMXXXL](https://github.com/sunskiefer/RMXXXL): the RMX-1000-style remix effect for the Force by the same author,
  where the limiter, Panic and presets come from.
- [ValleyRackFree](https://github.com/ValleyAudio/ValleyRackFree), [BogaudioModules](https://github.com/bogaudio/BogaudioModules)
  and [Audible Instruments](https://github.com/VCVRack/AudibleInstruments): the VCV Rack modules PlateauXXL ports.

## License

PlateauXXL is released under the **GNU GPL v3.0 or later** ([LICENSE](LICENSE)), as the Plateau and Bogaudio code it
includes. Third-party components keep their own licenses (below). The knob artwork includes VCV Component Library
graphics (CC BY-NC 4.0), so PlateauXXL is free and must not be sold.

## Credits

- **Plateau:** Dale Johnson, [Valley Audio](https://github.com/ValleyAudio/ValleyRackFree) (GPL-3.0-or-later), after
  J. Dattorro, "Effect Design Part 1", J. Audio Eng. Soc. 45(9), 1997. The DSP is vendored unchanged in
  `third_party/valley` ([VENDORED.md](third_party/valley/VENDORED.md)); the module's control layer is re-written for
  MPC OS in `src/engine.cc`.
- **LFO:** Matt Demanett, [Bogaudio](https://github.com/bogaudio/BogaudioModules) (GPL-3.0-or-later). The DSP library
  is vendored unchanged in `third_party/bogaudio` ([VENDORED.md](third_party/bogaudio/VENDORED.md)).
- **Tidal Modulator 2 and Random Sampler:** Emilie Gillet, Mutable Instruments Tides 2 and Marbles (MIT), from VCV's
  fork of the eurorack code, vendored unchanged in `third_party/mutable` ([VENDORED.md](third_party/mutable/VENDORED.md));
  the module glue follows VCV [Audible Instruments](https://github.com/VCVRack/AudibleInstruments)
  (GPL-3.0-or-later). One local fix: Random Sampler's Y output read past its scale table (also in the firmware and in
  VCV), now given X's scale.
- **Brickwall limiter, Panic, presets:** from [RMXXXL](https://github.com/sunskiefer/RMXXXL) by L'Cronx
  (GPL-3.0-or-later); the limiter's window minimum is computed faster here, with the same output sample for sample.
- **VST2 wrapper, skin generator, installer:** sd88me ([mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins))
  (MIT), as a submodule in `third_party/mpc-vst-plugins`.
- **Knob artwork** (`art/`): Valley's Rogan knobs (ValleyRackFree), VCV's Rogan knobs (VCV Component Library,
  CC BY-NC 4.0) and Bogaudio's knobs (CC BY-SA 4.0); see [art/README.md](art/README.md).
- **Interface font:** [Titillium Web](https://fonts.google.com/specimen/Titillium+Web), SIL Open Font License 1.1
  (`art/fonts/OFL.txt`).

PlateauXXL is not affiliated with or endorsed by Valley Audio, Bogaudio, Mutable Instruments, VCV or Akai
Professional.
