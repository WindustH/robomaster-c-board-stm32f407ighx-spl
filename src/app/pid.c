#include "mod/app.h"
#include "mod/bsp.h"
#include <string.h>

static volatile pidStat pid_controllers[8];

static void initialize_pid_controller(u8 motor_id) {
  if (motor_id > 7)
    return;

  // Initialize with default values
  pid_controllers[motor_id].kp = 0.0f;
  pid_controllers[motor_id].ki = 0.0f;
  pid_controllers[motor_id].kd = 0.0f;
  pid_controllers[motor_id].dt = SECOND_PER_TICK;
  pid_controllers[motor_id].output_limit = 1000.0f;
  pid_controllers[motor_id].mode = PID_VELOCITY;

  pid_controllers[motor_id].integral = 0.0f;
  pid_controllers[motor_id].prev_error = 0.0f;
  pid_controllers[motor_id].target = 0.0f;
  pid_controllers[motor_id].enabled = 0;
}

static void pid_setup() {
  // Initialize all PID controllers
  for (u8 i = 0; i < 8; i++) {
    initialize_pid_controller(i);
  }
}

static void set_target_impl(u8 motor_id, f32 target) {
  if (motor_id > 7)
    return;
  pid_controllers[motor_id].target = target;
}

static void pid_enable(u8 motor_id) {
  if (motor_id > 7)
    return;

  pid_controllers[motor_id].enabled = 1;

  // Reset integral and previous error when enabling
  pid_controllers[motor_id].integral = 0.0f;
  pid_controllers[motor_id].prev_error = 0.0f;
}

static void pid_disable(u8 motor_id) {
  if (motor_id > 7)
    return;
  pid_controllers[motor_id].enabled = 0;
}

static f32 pid_compute(u8 motor_id) {
  if (motor_id > 7)
    return 0.0f;

  volatile pidStat *pid = &pid_controllers[motor_id];
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

static void pid_update() {

  for (u8 motor_id = 0; motor_id < 8; motor_id++) {
    if (!pid_controllers[motor_id].enabled)
      continue;

    f32 output = pid_compute(motor_id);

    // Convert output to current (assuming output is in appropriate units)
    i16 current = (i16)output;

    // Set motor current
    bsp.motor.set_current(motor_id, current);
    // Update all enabled PID controllers
    bsp.led.show(0xFFFF0000);
  }
}

static void pid_reset(u8 motor_id) {
  if (motor_id > 7)
    return;

  pid_controllers[motor_id].integral = 0.0f;
  pid_controllers[motor_id].prev_error = 0.0f;
}

// Function to get PID status for monitoring
pidStat *pid_get_status(u8 motor_id) {
  if (motor_id > 7)
    return NULL;
  return (pidStat *)&pid_controllers[motor_id];
}

static void set_kp_impl(u8 motor_id, f32 kp) {
  if (motor_id > 7)
    return;
  pid_controllers[motor_id].kp = kp;
}

static void set_ki_impl(u8 motor_id, f32 ki) {
  if (motor_id > 7)
    return;
  pid_controllers[motor_id].ki = ki;
}

static void set_kd_impl(u8 motor_id, f32 kd) {
  if (motor_id > 7)
    return;
  pid_controllers[motor_id].kd = kd;
}

static void set_output_limit_impl(u8 motor_id, f32 output_limit) {
  if (motor_id > 7)
    return;
  pid_controllers[motor_id].output_limit = output_limit;
}

static void set_mode_impl(u8 motor_id, pidMode mode) {
  if (motor_id > 7)
    return;
  pid_controllers[motor_id].mode = mode;
}

const _PidMod _pid = {.setup = pid_setup,
                      .update = pid_update,
                      .reset = pid_reset,
                      .enable = pid_enable,
                      .disable = pid_disable,
                      .status = pid_get_status,
                      .set_target = set_target_impl,
                      .set_kp = set_kp_impl,
                      .set_ki = set_ki_impl,
                      .set_kd = set_kd_impl,
                      .set_output_limit = set_output_limit_impl,
                      .set_mode = set_mode_impl};