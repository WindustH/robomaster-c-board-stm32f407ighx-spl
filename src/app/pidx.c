#include "def.h"
#include "mod/app.h"
#include "mod/bsp.h"
#include "utils/pid_assist.h"
#include <string.h>

static volatile pidStat pidx_stat[8];

static void setup_pidx() {
  pidx_stat[MOTOR_ID].kp = 0.0f;
  pidx_stat[MOTOR_ID].ki = 0.0f;
  pidx_stat[MOTOR_ID].kd = 0.0f;
  pidx_stat[MOTOR_ID].kpr = 0.0f;
  pidx_stat[MOTOR_ID].kir = 0.0f;
  pidx_stat[MOTOR_ID].kdr = 0.0f;

  pidx_stat[MOTOR_ID].dt = SECOND_PER_TICK;
  pidx_stat[MOTOR_ID].output_limit = 10.0f;

  pidx_stat[MOTOR_ID].r = 0.0f;
  pidx_stat[MOTOR_ID].p = 0.0f;
  pidx_stat[MOTOR_ID].i = 0.0f;
  pidx_stat[MOTOR_ID].d = 0.0f;
  pidx_stat[MOTOR_ID].prev_error = 0.0f;
  pidx_stat[MOTOR_ID].target = 0.0f;
  pidx_stat[MOTOR_ID].enabled = 0;
}

static void set_pidx_target(f32 target) { pidx_stat[MOTOR_ID].target = target; }

static void enable_pidx() {
  pidx_stat[MOTOR_ID].enabled = 1;
  pidx_stat[MOTOR_ID].p = 0.0f;
  pidx_stat[MOTOR_ID].i = 0.0f;
  pidx_stat[MOTOR_ID].d = 0.0f;
  pidx_stat[MOTOR_ID].prev_error = 0.0f;
  bsp.motor.reset_postition();
}

static void disable_pidx() { pidx_stat[MOTOR_ID].enabled = 0; }

static void update_pidx() {
  if (!pidx_stat[MOTOR_ID].enabled)
    return;
  f32 velocity = pid_compute(app.pidx.status(), bsp.motor.status()->th);

  app.pidv.set_target(velocity);
}

static void reset_pidx() {
  pidx_stat[MOTOR_ID].i = 0.0f;
  pidx_stat[MOTOR_ID].prev_error = 0.0f;
}

static volatile pidStat *get_pidx_status() { return &pidx_stat[MOTOR_ID]; }

static void set_pidx_kp(f32 kp) { pidx_stat[MOTOR_ID].kp = kp; }

static void set_pidx_ki(f32 ki) { pidx_stat[MOTOR_ID].ki = ki; }

static void set_pidx_kd(f32 kd) { pidx_stat[MOTOR_ID].kd = kd; }

static void set_pidx_kpr(f32 kp) { pidx_stat[MOTOR_ID].kpr = kp; }

static void set_pidx_kir(f32 ki) { pidx_stat[MOTOR_ID].kir = ki; }

static void set_pidx_kdr(f32 kd) { pidx_stat[MOTOR_ID].kdr = kd; }
static void set_pidx_r(f32 r) { pidx_stat[MOTOR_ID].r = r; }

static void set_pidx_output_limit(f32 output_limit) {
  pidx_stat[MOTOR_ID].output_limit = output_limit;
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
                        .set_kpr = set_pidx_kpr,
                        .set_kir = set_pidx_kir,
                        .set_kdr = set_pidx_kdr,
                        .set_ol = set_pidx_output_limit,
                        .set_r = set_pidx_r};