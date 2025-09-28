#pragma once
#include "type.h"

typedef struct {
  void (*setup)(u8 motor_id, f32 kp, f32 ki, f32 kd, f32 dt, f32 output_limit,
                pidMode mode);
  void (*update)(u8 motor_id);
  void (*reset)(u8 motor_id);
  void (*enable)(u8 motor_id);
  void (*disable)(u8 motor_id);
  void (*set_target)(u8 motor_id, f32 target);
  void (*set_kp)(u8 motor_id, f32 kp);
  void (*set_ki)(u8 motor_id, f32 ki);
  void (*set_kd)(u8 motor_id, f32 kd);
  void (*set_output_limit)(u8 motor_id, f32 output_limit);
  void (*set_mode)(u8 motor_id, pidMode mode);
  pidStat *(*status)(u8 motor_id);
} _PidMod;
extern const _PidMod _pid;

typedef struct {
  void (*update)();
  u32 (*get)();
} _TickMod;
extern const _TickMod _tick;

typedef struct {
  void (*update)();
  monitor_read_t *read;
  monitor_write_t *write;
} _MonMod;
extern const _MonMod _mon;

typedef struct {
  void (*init)();
  _PidMod pid;
  _MonMod mon;
  _TickMod tick;
} _AppMod;
extern _AppMod app;