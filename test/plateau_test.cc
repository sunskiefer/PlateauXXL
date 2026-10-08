// SPDX-License-Identifier: GPL-3.0-or-later
// Offline DSP tests for Plateau on x86 (run under ASan + UBSan by test/run_tests.sh). They drive the engine the
// way the wrapper does (set_param strings, 128-frame int16 blocks) and check what can be heard:
// a tail that rings and decays, Decay and Size lengthening it, Hold keeping it, Clear silencing it, Dry/Wet,
// the filters, stability at the extremes, and the parameter display text.
// `plateau_test --wav DIR` also writes a few renders to listen to.
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <string>
#include <vector>

extern "C" {
#include "engine.h"
}

namespace {

const int kBlock = 128;
const int kRate = 44100;
int failures = 0;

void Check(bool ok, const char* what) {
  printf("%s %s\n", ok ? "ok  " : "FAIL", what);
  if (!ok) ++failures;
}

struct Plug {
  const mpc_engine_t* e;
  void* inst;
  Plug() : e(mpc_engine()), inst(e->create(NULL)) {}
  ~Plug() { e->destroy(inst); }
  void Set(const char* k, double v) {
    char b[32];
    snprintf(b, sizeof b, "%g", v);
    e->set_param(inst, k, b);
  }
  std::string Text(const char* k) {
    char key[64], b[64] = {0};
    snprintf(key, sizeof key, "%s_display", k);
    e->get_param(inst, key, b, sizeof b);
    return b;
  }
  // Run n frames of input (stereo interleaved, NULL = silence) and append the output.
  void Run(const std::vector<int16_t>* in, int frames, std::vector<int16_t>* out) {
    std::vector<int16_t> zero(2 * kBlock, 0), o(2 * kBlock);
    for (int f = 0; f < frames; f += kBlock) {
      const int16_t* src = zero.data();
      if (in && (size_t)(2 * f + 2 * kBlock) <= in->size()) src = in->data() + 2 * f;
      e->process(inst, src, o.data(), kBlock);
      out->insert(out->end(), o.begin(), o.end());
    }
  }
};

double Rms(const std::vector<int16_t>& v, int from_frame, int frames) {
  double s = 0;
  int n = 0;
  for (int i = from_frame; i < from_frame + frames && 2 * i + 1 < (int)v.size(); ++i, n += 2) {
    double l = v[2 * i] / 32768.0, r = v[2 * i + 1] / 32768.0;
    s += l * l + r * r;
  }
  return n ? sqrt(s / n) : 0;
}

double Db(double x) { return 20.0 * log10(x + 1e-12); }

// Frames until the tail's 50 ms RMS falls 30 dB below its peak (an RT30, in frames).
int Rt30(const std::vector<int16_t>& v) {
  const int win = kRate / 20;
  double peak = 0;
  int frames = (int)v.size() / 2;
  for (int f = 0; f + win <= frames; f += win) peak = fmax(peak, Rms(v, f, win));
  bool past_peak = false;
  for (int f = 0; f + win <= frames; f += win) {
    double r = Rms(v, f, win);
    if (r >= peak) past_peak = true;
    if (past_peak && Db(r) < Db(peak) - 30.0) return f;
  }
  return frames;
}

std::vector<int16_t> Impulse(int frames) {
  std::vector<int16_t> v(2 * frames, 0);
  v[0] = v[1] = 24000;
  return v;
}

std::vector<int16_t> Noise(int frames, double amp) {
  std::vector<int16_t> v(2 * frames);
  uint32_t x = 12345;
  for (size_t i = 0; i < v.size(); ++i) {
    x = x * 1664525u + 1013904223u;
    v[i] = (int16_t)(((int32_t)(x >> 16) - 32768) * amp);
  }
  return v;
}

// Wet-only impulse response with the given settings.
std::vector<int16_t> Ir(double decay, double size, int seconds, const char* extra_key = NULL, double extra = 0) {
  Plug p;
  p.Set("dry", 0);
  p.Set("wet", 1);
  p.Set("decay", decay);
  p.Set("size", size);
  if (extra_key) p.Set(extra_key, extra);
  std::vector<int16_t> in = Impulse(kBlock), out;
  p.Run(&in, kBlock, &out);
  p.Run(NULL, seconds * kRate, &out);
  return out;
}


void WriteWav(const char* path, const std::vector<int16_t>& v) {
  FILE* f = fopen(path, "wb");
  if (!f) return;
  uint32_t data = v.size() * 2, riff = 36 + data, fmt = 16, rate = kRate, byte_rate = kRate * 4;
  uint16_t pcm = 1, ch = 2, align = 4, bits = 16;
  fwrite("RIFF", 1, 4, f); fwrite(&riff, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f); fwrite(&fmt, 4, 1, f);
  fwrite(&pcm, 2, 1, f); fwrite(&ch, 2, 1, f); fwrite(&rate, 4, 1, f); fwrite(&byte_rate, 4, 1, f);
  fwrite(&align, 2, 1, f); fwrite(&bits, 2, 1, f); fwrite("data", 1, 4, f); fwrite(&data, 4, 1, f);
  fwrite(v.data(), 2, v.size(), f);
  fclose(f);
}

}  // namespace

