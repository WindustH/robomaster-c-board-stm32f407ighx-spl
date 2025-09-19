#include "setup.h"
#include "uart_it.h"

int main(void) {
  setup_clock();
  setup_led();
  setup_schedule();
  setup_uart();
  setup_it_uart();

  //  main loop
  while (true) {
    if (uart_received_byte()) {
      uart_send_str("see you!\n");
    }
  }
}
