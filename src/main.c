#include "bsp.h"

int main() {
  bsp.clock.setup();
  bsp.led.setup();
  bsp.uart.setup();
  bsp.uart.dma.setup();
  setup_cron();

  //  main loop
  while (true) {
    bsp.uart.dma.proc_rx_dat();
  }
}
