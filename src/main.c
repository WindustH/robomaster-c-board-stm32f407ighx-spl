#include "mod/app.h"
#include "mod/bsp.h"
#include "stm32f4xx.h"
#include "type.h"

// static volatile buf uart_rx_buf;

int main() {

  bsp.init();
  bsp.clock.setup();
  bsp.led.setup();
  // bsp.uart.setup();
  // bsp.uart.dma.setup(&uart_rx_buf);
  bsp.cron.setup();

  bsp.can.setup();
  bsp.motor.setup();

  app.init();
  // app.sendback.bind_buf(&uart_rx_buf);
  // app.handle_cmd.bind_buf(&uart_rx_buf);
  // bsp.motor.set(0,  500);
  // bsp.motor.set(1,  500);
  // bsp.motor.set(2,  500);
  // bsp.motor.set(3,  500);
  // bsp.motor.set(4,  500);
  // bsp.motor.set(5, 1000);
  // bsp.motor.set(1, 500);
  // Configure PID controller for motor 1

  app.pid.setup();
  app.pid.set_kp(5, 0.02f);
  app.pid.set_ki(5, 0.2f);
  app.pid.set_kd(5, 0.0f);
  app.pid.set_output_limit(5, 1000.0f);
  app.pid.set_mode(5, PID_POSITION);
  app.pid.enable(5);
  app.pid.set_target(5, 300);
  bsp.motor.set_current(5, 500);
  bsp.led.show(0xffffffff);

  bsp.cron.add_job(app.tick.update);
  bsp.cron.add_job(bsp.motor.send_ctrl_signal);
  // bsp.cron.add_job(app.mon.update);
  bsp.cron.add_job(app.pid.update);

  bsp.can.bind_rx_callback(bsp.motor.update_status);

  // Main loop
  while (true) {
    // bsp.uart.dma.rxbuf_daemon();
    // app.sendback.daemon();
    // app.handle_cmd.daemon();
  }
}