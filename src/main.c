#include "def.h"
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

  app.pidv.setup();
  app.pidv.set_kp(30.0f);
  app.pidv.set_ki(5.6f);
  app.pidv.set_kd(1.3f);
  app.pidv.set_ol(MAX_CURRENT);
  app.pidv.set_target(0.0f);
  app.pidv.enable();
  bsp.cron.add_job(app.pidv.update);

  // app.pidx.setup();
  // app.pidx.set_kp(40.0f);
  // app.pidx.set_ki(14.0f);
  // app.pidx.set_kd(25.0f);
  // app.pidx.set_ol(6000.0f);
  // app.pidx.set_target(0.0f);
  // app.pidx.enable();
  // bsp.cron.add_job(app.pidx.update);

  // app.mon.setup();
  // bsp.cron.add_job(app.mon.update);

  // bsp.led.show(0xffffffff);

  bsp.cron.add_job(app.tick.update);
  bsp.cron.add_job(bsp.motor.send_ctrl_signal);

  // bsp.led.show(0xFFFF0000);
  bsp.can.bind_rx_callback(bsp.motor.update_status);

  // Main loop
  while (true) {
    // bsp.uart.dma.rxbuf_daemon();
    // app.sendback.daemon();
    // app.handle_cmd.daemon();
  }
}