#include "mod/app.h"
#include "mod/bsp.h"

// Global monitor data structures
static monitor_read_t monitor_read_data = {0};
static monitor_write_t monitor_write_data = {0};

static void update_monitor() {
  // Update timestamp
  monitor_read_data.timestamp = app.tick.get();

  // Update motor status for all motors
  for (u8 i = 0; i < 8; i++) {
    monitor_read_data.motors[i] = bsp.motor.status(i);
  }

  // Update PID status for all motors
  for (u8 i = 0; i < 8; i++) {
    pidStat *pid_status = app.pid.status(i);
    if (pid_status != NULL) {
      monitor_read_data.pids[i] = *pid_status;
    }
  }

  // Apply write data to system
  for (u8 i = 0; i < 8; i++) {
    if (monitor_write_data.use_pid_control) {
      // Use PID control
      if (monitor_write_data.pid_enabled[i]) {
        app.pid.set_target(i, monitor_write_data.pid_targets[i]);
        app.pid.enable(i);
      } else {
        app.pid.disable(i);
      }
    } else {
      // Use direct current control
      app.pid.disable(i);
      bsp.motor.set(i, monitor_write_data.motor_currents[i]);
    }
  }
}

const _MonMod _mon = {.update = update_monitor,
                      .read = &monitor_read_data,
                      .write = &monitor_write_data};