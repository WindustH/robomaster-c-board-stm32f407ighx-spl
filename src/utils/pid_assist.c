#include "utils/pid_assist.h"
#include "mod/bsp.h"
f32 pid_compute(volatile pidStat *const pid, const f32 feedback) {
  if (!pid->enabled)
    return 0.0f;

  // Calculate error
  pid->p = pid->target - feedback;

  f32 kp, ki, kd;
  if (pid->p < pid->r && pid->p > -pid->r) {
    kp = pid->kpr;
    ki = pid->kir;
    kd = pid->kdr;
  } else {
    kp = pid->kp;
    ki = pid->ki;
    kd = pid->kd;
  }
  // Proportional term
  f32 p_term = kp * pid->p;

  // Integral term with anti-windup
  pid->i += pid->p * pid->dt;

  // Clamp integral to prevent windup
  f32 max_integral = pid->output_limit / (pid->ki + 1e-6f);
  if (pid->i > max_integral)
    pid->i = max_integral;
  if (pid->i < -max_integral)
    pid->i = -max_integral;

  f32 i_term = ki * pid->i;

  // Derivative term
  pid->d = (pid->p - pid->prev_error) / pid->dt;
  f32 d_term = kd * pid->d;
  pid->prev_error = pid->p;

  f32 output = p_term + i_term + d_term;

  if (output > pid->output_limit)
    output = pid->output_limit;
  if (output < -pid->output_limit)
    output = -pid->output_limit;

  return output;
}
