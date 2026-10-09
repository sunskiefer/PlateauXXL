# PlateauXXL user guide

PlateauXXL is Valley Audio's **Plateau** reverb from VCV Rack as an insert effect for MPC OS on the Akai Force, with
the patch cables replaced by modulation sources that run inside the plugin: four **Bogaudio LFOs**, **Tidal
Modulator 2** (Mutable Instruments Tides 2), **Random Sampler** (Mutable Instruments Marbles), and two step and two
gate sequencers. On the device it is listed as **PlateauXXL**, manufacturer **ANDREALPHEUS**.

- [Install](#install)
- [The signal path](#the-signal-path)
- [Pages and Q-Links](#pages-and-q-links)
- [PLATEAU: the reverb](#plateau-the-reverb)
- [IN / OUT and SEQ SET](#in--out-and-seq-set)
- [PRESETS](#presets)
- [CV IN: patching the jacks](#cv-in-patching-the-jacks)
- [LFO: Bogaudio LFO](#lfo-bogaudio-lfo)
- [TIDAL: Tidal Modulator 2](#tidal-tidal-modulator-2)
- [RANDOM: Random Sampler](#random-random-sampler)
- [SEQ: step and gate sequencers](#seq-step-and-gate-sequencers)
- [Tempo and transport](#tempo-and-transport)
- [Panic](#panic)
- [Differences from the VCV modules](#differences-from-the-vcv-modules)
- [Troubleshooting](#troubleshooting)

## Install

You need a first-generation Akai Force on MPC OS 3.x with root SSH access (for example through MockbaMod).

1. Download `PlateauXXL-<version>-mpc-armv7.zip` from [Releases](../../../releases) and unzip it on your computer.
2. Copy the folder to the Force and run the installer (save your project first: it stops MPC):

   ```
   scp -r PlateauXXL-<version> root@<device-ip>:/tmp/
   ssh -t root@<device-ip> sh /tmp/PlateauXXL-<version>/install.sh
   ```

3. MPC starts again. Insert **PlateauXXL** (ANDREALPHEUS) on a track, a submix or the master.

Running the installer again upgrades in place; `uninstall.sh` in the same folder removes it. Your presets live in
`/sdcard/PlateauXXL Presets` and are not touched by either.

## The signal path

```
 input --> [ Dry ] -----------------------------------------------------+
   |                                                                     +--> Saturate --> Limiter --> output
   +--> In Low / In High --> Pre-Delay --> diffusers --> TANK --> [ Wet ]-+
                                                         (Size, Diffusion, Decay, Rev Low / High,
                                                          Mod Rate / Shape / Depth, Hold, Clear, Tuned)
```

Every knob in brackets and in the tank, and Hold and Clear, has a CV input (see [CV IN](#cv-in-patching-the-jacks)).

## Pages and Q-Links

| Tab | Sub-pages | What |
| --- | --- | --- |
| PLATEAU | PLATEAU · IN / OUT · SEQ SET · PRESETS | The reverb and Panic; input, limiter and the sequencers' settings; presets |
| CV IN | CV 1 · CV 2 | A source and an attenuverter for each CV input |
| LFO | LFO 1 · LFO 2 · LFO 3 · LFO 4 | Four Bogaudio LFOs |
| TIDAL / RANDOM | TIDAL · RANDOM 1 · RANDOM 2 | Tidal Modulator 2 and Random Sampler |
| SEQ | SEQ 1 · SEQ 2 · GATE 1 · GATE 2 | The step and gate sequencers' steps |

**Q-Links:** every control is on a Q-Link, in reading order: the top row left to right, then the next row, and so on.
Q-Link 1 is the first control at the top left, and 1-8 are knob bank 1, 9-16 bank 2. A page holds 16 Q-Links, so a
screen with more controls has two Q-Link sub-pages that show the same screen: **IN / OUT** (the top row) and
**SEQ SET** (the sequencer rows), and **RANDOM 1** (the top two rows) and **RANDOM 2** (the bottom two).

How a Q-Link turn acts on each kind of control:

| Control | A Q-Link turn |
| --- | --- |
| Knob | Turns it |
| On/off switch | Flips it, once per turn, whichever way you turn |
| Option list | Moves one option at a time, at most one every 0.2 s, so a fast turn doesn't skip options |
| Button (Clear, Panic, Save, Load) | Fires it once per turn to the right |

A tap on the screen always works too: a button fires on every tap, and a tap on an option jumps straight to it.

## PLATEAU: the reverb

The main page, in Plateau's dark panel colours with Valley's knobs (blue for the tank, white for levels, teal for
filters, red for modulation).

| Control | Range | What it does |
| --- | --- | --- |
| **Size** | 0-100 % | The tank's size, from a tiny room to a huge hall; it glides like the VCV knob. With Tuned on it becomes pitched |
| **Diffusion** | 0-100 % | How dense the tank's allpass diffusers are: low is grainy and echoey, high is smooth |
| **Decay** | 0-100 % | How long the tail rings |
| **Hold** | Off / On | Freezes the tank: the tail sustains forever and new input stops going in |
| **Clear** | button | Fades the tank out, empties it and fades back in (4 ms each way) |
| **Tuned** | Off / On | Rescales Size so small tank sizes ring as resonant pitches |
| **Diffuse In** | Off / On | The input diffusers before the tank (Off: the tank gets the raw input, more echoey) |
| **Panic** | button | Everything back to its default (see [Panic](#panic)) |
| **In Low / In High** | 14 Hz-440 Hz / 440 Hz-14.1 kHz | Low cut and high cut of the input, before the tank |
| **Rev Low / Rev High** | 14 Hz-440 Hz / 440 Hz-14.1 kHz | Low cut and high cut inside the tank's feedback loop: the tail gets darker or thinner as it repeats |
| **Rate / Shape / Depth** | | The tank's own four LFOs (Dattorro's chorus): how fast, how their triangle leans (-100 % saw down to +100 % saw up), how far they move the delay lines |

Q-Links: Size, Diffusion, Decay, Hold, Clear, Tuned, Diffuse In, Panic, In Low, In High, Rev Low, Rev High, Rate,
Shape, Depth.

## IN / OUT and SEQ SET

One screen with two Q-Link sub-pages.

**IN / OUT** (the top row):

| Control | Range | What it does |
| --- | --- | --- |
| **Dry** | 0-100 % | The input passed straight through |
| **Wet** | 0-100 % | The reverb |
| **Pre-Delay** | 0-500 ms | Time before the input reaches the tank |
| **Drive** | 0 to +18 dB | Pushes the signal into the limiter |
| **Ceiling** | -12 to 0 dB | The limiter's maximum output level; nothing goes over it |
| **Release** | 10-500 ms | How fast the limiter lets go after a peak |
| **Saturate** | Off / On | Plateau's soft output saturation, before the limiter |

The limiter is RMXXXL's look-ahead brickwall limiter: it sees peaks 1.45 ms ahead and turns the gain down before
they arrive, so it doesn't distort transients, and a final clamp at the Ceiling makes it a true brickwall. At its
defaults (Drive 0 dB, Ceiling -0.3 dB) it only catches overs.

**SEQ SET** (the bottom two rows): for each sequencer, **Rate** (how long a step is, 8 bars to 1/32 with triplets
and dotted values), **Length** (1-16 steps) and **Slew** (step sequencers: 0 % jumps, up to 100 % glides across the
whole step) or **Width** (gate sequencers: how much of a step the gate stays high, 5-100 %).

## PRESETS

16 slots of your own.

1. Tap a **slot** (1-16). The line beside the buttons says **STORED** or **EMPTY**.
2. **SAVE** stores every setting in that slot (it overwrites it without asking). The line says **SAVED**.
3. **LOAD** brings every setting back from that slot. The line says **LOADED**.

A preset holds everything: the reverb, the limiter, the CV routing, every LFO, Tidal, Random and sequencer setting.
The slots are files in `/sdcard/PlateauXXL Presets` (`Preset 01.txt` to `Preset 16.txt`), outside the plugin folder,
so they survive updates and can be copied to another device or backed up. Saving and loading happen off the audio
thread, so they never click. The Force has no keyboard on plugin screens, so slots are numbered, not named.

## CV IN: patching the jacks

Plateau has 15 CV inputs. Each has a **source** (the list above it) and, except Hold and Clear, an **attenuverter**
(the knob below it), coloured as on the module's panel: white for the levels, teal for the filters, blue for the
tank, red for the modulation.

| Input | What the voltage does (Plateau's own CV math) |
| --- | --- |
| Dry, Wet | +1 V adds the whole range (0-100 %) |
| Pre-Delay | Bends the pre-delay time up or down around the knob |
| In Low, In High, Rev Low, Rev High | One octave per volt |
| Size | 0.1 of the range per volt |
| Diffusion | 10 % per volt |
| Decay | About 10 % per volt |
| Mod Rate, Mod Shape | 0.1 of the range per volt |
| Mod Depth | 10 % per volt |
| Hold | Holds the tank while the source is above 0.5 V |
| Clear | Clears the tank each time the source rises above 0.5 V |

**Sources:** Off, LFO 1-4, Tidal 1-4, X1, X2, X3, Y, T1, T2, T3, Seq 1, Seq 2, Gate 1, Gate 2. The voltage is
multiplied by the attenuverter (-100 % to +100 %; a negative value turns the movement upside down) and added to the
knob. Any number of inputs can share one source.

Sub-page **CV 1**: Dry, Wet, Pre-Delay, In Low, In High, Size, Diffusion, Decay (sources are Q-Links 1-8,
attenuverters 9-16). **CV 2**: Rev High, Rev Low, Mod Rate, Mod Shape, Mod Depth, Hold, Clear.

## LFO: Bogaudio LFO

Four copies of Matt Demanett's **Bogaudio LFO**, each on its own sub-page, on Bogaudio's light panel with its knobs.
The module has six output jacks (sine, triangle, ramp up, ramp down, square, stepped); here **Wave** picks which one
goes out, as source LFO 1-4.

| Control | What it does |
| --- | --- |
| **Sync** | Free, or locked to the MPC tempo: 8 bars to 1/32, triplets (T) and dotted (.) values. Default 1 bar |
| **Freq** | With Sync on Free: the rate, as on the module (0.06 Hz to 523 Hz; 2.04 Hz in the middle). Shows "Sync" when locked |
| **Slow** | Divides the free rate by 16, for very slow sweeps |
| **SAM** (sample) | Sample and hold: steps the wave into fewer, longer stairs |
| **PW** | The square's pulse width |
| **SMTH** (smooth) | Rounds the wave's corners (slew), up to a soft sine-like shape |
| **OFF** (offset) | Shifts the wave up or down, ±5 V |
| **SCL** (scale) | The wave's size, 0-100 % of ±5 V |
| **Wave** | Sine, Triangle, Ramp Up, Ramp Down, Square or Stepped (a new random level each cycle) |

When Sync is on, the LFO's phase is the song position: it stays locked to the bar and restarts with the transport.

## TIDAL: Tidal Modulator 2

Emilie Gillet's **Tides 2** (Mutable Instruments), as **Tidal Modulator 2** in VCV's Audible Instruments: a
generator of related slopes, envelopes and cycles with four outputs, sources Tidal 1-4. It's drawn on the Mutable
panel's grey with VCV's Rogan knobs.

| Control | What it does |
| --- | --- |
| **Range** | Low (0.125 Hz), Medium (2 Hz) or High (131 Hz) at Frequency 0 |
| **Ramp** | **AD**: an attack-decay envelope per trigger. **Cycle**: loops. **AR**: attack, hold, release per gate |
| **Output mode** | How the four outputs relate: **Gates** (1 the slope, 2 a unipolar copy, 3 end of attack, 4 end of release), **Amp** (the same slope panned across the four by Shift), **Slope** (four phase-shifted copies), **Freq** (four related frequencies) |
| **Clock** | Free, or a tempo division (8 bars to 1/32). In **Cycle** the cycle locks to it (Frequency then picks a ratio: ×1/16 to ×16); in **AD** and **AR** each division triggers the envelope |
| **Frequency** | ±48 semitones around the range's base rate |
| **Smoothness** | Below 50 %: softer, filtered; above: wavefolded, more complex |
| **Shape** | The curve of the slope, from exponential through linear to logarithmic and wavetable shapes |
| **Slope** | The balance between rise and fall (attack and decay) |
| **Shift / Level** | Output mode's spread across the four outputs (phase, level or ratio). In Gates mode it sets output 1's level, so it starts at 100 % here |

## RANDOM: Random Sampler

Emilie Gillet's **Marbles** (Mutable Instruments), as **Random Sampler** in VCV's Audible Instruments: random gates
and random voltages, controlled for musical randomness. One screen with two Q-Link sub-pages, RANDOM 1 (the top two
rows) and RANDOM 2 (the bottom two).

**t (gates):** sources **T1**, **T2**, **T3** (10 V gates). T2 is the master clock; T1 and T3 are derived from it.

| Control | What it does |
| --- | --- |
| **T Mode** | **Coin**: T1 and T3 toss a coin on each T2 tick. **Cluster**: gates that come in bursts. **Drums**: drum-like patterns |
| **Rate** | The clock rate (2 Hz in the middle, ±60 semitones), when Clock is Free |
| **Bias** | The coin's weight: towards T1 or towards T3 |
| **Jitter** | How much the clock wanders off the grid |
| **T Range** | The clock ×1/4, ×1 or ×4 |
| **Deja Vu** (left switch) | Loops the T random sequence (see Deja Vu and Length below) |

**X (voltages):** sources **X1**, **X2**, **X3** (one new value per clock tick) and **Y** (a slower, smoothed value
at the Y Divider of the clock).

| Control | What it does |
| --- | --- |
| **X Mode** | **Identical**: the three X outputs share Spread / Bias / Steps. **Bump** and **Tilt**: X2 and X3 get more or less of them |
| **Spread** | How wide the random values spread (narrow around the bias to fully random) |
| **Bias** | Where the values centre (low to high) |
| **Steps** | Below 50 %: smoothed, gliding values. Above: quantized to the Scale, more notes as you turn up |
| **X Range** | +2 V, +5 V or ±5 V |
| **Deja Vu** (right switch) | Loops the X random sequence |
| **Scale** | The quantizer's scale: Major, Minor, Pentatonic, Pelog, Bhairav, Shri |
| **Y Divider** | Y's rate as a division of the clock, 1/64 to 1/1 |

**Shared:** **Clock** (Free, or a tempo division that becomes the T clock), **Deja Vu** (knob: how strongly the
looped sequence repeats; at 50 % it never changes, below it drifts, above it jumps around) and **Length** (the loop's
length, 1-16 steps).

## SEQ: step and gate sequencers

**SEQ 1** and **SEQ 2** are 16-step CV sequencers (sources Seq 1 and Seq 2): each step is a knob from -5 V to
+5 V. **GATE 1** and **GATE 2** are 16-step gate sequencers (sources Gate 1 and Gate 2): each step is on or off, and
an "on" step sends 10 V for its Width. Steps 1-8 are the top row (Q-Links 1-8, bank 1), 9-16 the bottom row
(Q-Links 9-16, bank 2). Their Rate, Length and Slew / Width are on **PLATEAU → SEQ SET**.

The sequencers always run on the MPC tempo: the step is wherever the song position is, so they stay in time with the
project, and they start from step 1 when you press play.

## Tempo and transport

PlateauXXL reads the project tempo and transport from MPC. Every tempo-locked source (an LFO with Sync on, Tidal and
Random with Clock set, the sequencers) follows tempo changes at once, and **pressing play restarts them from the top**,
as a reset input would on VCV. With the transport stopped they keep running at the project tempo.

## Panic

**PANIC** (on the PLATEAU page) puts every setting back to its default at once, as a freshly inserted Plateau: the
reverb, the limiter, every CV routing and attenuverter, every LFO, Tidal, Random and sequencer setting and step. It
empties the tank, releases Hold and restarts every source. Only the selected preset slot stays, so you can reload a
preset straight after. The screen and the Q-Links follow within a fraction of a second.

## Differences from the VCV modules

- **Filters:** on the module, only the first half of each filter knob's travel moves the filter (the rest is headroom
  for CV). Here the whole knob covers that same span: 440 Hz to 14.1 kHz (low cuts 440 Hz down to 14 Hz).
- **Hold** latches (on the module it is momentary unless "Tog." is lit); the Hold input holds while high.
- **Input sensitivity** (a context-menu option on the module) is left out: it equals Wet 18 dB lower.
- **Bogaudio LFO:** one output, picked by Wave (the module has six jacks); a tempo Sync is added.
- **Tidal Modulator 2:** the Clock and Trig inputs are the MPC tempo; Shift / Level starts at 100 % (the module's
  50 % mutes output 1 in Gates mode).
- **Random Sampler:** the T clock input is the MPC tempo; External mode (sampling an input voltage) and the X clock
  input are not offered. Y uses X's scale (the module leaves Y on an empty placeholder scale, which reads past its
  table once Steps quantizes).
- **Levels:** VCV's ±10 V audio maps to full scale, as in VCV Rack's own audio interface, so the reverb sits at the
  same level as on the module.

## Troubleshooting

| Problem | What to do |
| --- | --- |
| Plateau isn't in the plugin list | Run the installer again; it registers the plugin in `MPC.settings` and restarts MPC |
| A source doesn't move anything | Check the input's attenuverter isn't at 0 %, and that the source runs (an LFO at Scale 0 %, a sequencer with all steps at 0 V, a gate sequencer with no steps on) |
| The tempo-locked sources don't follow | They use the project tempo; press play once so they lock to the song position |
| The tail never stops | Hold is on, or its input is on a gate that stays high; press Clear or Panic |
| A preset says SAVE FAILED | The card or internal storage is full or read-only. Check `/sdcard/PlateauXXL Presets` over SSH |
| The sound is crushed | Drive is high or Ceiling low on IN / OUT; Panic resets them |
| Something sounds wrong after a lot of changes | Panic, then load your preset |
