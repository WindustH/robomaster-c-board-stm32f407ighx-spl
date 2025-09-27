#include "bsp.h"
#include "stm32f4xx.h"
#include "type.h"

static void setup_uart_it() {
  // Enable interrupt in NVIC
  NVIC_SetPriority(USART1_IRQn, 1);
  NVIC_EnableIRQ(USART1_IRQn);

  // Enable UART receive interrupt
  USART1->CR1 |= USART_CR1_RXNEIE;
}

static void send_byte_impl(const u8 byte) {
  // Wait for transmit buffer to be empty
  while (!(USART1->SR & USART_SR_TXE))
    ;
  // Write data to the data register
  USART1->DR = byte;
  // Wait for transmission to complete
  while (!(USART1->SR & USART_SR_TC))
    ;
}

static void send_str_impl(const char *str) {
  for (const char *p = str; *p; p++) {
    send_byte_impl(*p);
  }
}

volatile u8 uart_rx_data = 0x00;
volatile u8 uart_rx_flag = 0;

static u8 has_new_byte() {
  if (uart_rx_flag == 1) {
    uart_rx_flag = 0;
    return 1;
  }
  return 0;
}

static u8 read_byte_impl() { return uart_rx_data; }

void USART1_IRQHandler() {
  // Check if the Receive Data Register Not Empty interrupt has occurred
  if (USART1->SR & USART_SR_RXNE) {
    // Read the data from the data register (this also clears the RXNE flag)
    uart_rx_data = (u8)USART1->DR;
    uart_rx_flag = 1;
  }
}

const _UartItMod _uart_it = {
    .setup = setup_uart_it,
    .send_str = send_str_impl,
    .send_byte = send_byte_impl,
    .has_new_byte = has_new_byte,
    .read_byte = read_byte_impl,
};