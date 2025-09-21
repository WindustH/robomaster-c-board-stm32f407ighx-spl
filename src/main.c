#include "bsp.h"

void init_uart_mod(void);
void init_bsp_mod(void);

int main() {
  init_uart_mod();
  init_bsp_mod();

  bsp.clock.setup();
  bsp.led.setup();
  bsp.uart.setup();
  bsp.uart.dma.setup();
  bsp.cron.setup();

  //  main loop
  while (true) {
    bsp.uart.dma.proc_rx_dat();
  }
}
