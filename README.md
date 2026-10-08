# Plateau for MPC OS

**Valley Audio's Plateau reverb, running natively inside MPC OS on the Akai Force.**

Plateau is Dale Johnson's plate reverb for VCV Rack, built on Jon Dattorro's 1997 algorithm: huge, smooth tails,
a modulated tank, Hold to freeze the tank and Clear to empty it, and a Tuned mode that turns tiny tank sizes into
resonant pitched tones. This is a port of it to a VST2 insert effect for MPC OS's built-in plugin host, made by
L'Cronx (shown on the device as **ANDREALPHEUS**), with the VCV panel's dark colours and Valley's own knobs.

> [!NOTE]
> **Status: 0.1.0, not yet tested on a device.** It passes its test suite on x86 (ASan + UBSan) and on the ARM build
> under QEMU. Report anything odd under [Issues](../../issues).

![The PLATEAU page](docs/img/plateau.png)

*The page, rendered offline from the skin (on the device MPC fills in the values).*

## Controls

| Section | Controls | Notes |
| --- | --- | --- |
| Reverb | Size, Diffusion, Decay | Size glides like the VCV knob; Tuned changes its scale |
| Input | Dry, Wet, Pre-Delay | Pre-delay up to 500 ms |
| Tank | Hold, Clear, Tuned, Diffuse In | Hold latches; Clear fades out, empties the tank, fades back in |
| Input filter / Reverb filter | In Low, In High, Rev Low, Rev High | Shown in Hz |
| Modulation | Rate, Shape, Depth | The tank's four LFOs |
| Output | Saturate | The module's soft output saturation |

Q-Links, bank 1: Size, Diffusion, Decay, Wet, Dry, Pre-Delay, Rev Low, Rev High. Bank 2: Rate, Shape, Depth, In Low,
In High, Hold, Clear, Tuned.

## Requirements

- An **Akai Force** (first generation). Other first-generation MPC OS units should work but are untested.
- **Root SSH access** to the device, for example through MockbaMod. Stock MPC OS can't install third-party plugins.
- **MPC OS 3.x.**

## Installation

Download `Plateau-<version>-mpc-armv7.zip` from [Releases](../../releases), or build it (below). Unzip it and follow
the `INSTALL.md` inside. In short:

```
scp -r Plateau-<version> root@<device-ip>:/tmp/
ssh -t root@<device-ip> sh /tmp/Plateau-<version>/install.sh
```

The installer asks for confirmation (`-y` skips it), **stops MPC** (save your project first), copies the plugin to
`/sdcard/Synths/ANDREALPHEUS - VST - Plateau/`, backs up and edits `MPC.settings`, and starts MPC again. Then insert
**Plateau** (manufacturer ANDREALPHEUS) on a track, submix or the master. `uninstall.sh` removes it the same way.

## Building

Linux or WSL with Python 3, gcc and [Zig](https://ziglang.org/) (`pip install ziglang pillow cairosvg`). No Docker.

```
git clone --recursive https://github.com/sunskiefer/Plateau-MPC
cd Plateau-MPC
./build.sh                # build/arm/plateau.so + the skin -> build/package/
test/run_tests.sh         # the framework's host test + Plateau's DSP tests, under ASan/UBSan
```

MPC stores automation by parameter index, so parameters in `params.json` are appended only, never reordered.

## Roadmap

See [ROADMAP.md](ROADMAP.md). Next: built-in modulation for every CV input of the original module.

## License

GNU GPL v3.0 or later ([LICENSE](LICENSE)), as the Plateau DSP it includes. Third-party parts keep their own
licences (below).

## Credits

- **Plateau:** Dale Johnson, [Valley Audio](https://github.com/ValleyAudio/ValleyRackFree) (GPL-3.0-or-later),
  after J. Dattorro, "Effect Design Part 1", J. Audio Eng. Soc. 45(9), 1997. The DSP is vendored unchanged in
  `third_party/valley` ([VENDORED.md](third_party/valley/VENDORED.md)).
- **Knob artwork:** Valley's Rogan knobs from ValleyRackFree, based on VCV Rack's Component Library graphics by
  Grayscale (CC BY-NC 4.0).
- **VST2 wrapper, skin generator, installer:** sd88me ([mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins))
  (MIT), as a submodule in `third_party/mpc-vst-plugins`.
- **Interface font:** [Titillium Web](https://fonts.google.com/specimen/Titillium+Web), SIL Open Font License 1.1
  (`art/fonts/OFL.txt`).

Not affiliated with or endorsed by Valley Audio, VCV or Akai Professional.
