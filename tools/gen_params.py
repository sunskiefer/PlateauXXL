#!/usr/bin/env python3
"""The single source of Plateau's parameter list: writes params.json (for mpc-vst-plugins' gen_vst.py) and
src/param_ids.h (P_<KEY> index constants and the option lists the engine needs).

    python3 tools/gen_params.py

MPC stores automation and saved values by parameter INDEX, so this list is append-only: never reorder, rename a key or
remove a parameter that has shipped. 0.1.0 shipped the first 18 (the reverb).
"""
import json
import os

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")

# Modulation sources a CV input can pick (index = the "<input>_src" option). Volts, as on VCV Rack.
SOURCES = ["Off", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "Tidal 1", "Tidal 2", "Tidal 3", "Tidal 4",
           "X1", "X2", "X3", "Y", "T1", "T2", "T3", "Seq 1", "Seq 2", "Gate 1", "Gate 2"]
# Tempo divisions: name, length in beats (quarter notes).
DIVISIONS = [("8 Bars", 32), ("4 Bars", 16), ("2 Bars", 8), ("1 Bar", 4), ("1/2", 2), ("1/4", 1), ("1/8", 0.5),
             ("1/16", 0.25), ("1/32", 0.125), ("1/2 T", 4 / 3.0), ("1/4 T", 2 / 3.0), ("1/8 T", 1 / 3.0),
             ("1/16 T", 1 / 6.0), ("1/2 .", 3), ("1/4 .", 1.5), ("1/8 .", 0.75)]
SYNC = ["Free"] + [d[0] for d in DIVISIONS]          # LFO, Tidal, Random: free running or locked to the MPC tempo
STEP_DIVS = [d[0] for d in DIVISIONS]                 # sequencers: always on the tempo
WAVES = ["Sine", "Triangle", "Ramp Up", "Ramp Down", "Square", "Stepped"]
OFF_ON = ["Off", "On"]


def knob(key, name, lo, hi, default, **kw):
    p = {"key": key, "name": name, "min": lo, "max": hi, "default": default, "dynamic_display": True}
    p.update(kw)
    return p


def opts(key, name, options, default=0, **kw):
    p = {"key": key, "name": name, "options": options, "default": default}
    p.update(kw)
    return p


P = []
# ---- 0.1.0: the reverb (indices 0-17) -----------------------------------------------------------------------------
P += [
    knob("dry", "Dry", 0.0, 1.0, 1.0), knob("wet", "Wet", 0.0, 1.0, 0.5), knob("pre_delay", "Pre-Delay", 0.0, 0.5, 0.0),
    knob("in_low", "In Low", 0.0, 10.0, 10.0), knob("in_high", "In High", 0.0, 10.0, 10.0),
    knob("size", "Size", 0.0, 1.0, 0.5), knob("diffusion", "Diffusion", 0.0, 10.0, 10.0),
    knob("decay", "Decay", 0.1, 0.9999, 0.54995), knob("rv_low", "Reverb Low", 0.0, 10.0, 10.0),
    knob("rv_high", "Reverb High", 0.0, 10.0, 10.0), knob("mod_rate", "Mod Rate", 0.0, 1.0, 0.0),
    knob("mod_shape", "Mod Shape", 0.0, 1.0, 0.5), knob("mod_depth", "Mod Depth", 0.0, 16.0, 0.5),
    opts("hold", "Hold", OFF_ON), {"key": "clear", "name": "Clear", "min": 0.0, "max": 1.0, "momentary": True,
                                   "type": "trigger", "default": 0.0},
    opts("tuned", "Tuned", OFF_ON), opts("diffuse_in", "Diffuse In", OFF_ON, 1), opts("saturate", "Saturate", OFF_ON),
]
assert len(P) == 18

