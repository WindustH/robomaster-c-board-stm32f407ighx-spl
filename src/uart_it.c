#include "uart_it.h"
#include "stm32f4xx.h"

void uart_send_byte(u8 byte) {
  USART_SendData(USART1, byte);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    ;
}
void uart_send_str(const char *str) {
  u8 i;
  for (i = 0; str[i] != '\0'; i++)
    uart_send_byte(str[i]);
}

extern volatile u8 uart_rx_flag;
extern volatile u8 uart_rx_data;

u8 uart_received_byte(void) {
  if (uart_rx_flag == 1) {
    uart_rx_flag = 0;
    return 1;
  }
  return 0;
}

u8 uart_read_byte(void) { return uart_rx_data; }