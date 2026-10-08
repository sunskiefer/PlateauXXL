// SPDX-License-Identifier: GPL-3.0-or-later
// Plateau for MPC OS: Valley Audio's Plateau reverb (Dale Johnson, after Dattorro 1997) as an MPC OS VST2 insert
// effect. The reverb itself is Valley's Dattorro class, vendored unchanged (third_party/valley, see VENDORED.md);
// this file is the Plateau module's control layer (Plateau.cpp, process() and getParameters()) without VCV Rack,
// driving it from MPC parameters through the mpc-vst-plugins engine interface (wrapper/engine.h).
//
// CV inputs: on VCV, Plateau's character comes from what is patched into its jacks. Here each CV input picks one of
// the built-in sources (sources.h) and has the module's own attenuverter; the CV math is getParameters()'s, in volts.
//
// Levels: VCV Rack's audio interface maps +-10 V to full scale, so a sample x is 10x volts. Plateau feeds the reverb
// with in_V * 0.1 and adds rev * wet * 10 volts back, so in full-scale units out = x * dry + rev(x) * wet, the same
// as the module. Output saturation works on volts, as in the module. Last comes RMXXXL's brickwall limiter.
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <new>

#include "Plateau/Dattorro.hpp"
#include "dsp/shaping/NonLinear.hpp"
#include "denormals.h"
#include "param_ids.h"
#include "sources.h"
#include "limiter.h"

extern "C" {
#include "engine.h"
#include "params.h"
}

