#include "utils/pid_assist.h"
#include "mod/bsp.h"
f32 pid_compute(volatile pidStat *const pid, const f32 feedback) {
  if (!pid->enabled)
    return 0.0f;

  // Calculate error
  pid->p = pid->target - feedback;

  // Proportional term
  f32 proportional = pid->kp * pid->p;

  // Integral term with anti-windup
  pid->i += pid->p * pid->dt;

  // Clamp integral to prevent windup
  f32 max_integral = pid->output_limit / (pid->ki + 1e-6f);
  if (pid->i > max_integral)
    pid->i = max_integral;
  if (pid->i < -max_integral)
    pid->i = -max_integral;

  f32 integral = pid->ki * pid->i;

  // Derivative term
  pid->d = (pid->p - pid->prev_error) / pid->dt;
  f32 derivative = pid->kd * pid->d;
  pid->prev_error = pid->p;

  // Calculate output
  f32 output = proportional + integral + derivative;

  // Clamp output
  if (output > pid->output_limit)
    output = pid->output_limit;
  if (output < -pid->output_limit)
    output = -pid->output_limit;

  return output;
}
