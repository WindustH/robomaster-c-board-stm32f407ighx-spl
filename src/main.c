#include "bsp/clock.h"
#include "bsp/cron.h"
#include "bsp/led.h"
#include "bsp/uart.h"
#include "bsp/uart/dma.h"

int main() {
  setup_clock();
  setup_led();
  setup_cron();

  setup_uart();
  setup_dma_uart();
  //  main loop
  while (true) {
    uart_process_received_data();
  }
}