namespace {

using plateau::kMaxBlock;

const double kSampleRate = 44100.0;
const int kCtl = 8;   // CV is applied every 8 frames (5.5 kHz); the sources themselves run at the audio rate

// Plateau.hpp constants.
const float kSizeMin = 0.0025f;
const float kSizeMax = 4.0f;
const float kDecayMax = 0.9999f;
const float kModDepthMax = 16.0f;
const float kModShapeMin = 0.001f;
const float kModShapeMax = 0.999f;
const float kSaturatorPreGain = 0.111f;
const float kSaturatorDrive = 0.95f;
const float kSaturatorPostGain = 9.999f;
const float kPreDelayNormSens = 0.1f;
const float kVolts = 10.0f;          // full scale = 10 V (VCV Rack's audio interface)
const float kClearFade = 0.004f;     // Plateau's clear envelope: 4 ms out, clear, 4 ms in
const float kGateThreshold = 0.5f;   // Plateau's Hold and Clear inputs: high above 0.5 V

inline float Clamp(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

static_assert(P_COUNT <= NPARAMS, "params.h is out of date: run tools/gen_params.py, then build");

struct Instance {
  Dattorro reverb;
  plateau::Sources sources;
  plateau::Limiter limiter;
  float s_drive, s_ceiling;   // limiter settings, smoothed per block
  float param[P_COUNT];
  // tempo
  float bpm;
  double beat;
  bool restart_pending;
  // per-sample smoothed controls (one-pole): Dry, Wet, Size (as tank scale) and Pre-delay
  float s_dry, s_wet, s_size, s_pre;
  bool smooth_init;
  // freeze / clear
  bool frozen;
  bool clear_cv_high;
  int clear_pending;     // a Clear press (or rising Clear CV) waiting to start
  int clear_stage;       // 0 idle, 1 fading out, 2 fading in
  float env;             // input and wet gain during a clear
  // last block of source voltages (rendered at the start of each block)
  float src[kNumSources][kMaxBlock];
  float out_l[kMaxBlock], out_r[kMaxBlock];   // the block before the limiter

  Instance() : reverb(kSampleRate, 16.0, kSizeMax) {}
};

// ------------------------------------------------------------------------------------------------ mapping
// Plateau.cpp getParameters(). cv: the input's volts times its attenuverter (0 when nothing is picked).
//
// Filters: the module adds 5 to the 0..10 knob and clamps at 10, so only the first half of each filter knob's travel
// moves the filter (the other half is headroom for CV). Here the whole knob covers that same span: the knob counts
// as knob / 2, so pitch = clamp(cv + 5 + knob / 2), 440 Hz at 0 to 14.1 kHz at 10 with no CV.
float FilterPitch(float knob, float cv) { return Clamp(cv + 5.0f + 0.5f * Clamp(knob, 0.0f, 10.0f), 0.0f, 10.0f); }
float PitchHz(float pitch) { return 440.0f * powf(2.0f, pitch - 5.0f); }
float LowCutPitch(float knob, float cv) { return 10.0f - FilterPitch(knob, cv); }

float SizeScale(float knob, float cv, bool tuned) {
  float size = cv * 0.1f + knob;
  if (tuned) return Clamp(kSizeMin * powf(2.0f, size * 5.0f), kSizeMin, 2.5f);
  size *= size;
  return Clamp(0.01f + size * (kSizeMax - 0.01f), 0.01f, kSizeMax);
}

// The module adds rescale(cv, 0, 10, 0.1, 0.999) to the knob, so the knob alone sits 0.1 higher than its own value.
float DecayGain(float knob, float cv) {
  float d = Clamp(knob + 0.1f + cv * (0.999f - 0.1f) / 10.0f, 0.1f, kDecayMax);
  d = 1.0f - d;
  return 1.0f - d * d;
}

float ModSpeed(float knob, float cv) {
  float m = Clamp(cv * 0.1f + knob, 0.0f, 1.0f);
  return m * m * 99.0f + 1.0f;
}

float ModShape(float knob, float cv) {
  float s = cv * 0.1f + knob;
  return Clamp(kModShapeMin + s * (kModShapeMax - kModShapeMin), kModShapeMin, kModShapeMax);
}

float ModDepth(float knob, float cv) { return Clamp(cv * kModDepthMax / 10.0f + knob, 0.0f, kModDepthMax); }

float PreDelay(float knob, float cv) { return Clamp(knob + 0.5f * (powf(2.0f, cv * kPreDelayNormSens) - 1.0f), 0.0f, 1.0f); }

// params.h gives every default normalised to 0..1 (an option list's as index / (count - 1); gen_vst.py), and an option
// list's min/max as 0: the engine works in parameter units (option indices) throughout.
float MinOf(int p) { return PARAMS[p].nopts > 0 ? 0.0f : PARAMS[p].min; }
float MaxOf(int p) { return PARAMS[p].nopts > 0 ? static_cast<float>(PARAMS[p].nopts - 1) : PARAMS[p].max; }
float DefaultValue(int p) {
  if (PARAMS[p].nopts > 0) return floorf(PARAMS[p].def * (PARAMS[p].nopts - 1) + 0.5f);
  return PARAMS[p].min + PARAMS[p].def * (PARAMS[p].max - PARAMS[p].min);
}

int Option(const Instance* s, int p) { return static_cast<int>(s->param[p] + 0.5f); }

// Volts reaching CV input `input` at frame i (source x attenuverter; Hold and Clear have no attenuverter).
float InputCv(const Instance* s, int input, int i) {
  int src = Option(s, kCvInput[input][1]);
  if (src <= 0 || src >= kNumSources) return 0.0f;
  float v = s->src[src][i];
  int att = kCvInput[input][2];
  return att >= 0 ? v * s->param[att] : v;
}

enum {   // index into kCvInput (gen_params.py CV_INPUTS order)
  CV_DRY, CV_WET, CV_PRE_DELAY, CV_IN_LOW, CV_IN_HIGH, CV_SIZE, CV_DIFFUSION, CV_DECAY, CV_RV_HIGH, CV_RV_LOW,
  CV_MOD_RATE, CV_MOD_SHAPE, CV_MOD_DEPTH, CV_HOLD, CV_CLEAR
};

// Settings that cost a pow() or a coefficient update go to the reverb once per kCtl frames, at frame i.
void PushControlSettings(Instance* s, int i) {
  Dattorro& r = s->reverb;
  const float* p = s->param;
  r.setInputFilterLowCutoffPitch(LowCutPitch(p[P_IN_LOW], InputCv(s, CV_IN_LOW, i)));
  r.setInputFilterHighCutoffPitch(FilterPitch(p[P_IN_HIGH], InputCv(s, CV_IN_HIGH, i)));
  r.enableInputDiffusion(Option(s, P_DIFFUSE_IN) != 0);
  r.setDecay(DecayGain(p[P_DECAY], InputCv(s, CV_DECAY, i)));
  r.setTankDiffusion(Clamp(InputCv(s, CV_DIFFUSION, i) + p[P_DIFFUSION], 0.0f, 10.0f));
  r.setTankFilterLowCutFrequency(LowCutPitch(p[P_RV_LOW], InputCv(s, CV_RV_LOW, i)));
  r.setTankFilterHighCutFrequency(FilterPitch(p[P_RV_HIGH], InputCv(s, CV_RV_HIGH, i)));
  r.setTankModSpeed(ModSpeed(p[P_MOD_RATE], InputCv(s, CV_MOD_RATE, i)));
  r.setTankModDepth(ModDepth(p[P_MOD_DEPTH], InputCv(s, CV_MOD_DEPTH, i)));
  r.setTankModShape(ModShape(p[P_MOD_SHAPE], InputCv(s, CV_MOD_SHAPE, i)));
  // Hold: the button latches; the Hold input holds while high, as the module's (non-toggle) Hold input does.
  bool freeze = Option(s, P_HOLD) != 0 || InputCv(s, CV_HOLD, i) > kGateThreshold;
  if (freeze != s->frozen) {
    s->frozen = freeze;
    r.freeze(freeze);
  }
  // Clear input: a rising edge clears, like the module's.
  bool clear_high = InputCv(s, CV_CLEAR, i) > kGateThreshold;
  if (clear_high && !s->clear_cv_high) s->clear_pending = 1;
  s->clear_cv_high = clear_high;
}

// ------------------------------------------------------------------------------------------------ engine interface
void* Create(const char*) {
  Instance* s = new (std::nothrow) Instance();
  if (!s) return NULL;
  for (int p = 0; p < P_COUNT; ++p) s->param[p] = DefaultValue(p);
  s->bpm = 120.0f;
  s->beat = 0.0;
  s->restart_pending = false;
  s->smooth_init = false;
  s->frozen = false;
  s->clear_cv_high = false;
  s->clear_pending = 0;
  s->clear_stage = 0;
  s->env = 1.0f;
  memset(s->src, 0, sizeof s->src);
  s->reverb.setSampleRate(kSampleRate);
  s->sources.Init(static_cast<float>(kSampleRate));
  s->limiter.Init();
  s->s_drive = DefaultValue(P_LIM_DRIVE);
  s->s_ceiling = DefaultValue(P_LIM_CEILING);
  PushControlSettings(s, 0);
  return s;
}

void Destroy(void* inst) { delete static_cast<Instance*>(inst); }

void Midi(void*, const uint8_t*, int) {}

int FindParam(const char* key, size_t len) {
  for (int p = 0; p < P_COUNT; ++p) {
    if (strlen(PARAMS[p].key) == len && !strncmp(key, PARAMS[p].key, len)) return p;
  }
  return -1;
}

void SetParam(void* inst, const char* key, const char* val);

void LoadState(Instance* s, const char* state) {
  char item[64];
  while (*state) {
    size_t n = strcspn(state, ";");
    if (n < sizeof item) {
      memcpy(item, state, n);
      item[n] = 0;
      char* eq = strchr(item, '=');
      if (eq) {
        *eq = 0;
        if (strcmp(item, "state")) SetParam(s, item, eq + 1);
      }
    }
    state += n;
    if (*state == ';') ++state;
  }
}

int SaveState(const Instance* s, char* buf, int buf_len) {
  int len = 0;
  for (int p = 0; p < P_COUNT && len < buf_len; ++p) {
    if (PARAMS[p].momentary) continue;
    len += snprintf(buf + len, buf_len - len, "%s=%g;", PARAMS[p].key, s->param[p]);
  }
  return len < buf_len ? len : buf_len - 1;
}

void SetParam(void* inst, const char* key, const char* val) {
  Instance* s = static_cast<Instance*>(inst);
  if (!strcmp(key, "state")) { LoadState(s, val); return; }
  if (!strcmp(key, "lfo_bpm")) {   // the MPC tempo (vst.json HAS_LFO_BPM)
    float b = static_cast<float>(atof(val));
    if (b >= 20.0f && b <= 400.0f) s->bpm = b;
    return;
  }
  if (!strcmp(key, "transport")) {   // play pressed or the song position jumped back (HAS_TRANSPORT)
    if (atoi(val) == 1) s->restart_pending = true;
    return;
  }
  int p = FindParam(key, strlen(key));
  if (p < 0) return;
  float v = Clamp(static_cast<float>(atof(val)), MinOf(p), MaxOf(p));
  if (p == P_CLEAR && v > 0.5f && s->param[p] <= 0.5f) s->clear_pending = 1;
  s->param[p] = v;
}

int FormatHz(char* buf, int len, float hz) {
  if (hz >= 1000.0f) return snprintf(buf, len, "%.1f kHz", hz / 1000.0f);
  if (hz >= 10.0f) return snprintf(buf, len, "%.0f Hz", hz);
  if (hz >= 1.0f) return snprintf(buf, len, "%.2f Hz", hz);
  return snprintf(buf, len, "%.3f Hz", hz);
}

bool KeyEndsWith(const char* key, const char* tail) {
  size_t a = strlen(key), b = strlen(tail);
  return a >= b && !strcmp(key + a - b, tail);
}

int Display(const Instance* s, int p, char* buf, int len) {
  float v = s->param[p];
  const char* key = PARAMS[p].key;
  switch (p) {
    case P_DRY: case P_WET: case P_SIZE: case P_MOD_RATE:
      return snprintf(buf, len, "%.0f %%", v * 100.0f);
    case P_PRE_DELAY:
      return snprintf(buf, len, "%.0f ms", v * 1000.0f);
    case P_IN_LOW: case P_RV_LOW:
      return FormatHz(buf, len, PitchHz(LowCutPitch(v, 0.0f)));
    case P_IN_HIGH: case P_RV_HIGH:
      return FormatHz(buf, len, PitchHz(FilterPitch(v, 0.0f)));
    case P_DIFFUSION:
      return snprintf(buf, len, "%.0f %%", v * 10.0f);
    case P_DECAY:   // the module's own scale: 0 % at 0.1, 100 % at 0.9999
      return snprintf(buf, len, "%.0f %%", (v - 0.1f) / (kDecayMax - 0.1f) * 100.0f);
    case P_MOD_SHAPE:
      return snprintf(buf, len, "%+.0f %%", v * 200.0f - 100.0f);
    case P_MOD_DEPTH:
      return snprintf(buf, len, "%.0f %%", v * 6.25f);
    case P_TD_FREQ:
      return snprintf(buf, len, "%+.1f st", v);
    case P_MB_T_RATE:
      return snprintf(buf, len, "%+.0f st", v * 60.0f);
    case P_LIM_DRIVE:
      return snprintf(buf, len, "+%.1f dB", v);
    case P_LIM_CEILING:
      return snprintf(buf, len, "%.1f dB", v);
    case P_LIM_RELEASE:
      return snprintf(buf, len, "%.0f ms", v);
  }
  if (!strncmp(key, "lfo", 3)) {
    int first = P_LFO1_WAVE + (key[3] - '1') * (P_LFO2_WAVE - P_LFO1_WAVE);
    if (KeyEndsWith(key, "_freq")) {
      if (Option(s, first + 1) > 0) return snprintf(buf, len, "Sync");
      float cv = v + (Option(s, first + 3) ? -11.0f : -7.0f);
      float hz = 261.626f * powf(2.0f, cv);
      return FormatHz(buf, len, hz > 2000.0f ? 2000.0f : hz);
    }
    if (KeyEndsWith(key, "_pw")) return snprintf(buf, len, "%+.0f %%", v * 100.0f);
    if (KeyEndsWith(key, "_offset")) return snprintf(buf, len, "%+.2f V", v * 5.0f);
    return snprintf(buf, len, "%.0f %%", v * 100.0f);
  }
  if (KeyEndsWith(key, "_cv")) return snprintf(buf, len, "%+.0f %%", v * 100.0f);
  if (!strncmp(key, "sq", 2) && key[3] == '_' && key[4] == 's' && key[5] >= '0' && key[5] <= '9')
    return snprintf(buf, len, "%+.2f V", v);
  if (PARAMS[p].min == 0.0f && PARAMS[p].max == 1.0f) return snprintf(buf, len, "%.0f %%", v * 100.0f);
  if (PARAMS[p].min == 0.05f && PARAMS[p].max == 1.0f) return snprintf(buf, len, "%.0f %%", v * 100.0f);
  return 0;
}

int GetParam(void* inst, const char* key, char* buf, int buf_len) {
  const Instance* s = static_cast<const Instance*>(inst);
  if (!strcmp(key, "state")) return SaveState(s, buf, buf_len);
  size_t len = strlen(key);
  if (len > 8 && !strcmp(key + len - 8, "_display")) {
    int p = FindParam(key, len - 8);
    return p < 0 ? 0 : Display(s, p, buf, buf_len);
  }
  int p = FindParam(key, len);
  if (p < 0) return 0;
  if (PARAMS[p].nopts > 0) return snprintf(buf, buf_len, "%d", Option(s, p));
  return snprintf(buf, buf_len, "%g", s->param[p]);
}

void Render(void*, int16_t* out_lr, int frames) { memset(out_lr, 0, sizeof(int16_t) * 2 * frames); }

inline int16_t ToShort(float x) {
  x *= 32768.0f;
  return static_cast<int16_t>(x > 32767.0f ? 32767.0f : (x < -32768.0f ? -32768.0f : x));
}

void ProcessBlock(Instance* s, const int16_t* in_lr, int16_t* out_lr, int frames) {
  // ---- tempo and sources
  plateau::Clock clock;
  clock.bpm = s->bpm;
  clock.restart = s->restart_pending;
  if (s->restart_pending) { s->beat = 0.0; s->restart_pending = false; }
  const double beat_step = s->bpm / 60.0 / kSampleRate;
  for (int i = 0; i < frames; ++i) clock.beat[i] = s->beat + i * beat_step;
  s->beat += frames * beat_step;
  if (s->beat > 1e9) s->beat = fmod(s->beat, 6144.0);   // keep precision on very long runs (6144: a multiple of every division)
  bool needed[kNumSources] = {};
  for (int c = 0; c < kNumCvInputs; ++c) {
    int src = Option(s, kCvInput[c][1]);
    if (src > 0 && src < kNumSources) needed[src] = true;
  }
  s->sources.Render(s->param, clock, needed, frames, s->src);

  // ---- reverb
  const bool tuned = Option(s, P_TUNED) != 0;
  const bool saturate = Option(s, P_SATURATE) != 0;
  const float a = 1.0f - expf(-1.0f / (0.005f * static_cast<float>(kSampleRate)));   // 5 ms
  const float env_step = 1.0f / (kClearFade * static_cast<float>(kSampleRate));
  Dattorro& r = s->reverb;
  float t_dry = 0, t_wet = 0, t_size = 0, t_pre = 0;

  for (int i = 0; i < frames; ++i) {
    if (i % kCtl == 0) {
      PushControlSettings(s, i);
      const float* p = s->param;
      t_dry = Clamp(InputCv(s, CV_DRY, i) + p[P_DRY], 0.0f, 1.0f);
      t_wet = Clamp(InputCv(s, CV_WET, i) + p[P_WET], 0.0f, 1.0f);
      t_size = SizeScale(p[P_SIZE], InputCv(s, CV_SIZE, i), tuned);
      t_pre = PreDelay(p[P_PRE_DELAY], InputCv(s, CV_PRE_DELAY, i));
      if (!s->smooth_init) {
        s->s_dry = t_dry; s->s_wet = t_wet; s->s_size = t_size; s->s_pre = t_pre;
        s->smooth_init = true;
      }
    }
    // Clear: fade input and wet out, clear the tank, fade back in (Plateau.cpp's envelope).
    if (s->clear_pending && s->clear_stage == 0) { s->clear_pending = 0; s->clear_stage = 1; }
    if (s->clear_stage == 1) {
      s->env -= env_step;
      if (s->env <= 0.0f) { s->env = 0.0f; r.clear(); s->clear_stage = 2; }
    } else if (s->clear_stage == 2) {
      s->env += env_step;
      if (s->env >= 1.0f) { s->env = 1.0f; s->clear_stage = 0; }
    }

    s->s_dry += a * (t_dry - s->s_dry);
    s->s_wet += a * (t_wet - s->s_wet);
    s->s_size += a * (t_size - s->s_size);
    s->s_pre += a * (t_pre - s->s_pre);
    r.setTimeScale(s->s_size);
    r.setPreDelay(s->s_pre);

    float l = in_lr[2 * i] * (1.0f / 32768.0f);
    float rt = in_lr[2 * i + 1] * (1.0f / 32768.0f);
    r.process(l * s->env, rt * s->env);
    float ol = l * s->s_dry + static_cast<float>(r.getLeftOutput()) * s->s_wet * s->env;
    float orr = rt * s->s_dry + static_cast<float>(r.getRightOutput()) * s->s_wet * s->env;
    if (saturate) {
      ol = tanhDriveSignal(ol * kVolts * kSaturatorPreGain, kSaturatorDrive) * kSaturatorPostGain / kVolts;
      orr = tanhDriveSignal(orr * kVolts * kSaturatorPreGain, kSaturatorDrive) * kSaturatorPostGain / kVolts;
    }
    s->out_l[i] = ol;
    s->out_r[i] = orr;
  }

  // ---- brickwall limiter: Drive in, Ceiling out (RMXXXL's)
  s->s_drive += 0.25f * (s->param[P_LIM_DRIVE] - s->s_drive);
  s->s_ceiling += 0.25f * (s->param[P_LIM_CEILING] - s->s_ceiling);
  s->limiter.Process(s->out_l, s->out_r, frames, s->s_drive, s->s_ceiling, s->param[P_LIM_RELEASE],
                     static_cast<float>(kSampleRate));
  for (int i = 0; i < frames; ++i) {
    out_lr[2 * i] = ToShort(s->out_l[i]);
    out_lr[2 * i + 1] = ToShort(s->out_r[i]);
  }
}

void Process(void* inst, const int16_t* in_lr, int16_t* out_lr, int frames) {
  Instance* s = static_cast<Instance*>(inst);
  ScopedDenormalDisable no_denormals;
  for (int off = 0; off < frames; off += kMaxBlock) {
    int n = frames - off < kMaxBlock ? frames - off : kMaxBlock;
    ProcessBlock(s, in_lr + 2 * off, out_lr + 2 * off, n);
  }
}

const mpc_engine_t kEngine = { Create, Destroy, Midi, SetParam, GetParam, Render, Process };

}  // namespace

extern "C" const mpc_engine_t* mpc_engine(void) { return &kEngine; }