# ---- 0.2.0: a source and an attenuverter for every CV input of the module ----------------------------------------
# (key of the knob it modulates, label). Hold and Clear are gates: no attenuverter on the module, none here.
CV_INPUTS = [("dry", "Dry"), ("wet", "Wet"), ("pre_delay", "Pre-Delay"), ("in_low", "In Low"), ("in_high", "In High"),
             ("size", "Size"), ("diffusion", "Diffusion"), ("decay", "Decay"), ("rv_high", "Rev High"),
             ("rv_low", "Rev Low"), ("mod_rate", "Mod Rate"), ("mod_shape", "Mod Shape"), ("mod_depth", "Mod Depth"),
             ("hold", "Hold"), ("clear", "Clear")]
GATE_INPUTS = ("hold", "clear")
for key, label in CV_INPUTS:
    P.append(opts(key + "_src", label + " Src", SOURCES))
    if key not in GATE_INPUTS:
        P.append(knob(key + "_cv", label + " CV", -1.0, 1.0, 0.5))

# LFO 1-4: Bogaudio LFO (one output, picked by Wave), plus tempo sync
for n in range(1, 5):
    k = "lfo%d_" % n
    P += [opts(k + "wave", "LFO%d Wave" % n, WAVES), opts(k + "sync", "LFO%d Sync" % n, SYNC, SYNC.index("1 Bar")),
          knob(k + "freq", "LFO%d Freq" % n, -5.0, 8.0, 0.0), opts(k + "slow", "LFO%d Slow" % n, OFF_ON),
          knob(k + "sample", "LFO%d Sample" % n, 0.0, 1.0, 0.0), knob(k + "pw", "LFO%d PW" % n, -1.0, 1.0, 0.0),
          knob(k + "smooth", "LFO%d Smooth" % n, 0.0, 1.0, 0.0), knob(k + "offset", "LFO%d Offset" % n, -1.0, 1.0, 0.0),
          knob(k + "scale", "LFO%d Scale" % n, 0.0, 1.0, 1.0)]

# Tidal Modulator 2 (Mutable Instruments Tides 2)
P += [knob("td_freq", "Tidal Freq", -48.0, 48.0, 0.0), knob("td_shape", "Tidal Shape", 0.0, 1.0, 0.5),
      knob("td_slope", "Tidal Slope", 0.0, 1.0, 0.5), knob("td_smooth", "Tidal Smooth", 0.0, 1.0, 0.5),
      knob("td_shift", "Tidal Shift", 0.0, 1.0, 1.0),   # module: 0.5, which mutes output 1 in Gates mode
      opts("td_range", "Tidal Range", ["Low", "Medium", "High"], 1),
      opts("td_output", "Tidal Output", ["Gates", "Amplitude", "Slope/Phase", "Frequency"], 0),
      opts("td_ramp", "Tidal Ramp", ["AD", "Cycle", "AR"], 1),
      opts("td_sync", "Tidal Sync", SYNC, 0)]

# Random Sampler (Mutable Instruments Marbles)
P += [knob("mb_t_rate", "T Rate", -1.0, 1.0, 0.0), knob("mb_t_bias", "T Bias", 0.0, 1.0, 0.5),
      knob("mb_t_jitter", "T Jitter", 0.0, 1.0, 0.0),
      opts("mb_t_mode", "T Mode", ["Coin Toss", "Clusters", "Drums"], 0),
      opts("mb_t_range", "T Range", ["x1/4", "x1", "x4"], 1),
      knob("mb_deja_vu", "Deja Vu", 0.0, 1.0, 0.5), knob("mb_length", "Length", 0.0, 1.0, 0.0),
      opts("mb_t_dv", "T Deja Vu", OFF_ON), opts("mb_x_dv", "X Deja Vu", OFF_ON),
      knob("mb_x_spread", "X Spread", 0.0, 1.0, 0.5), knob("mb_x_bias", "X Bias", 0.0, 1.0, 0.5),
      knob("mb_x_steps", "X Steps", 0.0, 1.0, 0.5),
      opts("mb_x_mode", "X Mode", ["Identical", "Bump", "Tilt"], 0),
      opts("mb_x_range", "X Range", ["+2 V", "+5 V", "+/-5 V"], 1),
      opts("mb_x_scale", "X Scale", ["Major", "Minor", "Pentatonic", "Pelog", "Bhairav", "Shri"], 0),
      opts("mb_y_div", "Y Divider", ["1/64", "1/48", "1/32", "1/24", "1/16", "1/12", "1/8", "1/6", "1/4", "1/3", "1/2",
                                     "1/1"], 8),
      opts("mb_sync", "Random Sync", SYNC, 0)]

