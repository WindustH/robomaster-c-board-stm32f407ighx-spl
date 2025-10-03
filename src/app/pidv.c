#include "def.h"
#include "mod/app.h"
#include "mod/bsp.h"
#include "utils/pid_assist.h"
#include <string.h>

static volatile pidStat pid_controllers[8];

static void setup_pidv() {
  pid_controllers[MOTOR_ID].kp = 0.0f;
  pid_controllers[MOTOR_ID].ki = 0.0f;
  pid_controllers[MOTOR_ID].kd = 0.0f;
  pid_controllers[MOTOR_ID].dt = SECOND_PER_TICK;
  pid_controllers[MOTOR_ID].output_limit = 1000.0f;

  pid_controllers[MOTOR_ID].p = 0.0f;
  pid_controllers[MOTOR_ID].i = 0.0f;
  pid_controllers[MOTOR_ID].d = 0.0f;
  pid_controllers[MOTOR_ID].prev_error = 0.0f;
  pid_controllers[MOTOR_ID].target = 0.0f;
  pid_controllers[MOTOR_ID].enabled = 0;
}

static void set_pidv_target(f32 target) {
  pid_controllers[MOTOR_ID].target = target;
}

static void enable_pidv() {
  pid_controllers[MOTOR_ID].enabled = 1;
  pid_controllers[MOTOR_ID].p = 0.0f;
  pid_controllers[MOTOR_ID].i = 0.0f;
  pid_controllers[MOTOR_ID].d = 0.0f;
  pid_controllers[MOTOR_ID].prev_error = 0.0f;
}

static void disable_pidv() { pid_controllers[MOTOR_ID].enabled = 0; }

static void update_pidv() {
  if (!pid_controllers[MOTOR_ID].enabled)
    return;
  f32 output = pid_compute(app.pidv.status(), (f32)bsp.motor.status()->v);
  i16 current = (i16)output;
  bsp.motor.set_current(current);
}

static void reset_pidv() {
  pid_controllers[MOTOR_ID].i = 0.0f;
  pid_controllers[MOTOR_ID].prev_error = 0.0f;
}

static volatile pidStat *get_pidv_status() {
  return &pid_controllers[MOTOR_ID];
}

static void set_pidv_kp(f32 kp) { pid_controllers[MOTOR_ID].kp = kp; }

static void set_pidv_ki(f32 ki) { pid_controllers[MOTOR_ID].ki = ki; }

static void set_pidv_kd(f32 kd) { pid_controllers[MOTOR_ID].kd = kd; }

static void set_pidv_output_limit(f32 output_limit) {
  pid_controllers[MOTOR_ID].output_limit = output_limit;
}

const _PidvMod _pidv = {.setup = setup_pidv,
                        .update = update_pidv,
                        .reset = reset_pidv,
                        .enable = enable_pidv,
                        .disable = disable_pidv,
                        .status = get_pidv_status,
                        .set_target = set_pidv_target,
                        .set_kp = set_pidv_kp,
                        .set_ki = set_pidv_ki,
                        .set_kd = set_pidv_kd,
                        .set_ol = set_pidv_output_limit};