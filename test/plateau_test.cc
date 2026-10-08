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
  std::vector<int16_t> ir = Ir(0.54995, 0.5, 6);
  double early = Rms(ir, kRate / 10, kRate / 10), late = Rms(ir, 5 * kRate, kRate / 2);
  char msg[200];
  snprintf(msg, sizeof msg, "impulse rings (%.1f dBFS at 0.1 s) and decays (%.1f dBFS at 5 s)", Db(early), Db(late));
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

  // ---- display text
  {
    Plug p;
    struct { const char* key; double v; const char* want; } t[] = {
      {"dry", 1, "100 %"}, {"wet", 0.5, "50 %"}, {"pre_delay", 0.25, "250 ms"}, {"in_high", 10, "14.1 kHz"},
      {"in_high", 0, "440 Hz"}, {"in_low", 10, "14 Hz"}, {"in_low", 0, "440 Hz"}, {"decay", 0.1, "0 %"},
      {"decay", 0.9999, "100 %"}, {"mod_shape", 0.5, "+0 %"}, {"mod_depth", 16, "100 %"}, {"diffusion", 10, "100 %"},
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
    Plug p;
    p.Set("mod_depth", 8); p.Set("mod_rate", 0.5);
    std::vector<int16_t> in = Noise(kRate, 0.3), out;
    out.reserve(2 * 30 * kRate);
    clock_t t0 = clock();
    for (int s = 0; s < 30; ++s) p.Run(&in, kRate, &out);
    double sec = (clock() - t0) / (double)CLOCKS_PER_SEC;
    printf("info 30 s of audio in %.2f s on this host (%.0fx real time)\n", sec, 30.0 / sec);
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