int main(int argc, char** argv) {
  const char* wav_dir = (argc > 2 && !strcmp(argv[1], "--wav")) ? argv[2] : NULL;

  // ---- tail
  std::vector<int16_t> ir = Ir(0.54995, 0.5, 10);
  double early = Rms(ir, kRate / 10, kRate / 10), late = Rms(ir, 9 * kRate, kRate / 2);
  char msg[200];
  snprintf(msg, sizeof msg, "impulse rings (%.1f dBFS at 0.1 s) and decays (%.1f dBFS at 9 s)", Db(early), Db(late));
  Check(early > 1e-3 && late < early * 0.01, msg);
  double l = 0, r = 0, lr = 0;
  for (size_t i = 0; i + 1 < ir.size(); i += 2) { l += ir[i] * (double)ir[i]; r += ir[i + 1] * (double)ir[i + 1]; lr += ir[i] * (double)ir[i + 1]; }
  double corr = lr / sqrt(l * r + 1e-9);
  snprintf(msg, sizeof msg, "stereo tail is decorrelated (L/R correlation %.2f)", corr);
  Check(fabs(corr) < 0.5, msg);

  // ---- decay and size lengthen the tail
  int rt_short = Rt30(Ir(0.3, 0.5, 8)), rt_mid = Rt30(Ir(0.7, 0.5, 8)), rt_long = Rt30(Ir(0.95, 0.5, 20));
  snprintf(msg, sizeof msg, "Decay lengthens the tail (RT30 %.2f s < %.2f s < %.2f s)", rt_short / (double)kRate,
           rt_mid / (double)kRate, rt_long / (double)kRate);
  Check(rt_short < rt_mid && rt_mid < rt_long, msg);
  int rt_small = Rt30(Ir(0.7, 0.2, 8)), rt_big = Rt30(Ir(0.7, 0.9, 20));
  snprintf(msg, sizeof msg, "Size lengthens the tail (RT30 %.2f s < %.2f s)", rt_small / (double)kRate, rt_big / (double)kRate);
  Check(rt_small < rt_big, msg);

  // ---- hold keeps the tail, clear removes it
  {
    Plug p;
    p.Set("dry", 0); p.Set("wet", 1); p.Set("decay", 0.5);
    std::vector<int16_t> in = Noise(kRate / 2, 0.3), out;
    p.Run(&in, kRate / 2, &out);
    p.Set("hold", 1);
    p.Run(&in, kRate / 2, &out);   // input during hold is ignored by the frozen tank... (the input path still adds)
    std::vector<int16_t> held;
    p.Run(NULL, 6 * kRate, &held);
    double a = Rms(held, kRate / 2, kRate / 2), b = Rms(held, 5 * kRate, kRate / 2);
    snprintf(msg, sizeof msg, "Hold sustains the tail (%.1f dB at 0.5 s, %.1f dB at 5 s)", Db(a), Db(b));
    Check(a > 1e-3 && b > a * 0.5, msg);
    p.Set("clear", 1);
    std::vector<int16_t> cleared;
    p.Run(NULL, kRate / 10, &cleared);
    p.Set("clear", 0);
    double c = Rms(cleared, kRate / 20, kRate / 20);
    snprintf(msg, sizeof msg, "Clear empties a held tank (%.1f dBFS 50 ms after the press)", Db(c));
    Check(c < 1e-4, msg);
    p.Set("hold", 0);
    std::vector<int16_t> after_in = Noise(kRate / 2, 0.3), after;
    p.Run(&after_in, kRate / 2, &after);
    Check(Rms(after, kRate / 4, kRate / 4) > 1e-3, "the reverb works again after Clear");
  }

  // ---- dry / wet
  {
    Plug p;
    p.Set("dry", 1); p.Set("wet", 0);
    const int n = 86 * kBlock;   // whole blocks, so every frame compared had its input
    std::vector<int16_t> in = Noise(n, 0.25), out;
    p.Run(&in, n, &out);
    int diff = 0;
    for (size_t i = 2 * kRate / 10; i < out.size() && i < in.size(); ++i) diff = abs(out[i] - in[i]) > diff ? abs(out[i] - in[i]) : diff;
    snprintf(msg, sizeof msg, "Dry 100 %% / Wet 0 %% passes the input through (max difference %d LSB)", diff);
    Check(diff <= 2, msg);
  }

  // ---- filters: In High at 0 darkens the whole reverb; Reverb High at 0 darkens the late tail (the filter sits in
  // the tank loop, so early output taps still carry the first pass)
  {
    auto hf = [](const std::vector<int16_t>& v, size_t from) {   // share of first-difference energy = brightness
      double d2 = 0, e = 0;
      for (size_t i = from + 2; i < v.size(); ++i) { double d = v[i] - (double)v[i - 2]; d2 += d * d; e += v[i] * (double)v[i]; }
      return d2 / (e + 1e-9);
    };
    std::vector<int16_t> open = Ir(0.7, 0.5, 2), in_dark = Ir(0.7, 0.5, 2, "in_high", 0), rv_dark = Ir(0.7, 0.5, 2, "rv_high", 0);
    double a = hf(in_dark, 0) / hf(open, 0);
    snprintf(msg, sizeof msg, "In High at 0 removes highs (brightness %.1f dB of fully open)", 10 * log10(a + 1e-12));
    Check(a < 0.1, msg);
    size_t late = 2 * kRate / 2;
    double b = hf(rv_dark, late) / hf(open, late);
    snprintf(msg, sizeof msg, "Reverb High at 0 darkens the tail after 0.5 s (brightness %.1f dB of fully open)", 10 * log10(b + 1e-12));
    Check(b < 0.3, msg);
  }

  // ---- extremes: largest tank, longest decay, deepest fastest modulation, loud noise, tuned and saturated
  {
    Plug p;
    p.Set("size", 1); p.Set("decay", 0.9999); p.Set("diffusion", 10); p.Set("mod_depth", 16); p.Set("mod_rate", 1);
    p.Set("mod_shape", 0); p.Set("pre_delay", 0.5); p.Set("in_low", 0); p.Set("in_high", 0); p.Set("rv_low", 0);
    std::vector<int16_t> in = Noise(10 * kRate, 0.9), out;
    p.Run(&in, 10 * kRate, &out);
    p.Set("tuned", 1); p.Set("saturate", 1); p.Set("size", 0);
    p.Run(&in, 5 * kRate, &out);
    p.Set("tuned", 0); p.Set("size", 1);   // size jumps back across the whole range
    p.Run(&in, 5 * kRate, &out);
    double tail = Rms(out, (int)out.size() / 2 - kRate, kRate);
    snprintf(msg, sizeof msg, "stable at the extremes for 20 s (last second %.1f dBFS)", Db(tail));
    Check(tail > 1e-4 && tail < 2.0, msg);
  }

  // ---- CV inputs fed by the built-in sources
  {
    // Wet from a gate: Wet knob at 0, its input on Gate 1 (every step on, full width) -> +10 V x 0.5 = full wet
    Plug p;
    p.Set("dry", 0); p.Set("wet", 0);
    for (int s = 1; s <= 16; ++s) { char k[16]; snprintf(k, sizeof k, "gt1_g%d", s); p.Set(k, 1); }
    p.Set("gt1_width", 1.0);
    std::vector<int16_t> in = Noise(kRate / 2, 0.3), off, on;
    p.Run(&in, kRate / 2, &off);
    p.Set("wet_src", 18);   // Gate 1
    p.Run(&in, kRate / 2, &on);
    snprintf(msg, sizeof msg, "Wet input on Gate 1 opens the wet signal (%.1f dB off, %.1f dB on)",
             Db(Rms(off, kRate / 4, kRate / 4)), Db(Rms(on, kRate / 4, kRate / 4)));
    Check(Rms(off, kRate / 4, kRate / 4) < 1e-4 && Rms(on, kRate / 4, kRate / 4) > 1e-3, msg);
  }
  {
    // Hold from a gate: Hold input on Gate 1 (all on) sustains a tail the knob alone would let decay
    Plug p;
    p.Set("dry", 0); p.Set("wet", 1); p.Set("decay", 0.3);
    for (int s = 1; s <= 16; ++s) { char k[16]; snprintf(k, sizeof k, "gt1_g%d", s); p.Set(k, 1); }
    p.Set("gt1_width", 1.0);
    p.Set("hold_src", 18);
    std::vector<int16_t> in = Noise(kRate / 2, 0.3), out, tail;
    p.Run(&in, kRate / 2, &out);
    p.Run(NULL, 4 * kRate, &tail);
    double held = Rms(tail, 3 * kRate, kRate / 2);
    p.Set("hold_src", 0);
    std::vector<int16_t> after;
    p.Run(NULL, 4 * kRate, &after);
    double released = Rms(after, 3 * kRate, kRate / 2);
    snprintf(msg, sizeof msg, "Hold input on a gate holds the tank, and lets go (%.1f dB held, %.1f dB after)", Db(held), Db(released));
    Check(held > 1e-3 && released < held * 0.01, msg);
  }
  {
    // Clear from a gate: a held tank, then the Clear input on Gate 1 (one 1/4 step, Length 1: a gate each beat)
    Plug p;
    p.Set("dry", 0); p.Set("wet", 1); p.Set("hold", 1);
    p.Set("gt1_g1", 1); p.Set("gt1_len", 1); p.Set("gt1_div", 5);   // "1/4"
    std::vector<int16_t> in = Noise(kRate / 4, 0.3), out, held, cleared;
    p.Run(&in, kRate / 4, &out);
    p.Run(NULL, kRate / 2, &held);
    p.Set("clear_src", 18);
    p.Run(NULL, kRate / 2, &cleared);   // at 120 BPM a gate comes within 0.5 s
    snprintf(msg, sizeof msg, "Clear input on a gate empties a held tank (%.1f dB held, %.1f dB after the gate)",
             Db(Rms(held, kRate / 4, kRate / 4)), Db(Rms(cleared, kRate / 2 - kRate / 20, kRate / 20)));
    Check(Rms(held, kRate / 4, kRate / 4) > 1e-3 && Rms(cleared, kRate / 2 - kRate / 20, kRate / 20) < 1e-4, msg);
  }
  {
    // Size from LFO 1 (synced 1/4): the tail differs from an unmodulated one, and stays sane
    std::vector<int16_t> plain = Ir(0.8, 0.5, 4), moved;
    Plug p;
    p.Set("dry", 0); p.Set("wet", 1); p.Set("decay", 0.8); p.Set("size", 0.5);
    p.Set("size_src", 1); p.Set("size_cv", 1); p.Set("lfo1_sync", 6);
    std::vector<int16_t> imp = Impulse(kBlock);
    p.Run(&imp, kBlock, &moved);
    p.Run(NULL, 4 * kRate, &moved);
    double diff = 0;
    for (size_t i = 0; i < plain.size() && i < moved.size(); ++i) diff += fabs(plain[i] - (double)moved[i]);
    double tail = Rms(moved, 3 * kRate, kRate / 2);
    snprintf(msg, sizeof msg, "Size input on LFO 1 moves the tank (mean difference %.0f LSB, tail %.1f dBFS)",
             diff / plain.size(), Db(tail));
    Check(diff / plain.size() > 5 && tail < 0.5, msg);
  }
  {
    // Transport and tempo: Dry knob 0, its input on Seq 1 (step 1 +5 V, the rest -5 V): only the first 1/16 after
    // play passes the dry signal; at 60 BPM a 1/16 lasts 0.25 s
    Plug p;
    p.Set("dry", 0); p.Set("wet", 0); p.Set("dry_cv", 1);
    p.Set("sq1_s1", 5);
    for (int s = 2; s <= 16; ++s) { char k[16]; snprintf(k, sizeof k, "sq1_s%d", s); p.Set(k, -5); }
    p.Set("dry_src", 16);
    p.e->set_param(p.inst, "lfo_bpm", "60");
    p.e->set_param(p.inst, "transport", "1");
    const int n = 86 * kBlock * 2;
    std::vector<int16_t> in = Noise(n, 0.25), out;
    p.Run(&in, n, &out);
    double first = Rms(out, kRate / 20, kRate / 10), later = Rms(out, kRate / 2, kRate / 10);
    snprintf(msg, sizeof msg, "Seq 1 restarts with the transport and runs at the MPC tempo (%.1f dB in step 1, %.1f dB in step 3)",
             Db(first), Db(later));
    Check(first > 0.05 && later < 1e-4, msg);
  }

  // ---- display text
  {
    Plug p;
    struct { const char* key; double v; const char* want; } t[] = {
      {"dry", 1, "100 %"}, {"wet", 0.5, "50 %"}, {"pre_delay", 0.25, "250 ms"}, {"in_high", 10, "14.1 kHz"},
      {"in_high", 0, "440 Hz"}, {"in_low", 10, "14 Hz"}, {"in_low", 0, "440 Hz"}, {"decay", 0.1, "0 %"},
      {"decay", 0.9999, "100 %"}, {"mod_shape", 0.5, "+0 %"}, {"mod_depth", 16, "100 %"}, {"diffusion", 10, "100 %"},
      {"size_cv", -0.5, "-50 %"}, {"lfo1_offset", 1, "+5.00 V"}, {"sq2_s16", -2.5, "-2.50 V"}, {"td_freq", 12, "+12.0 st"},
      {"lfo2_freq", 0, "Sync"},
    };
    for (size_t i = 0; i < sizeof t / sizeof t[0]; ++i) {
      p.Set(t[i].key, t[i].v);
      std::string got = p.Text(t[i].key);
      snprintf(msg, sizeof msg, "%s = %g shows \"%s\" (want \"%s\")", t[i].key, t[i].v, got.c_str(), t[i].want);
      Check(got == t[i].want, msg);
    }
  }

  // ---- speed (host only; the device check is the framework's bench)
  {
    for (int busy = 0; busy < 2; ++busy) {
      Plug p;
      p.Set("mod_depth", 8); p.Set("mod_rate", 0.5);
      if (busy) {   // every source running: each CV input on a different one
        const char* in_keys[] = { "dry", "wet", "pre_delay", "in_low", "in_high", "size", "diffusion", "decay",
                                  "rv_high", "rv_low", "mod_rate", "mod_shape", "mod_depth", "hold", "clear" };
        const int srcs[] = { 1, 2, 3, 4, 5, 6, 9, 12, 7, 8, 10, 11, 16, 13, 19 };
        for (int i = 0; i < 15; ++i) { char k[32]; snprintf(k, sizeof k, "%s_src", in_keys[i]); p.Set(k, srcs[i]); }
        p.Set("decay_cv", 0.05); p.Set("size_cv", 0.1);
      }
      std::vector<int16_t> in = Noise(kRate, 0.3), out;
      out.reserve(2 * 30 * kRate);
      clock_t t0 = clock();
      for (int s = 0; s < 30; ++s) p.Run(&in, kRate, &out);
      double sec = (clock() - t0) / (double)CLOCKS_PER_SEC;
      printf("info %s: 30 s of audio in %.2f s on this host (%.0fx real time)\n",
             busy ? "every source in use" : "reverb only", sec, 30.0 / sec);
    }
  }

  if (wav_dir) {
    char path[512];
    snprintf(path, sizeof path, "%s/ir_default.wav", wav_dir); WriteWav(path, Ir(0.54995, 0.5, 6));
    snprintf(path, sizeof path, "%s/ir_huge.wav", wav_dir); WriteWav(path, Ir(0.95, 1.0, 15, "mod_depth", 8));
    printf("info wrote renders to %s\n", wav_dir);
  }

  printf(failures ? "FAILED (%d)\n" : "PASSED\n", failures);
  return failures ? 1 : 0;
}
