#pragma once
#include "type.h"

typedef struct {
  void (*setup)();
  void (*update)();
  void (*reset)();
  void (*enable)();
  void (*disable)();
  void (*set_target)(f32 target);
  void (*set_kp)(f32 kp);
  void (*set_ki)(f32 ki);
  void (*set_kd)(f32 kd);
  void (*set_output_limit)(f32 output_limit);
  volatile pidStat *(*status)();
} _PidvMod;
extern const _PidvMod _pidv;

typedef struct {
  void (*setup)();
  void (*update)();
  void (*reset)();
  void (*enable)();
  void (*disable)();
  void (*set_target)(f32 target);
  void (*set_kp)(f32 kp);
  void (*set_ki)(f32 ki);
  void (*set_kd)(f32 kd);
  void (*set_output_limit)(f32 output_limit);
  volatile pidStat *(*status)();
} _PidxMod;
extern const _PidxMod _pidx;

typedef struct {
  void (*update)();
  u32 (*get)();
} _TickMod;
extern const _TickMod _tick;

typedef struct {
  void (*update)();
  monRead *read;
  monWrite *write;
} _MonMod;
extern const _MonMod _mon;

typedef struct {
  void (*init)();
  _PidvMod pidv;
  _PidxMod pidx;
  _MonMod mon;
  _TickMod tick;
} _AppMod;
extern _AppMod app;