# Step sequencers 1-2 (CV, +/-5 V) and gate sequencers 1-2
for n in range(1, 3):
    k = "sq%d_" % n
    P += [knob(k + "s%d" % s, "Seq%d Step %d" % (n, s), -5.0, 5.0, 0.0) for s in range(1, 17)]
    P += [knob(k + "len", "Seq%d Length" % n, 1, 16, 16, display="int"),
          opts(k + "div", "Seq%d Rate" % n, STEP_DIVS, STEP_DIVS.index("1/16")),
          knob(k + "slew", "Seq%d Slew" % n, 0.0, 1.0, 0.0)]
for n in range(1, 3):
    k = "gt%d_" % n
    P += [opts(k + "g%d" % s, "Gate%d Step %d" % (n, s), OFF_ON) for s in range(1, 17)]
    P += [knob(k + "len", "Gate%d Length" % n, 1, 16, 16, display="int"),
          opts(k + "div", "Gate%d Rate" % n, STEP_DIVS, STEP_DIVS.index("1/16")),
          knob(k + "width", "Gate%d Width" % n, 0.05, 1.0, 0.5)]


# ---- 0.3.0: brickwall limiter at the output, as in RMXXXL (Drive in, Ceiling out) ------------------------------------
P += [knob("lim_drive", "Limiter Drive", 0.0, 18.0, 0.0, unit="dB"),
      knob("lim_ceiling", "Ceiling", -12.0, 0.0, -0.3, unit="dB"),
      knob("lim_release", "Limiter Release", 10.0, 500.0, 80.0, unit="ms")]


def c_ident(key):
    return "P_" + key.upper()


def main():
    keys = [p["key"] for p in P]
    assert len(keys) == len(set(keys)), "duplicate key"
    with open(os.path.join(ROOT, "params.json"), "w") as f:
        json.dump({"name": "Plateau", "params": P}, f, indent=1)
        f.write("\n")
    lines = ["// generated by tools/gen_params.py: do not edit", "#pragma once", "", "enum ParamId {"]
    lines += ["  %s = %d," % (c_ident(k), i) for i, k in enumerate(keys)]
    lines += ["  P_COUNT = %d" % len(keys), "};", ""]
    lines.append("static const int kNumSources = %d;" % len(SOURCES))
    lines.append("enum SourceId { %s };" % ", ".join("SRC_%d" % i for i in range(len(SOURCES))))
    lines.append("static const float kDivisionBeats[%d] = { %s };" % (
        len(DIVISIONS), ", ".join(repr(float(d[1])) + "f" for d in DIVISIONS)))
    lines.append("static const int kNumCvInputs = %d;" % len(CV_INPUTS))
    lines.append("// per CV input: the knob it modulates, its source and attenuverter (-1: none)")
    lines.append("static const int kCvInput[%d][3] = {" % len(CV_INPUTS))
    for key, _ in CV_INPUTS:
        cv = c_ident(key + "_cv") if key not in GATE_INPUTS else "-1"
        lines.append("  { %s, %s, %s }," % (c_ident(key), c_ident(key + "_src"), cv))
    lines.append("};")
    with open(os.path.join(ROOT, "src", "param_ids.h"), "w") as f:
        f.write("\n".join(lines) + "\n")
    print("gen_params: %d parameters" % len(P))


if __name__ == "__main__":
    main()
