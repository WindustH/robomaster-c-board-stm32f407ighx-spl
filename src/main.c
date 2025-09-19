#include "bsp/cron.h"
#include "bsp/led.h"
#include "bsp/setup.h"
#include "bsp/uart.h"
#include "bsp/uart/it.h"

int main() {
  setup_clock();
  setup_led();
  setup_cron();

  setup_uart();
  setup_it_uart();

  //  main loop
  while (true) {
    if (uart_received_byte()) {
      uart_send_str("see you!\n");
    }
  }
}
