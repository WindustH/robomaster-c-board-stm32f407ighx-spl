#include "mod/app.h"
#include "mod/bsp.h"
#include "stm32f4xx.h"

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
  bsp.can.bind_rx_callback(bsp.motor.update_status);

  app.init();
  // app.sendback.bind_buf(&uart_rx_buf);
  // app.handle_cmd.bind_buf(&uart_rx_buf);
  // bsp.motor.set(0,  500);
  // bsp.motor.set(1,  500);
  // bsp.motor.set(2,  500);
  // bsp.motor.set(3,  500);
  // bsp.motor.set(4,  500);
  // bsp.motor.set(5, 1000);
  bsp.motor.set(6, 1000);
  // bsp.motor.set(7, 1000);
  bsp.led.show(0xffffffff);

  bsp.cron.add_job(app.tick.update);
  bsp.cron.add_job(bsp.motor.send_ctrl_signal);
  bsp.cron.add_job(app.mon.update);

  // Main loop
  while (true) {

    // bsp.uart.dma.rxbuf_daemon();
    // app.sendback.daemon();
    // app.handle_cmd.daemon();
  }
}