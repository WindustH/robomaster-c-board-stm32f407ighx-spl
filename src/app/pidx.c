#include "def.h"
#include "mod/app.h"
#include "mod/bsp.h"
#include "utils/pid_assist.h"
#include <string.h>

static volatile pidStat pid_controllers[8];

static void pid_setup() {
  pid_controllers[MOTOR_ID].kp = 0.0f;
  pid_controllers[MOTOR_ID].ki = 0.0f;
  pid_controllers[MOTOR_ID].kd = 0.0f;
  pid_controllers[MOTOR_ID].dt = SECOND_PER_TICK;
  pid_controllers[MOTOR_ID].output_limit = 10.0f;

  pid_controllers[MOTOR_ID].p = 0.0f;
  pid_controllers[MOTOR_ID].i = 0.0f;
  pid_controllers[MOTOR_ID].d = 0.0f;
  pid_controllers[MOTOR_ID].prev_error = 0.0f;
  pid_controllers[MOTOR_ID].target = 0.0f;
  pid_controllers[MOTOR_ID].enabled = 0;
}

static void set_target_impl(f32 target) {
  pid_controllers[MOTOR_ID].target = target;
}

static void pid_enable() {
  pid_controllers[MOTOR_ID].enabled = 1;
  pid_controllers[MOTOR_ID].i = 0.0f;
  pid_controllers[MOTOR_ID].prev_error = 0.0f;
}

static void pid_disable() { pid_controllers[MOTOR_ID].enabled = 0; }

static void pid_update() {
  if (!pid_controllers[MOTOR_ID].enabled)
    return;
  f32 velocity = pid_compute(app.pidx.status(), bsp.motor.status()->th);
  app.pidv.set_target(velocity);
}

static void pid_reset() {
  pid_controllers[MOTOR_ID].i = 0.0f;
  pid_controllers[MOTOR_ID].prev_error = 0.0f;
}

static volatile pidStat *pid_get_status() { return &pid_controllers[MOTOR_ID]; }

static void set_kp_impl(f32 kp) { pid_controllers[MOTOR_ID].kp = kp; }

static void set_ki_impl(f32 ki) { pid_controllers[MOTOR_ID].ki = ki; }

static void set_kd_impl(f32 kd) { pid_controllers[MOTOR_ID].kd = kd; }

static void set_output_limit_impl(f32 output_limit) {
  pid_controllers[MOTOR_ID].output_limit = output_limit;
}

const _PidxMod _pidx = {.setup = pid_setup,
                        .update = pid_update,
                        .reset = pid_reset,
                        .enable = pid_enable,
                        .disable = pid_disable,
                        .status = pid_get_status,
                        .set_target = set_target_impl,
                        .set_kp = set_kp_impl,
                        .set_ki = set_ki_impl,
                        .set_kd = set_kd_impl,
                        .set_ol = set_output_limit_impl};