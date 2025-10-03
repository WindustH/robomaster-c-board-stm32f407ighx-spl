#include "def.h"
#include "mod/app.h"
#include "mod/bsp.h"
#include "utils/pid_assist.h"
#include <string.h>

static volatile pidStat pidx_controllers[8];

static void setup_pidx() {
  pidx_controllers[MOTOR_ID].kp = 0.0f;
  pidx_controllers[MOTOR_ID].ki = 0.0f;
  pidx_controllers[MOTOR_ID].kd = 0.0f;
  pidx_controllers[MOTOR_ID].dt = SECOND_PER_TICK;
  pidx_controllers[MOTOR_ID].output_limit = 10.0f;

  pidx_controllers[MOTOR_ID].p = 0.0f;
  pidx_controllers[MOTOR_ID].i = 0.0f;
  pidx_controllers[MOTOR_ID].d = 0.0f;
  pidx_controllers[MOTOR_ID].prev_error = 0.0f;
  pidx_controllers[MOTOR_ID].target = 0.0f;
  pidx_controllers[MOTOR_ID].enabled = 0;
}

static void set_pidx_target(f32 target) {
  pidx_controllers[MOTOR_ID].target = target;
}

static void enable_pidx() {
  pidx_controllers[MOTOR_ID].enabled = 1;
  pidx_controllers[MOTOR_ID].p = 0.0f;
  pidx_controllers[MOTOR_ID].i = 0.0f;
  pidx_controllers[MOTOR_ID].d = 0.0f;
  pidx_controllers[MOTOR_ID].prev_error = 0.0f;
}

static void disable_pidx() { pidx_controllers[MOTOR_ID].enabled = 0; }

static void update_pidx() {
  if (!pidx_controllers[MOTOR_ID].enabled)
    return;
  f32 velocity = pid_compute(app.pidx.status(), bsp.motor.status()->th);
  app.pidv.set_target(velocity);
}

static void reset_pidx() {
  pidx_controllers[MOTOR_ID].i = 0.0f;
  pidx_controllers[MOTOR_ID].prev_error = 0.0f;
}

static volatile pidStat *get_pidx_status() {
  return &pidx_controllers[MOTOR_ID];
}

static void set_pidx_kp(f32 kp) { pidx_controllers[MOTOR_ID].kp = kp; }

static void set_pidx_ki(f32 ki) { pidx_controllers[MOTOR_ID].ki = ki; }

static void set_pidx_kd(f32 kd) { pidx_controllers[MOTOR_ID].kd = kd; }

static void set_pidx_output_limit(f32 output_limit) {
  pidx_controllers[MOTOR_ID].output_limit = output_limit;
}

const _PidxMod _pidx = {.setup = setup_pidx,
                        .update = update_pidx,
                        .reset = reset_pidx,
                        .enable = enable_pidx,
                        .disable = disable_pidx,
                        .status = get_pidx_status,
                        .set_target = set_pidx_target,
                        .set_kp = set_pidx_kp,
                        .set_ki = set_pidx_ki,
                        .set_kd = set_pidx_kd,
                        .set_ol = set_pidx_output_limit};