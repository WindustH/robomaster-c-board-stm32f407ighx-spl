#include "app.h"
#include "app/cron_job.h"
#include "bsp.h"
#include "stm32f4xx_hal.h"

static volatile buf uart_rx_buf;

int main() {
  // Initialize HAL
  HAL_Init();

  // Setup system clock
  bsp.clock.setup();

  // Setup BSP modules
  bsp.setup();
  bsp.led.setup();
  // bsp.uart.setup();
  // bsp.uart.dma.setup(&uart_rx_buf);
  bsp.cron.setup();
  bsp.cron.add_job(light_breathe);

  // app.setup();
  // app.sendback.bind_buf(&uart_rx_buf);
  // app.handle_cmd.bind_buf(&uart_rx_buf);

  // Main loop
  while (true) {
    // bsp.uart.dma.rxbuf_daemon();
    // app.sendback.daemon();
    // app.handle_cmd.daemon();
  }
}