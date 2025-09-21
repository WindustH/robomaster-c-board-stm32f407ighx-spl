#include "app.h"
#include "app/cron_job.h"
#include "bsp.h"
static volatile buf uart_rx_buf;

int main() {
  bsp.setup();
  bsp.clock.setup();
  bsp.led.setup();
  bsp.uart.setup();
  bsp.uart.dma.setup(&uart_rx_buf);
  bsp.cron.setup();
  bsp.cron.add_job(light_breathe);

  app.setup();
  app.sendback.bind_buf(&uart_rx_buf);

  //  main loop
  while (true) {
    bsp.uart.dma.rxbuf_daemon();
    app.sendback.daemon();
  }
}
