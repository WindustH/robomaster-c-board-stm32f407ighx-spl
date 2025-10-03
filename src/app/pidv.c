#include "def.h"
#include "mod/app.h"
#include "mod/bsp.h"
#include "utils/pid_assist.h"
#include <string.h>

static volatile pidStat pidv_stat[8];

static void setup_pidv() {
  pidv_stat[MOTOR_ID].kp = 0.0f;
  pidv_stat[MOTOR_ID].ki = 0.0f;
  pidv_stat[MOTOR_ID].kd = 0.0f;
  pidv_stat[MOTOR_ID].kpr = 0.0f;
  pidv_stat[MOTOR_ID].kir = 0.0f;
  pidv_stat[MOTOR_ID].kdr = 0.0f;

  pidv_stat[MOTOR_ID].dt = SECOND_PER_TICK;
  pidv_stat[MOTOR_ID].output_limit = 1000.0f;

  pidv_stat[MOTOR_ID].r = 0.0f;
  pidv_stat[MOTOR_ID].p = 0.0f;
  pidv_stat[MOTOR_ID].i = 0.0f;
  pidv_stat[MOTOR_ID].d = 0.0f;
  pidv_stat[MOTOR_ID].prev_error = 0.0f;
  pidv_stat[MOTOR_ID].target = 0.0f;
  pidv_stat[MOTOR_ID].enabled = 0;
}

static void set_pidv_target(f32 target) { pidv_stat[MOTOR_ID].target = target; }

static void enable_pidv() {
  pidv_stat[MOTOR_ID].enabled = 1;
  pidv_stat[MOTOR_ID].p = 0.0f;
  pidv_stat[MOTOR_ID].i = 0.0f;
  pidv_stat[MOTOR_ID].d = 0.0f;
  pidv_stat[MOTOR_ID].prev_error = 0.0f;
}

static void disable_pidv() { pidv_stat[MOTOR_ID].enabled = 0; }

static void update_pidv() {
  if (!pidv_stat[MOTOR_ID].enabled)
    return;
  f32 output = pid_compute(app.pidv.status(), (f32)bsp.motor.status()->v);
  i16 current = (i16)output;
  bsp.motor.set_current(current);
}

static void reset_pidv() {
  pidv_stat[MOTOR_ID].i = 0.0f;
  pidv_stat[MOTOR_ID].prev_error = 0.0f;
}

static volatile pidStat *get_pidv_status() { return &pidv_stat[MOTOR_ID]; }

static void set_pidv_kp(f32 kp) { pidv_stat[MOTOR_ID].kp = kp; }

static void set_pidv_ki(f32 ki) { pidv_stat[MOTOR_ID].ki = ki; }

static void set_pidv_kd(f32 kd) { pidv_stat[MOTOR_ID].kdr = kd; }

static void set_pidv_kpr(f32 kpr) { pidv_stat[MOTOR_ID].kpr = kpr; }

static void set_pidv_kir(f32 kir) { pidv_stat[MOTOR_ID].kir = kir; }

static void set_pidv_kdr(f32 kdr) { pidv_stat[MOTOR_ID].kdr = kdr; }

static void set_pidv_r(f32 r) { pidv_stat[MOTOR_ID].r = r; }

static void set_pidv_output_limit(f32 output_limit) {
  pidv_stat[MOTOR_ID].output_limit = output_limit;
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
                        .set_kpr = set_pidv_kpr,
                        .set_kir = set_pidv_kir,
                        .set_kdr = set_pidv_kdr,
                        .set_ol = set_pidv_output_limit,
                        .set_r = set_pidv_r};