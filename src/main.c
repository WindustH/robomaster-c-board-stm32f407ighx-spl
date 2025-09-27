#include "app.h"
#include "app/cron_job.h"
#include "bsp.h"
#include "stm32f4xx.h"

// static volatile buf uart_rx_buf;

int main() {

  bsp.init();
  bsp.clock.setup();
  bsp.led.setup();
  // bsp.uart.setup();
  // bsp.uart.dma.setup(&uart_rx_buf);
  bsp.cron.setup();
  bsp.cron.add_job(light_breathe);

  bsp.can.setup();
  bsp.motor.setup();
  bsp.can.bind_rx_callback(bsp.motor.update_status);
  bsp.cron.add_job(bsp.motor.send_ctrl_signal);

  app.init();
  // app.sendback.bind_buf(&uart_rx_buf);
  // app.handle_cmd.bind_buf(&uart_rx_buf);

  // Main loop
  while (true) {
    // bsp.led.show(0xffffffff);
    // bsp.uart.dma.rxbuf_daemon();
    // app.sendback.daemon();
    // app.handle_cmd.daemon();
  }
}