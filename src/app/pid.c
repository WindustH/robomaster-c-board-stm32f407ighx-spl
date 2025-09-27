#include "app.h"
#include "bsp.h"
#include <string.h>

static pidStat pid_controllers[8] = {0};

static void setup_pid(u8 motor_id, f32 kp, f32 ki, f32 kd, f32 dt,
                      f32 output_limit, pidMode mode) {
  if (motor_id > 7)
    return;

  pidStat *pid = &pid_controllers[motor_id];

  pid->kp = kp;
  pid->ki = ki;
  pid->kd = kd;
  pid->dt = dt;
  pid->output_limit = output_limit;
  pid->mode = mode;

  pid->integral = 0.0f;
  pid->prev_error = 0.0f;
  pid->target = 0.0f;
  pid->enabled = 0;
}

static void set_target_impl(u8 motor_id, f32 target) {
  if (motor_id > 7)
    return;

  pidStat *pid = &pid_controllers[motor_id];
  pid->target = target;
}

static void pid_enable(u8 motor_id) {
  if (motor_id > 7)
    return;

  pidStat *pid = &pid_controllers[motor_id];
  pid->enabled = 1;

  // Reset integral and previous error when enabling
  pid->integral = 0.0f;
  pid->prev_error = 0.0f;
}

static void pid_disable(u8 motor_id) {
  if (motor_id > 7)
    return;

  pidStat *pid = &pid_controllers[motor_id];
  pid->enabled = 0;
}

static f32 pid_compute(u8 motor_id) {
  if (motor_id > 7)
    return 0.0f;

  pidStat *pid = &pid_controllers[motor_id];
  if (!pid->enabled)
    return 0.0f;

  // Get current feedback based on control mode
  f32 feedback = 0.0f;
  motStat status = bsp.motor.status(motor_id);

  switch (pid->mode) {
  case PID_POSITION:
    feedback = (f32)status.th;
    break;
  case PID_VELOCITY:
    feedback = (f32)status.v;
    break;
  case PID_CURRENT:
    feedback = (f32)status.i;
    break;
  }

  // Calculate error
  f32 error = pid->target - feedback;

  // Proportional term
  f32 proportional = pid->kp * error;

  // Integral term with anti-windup
  pid->integral += error * pid->dt;

  // Clamp integral to prevent windup
  f32 max_integral = pid->output_limit / (pid->ki + 1e-6f);
  if (pid->integral > max_integral)
    pid->integral = max_integral;
  if (pid->integral < -max_integral)
    pid->integral = -max_integral;

  f32 integral = pid->ki * pid->integral;

  // Derivative term
  f32 derivative = pid->kd * (error - pid->prev_error) / pid->dt;
  pid->prev_error = error;

  // Calculate output
  f32 output = proportional + integral + derivative;

  // Clamp output
  if (output > pid->output_limit)
    output = pid->output_limit;
  if (output < -pid->output_limit)
    output = -pid->output_limit;

  return output;
}

static void pid_update(u8 motor_id) {
  if (motor_id > 7)
    return;

  pidStat *pid = &pid_controllers[motor_id];
  if (!pid->enabled)
    return;

  f32 output = pid_compute(motor_id);

  // Convert output to current (assuming output is in appropriate units)
  i16 current = (i16)output;

  // Set motor current
  bsp.motor.set(motor_id, current);
}

static void pid_reset(u8 motor_id) {
  if (motor_id > 7)
    return;

  pidStat *pid = &pid_controllers[motor_id];
  pid->integral = 0.0f;
  pid->prev_error = 0.0f;
}

const _PidMod _pid = {.setup = setup_pid,
                      .update = pid_update,
                      .reset = pid_reset,
                      .enable = pid_enable,
                      .disable = pid_disable,
                      .set_target = set_target_impl};