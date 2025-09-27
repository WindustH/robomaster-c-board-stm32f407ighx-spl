#include "bsp.h"
#include "stm32f4xx_hal.h"
#include "type.h"

// External UART handle from uart.c
extern UART_HandleTypeDef huart1;

static void setup_impl() {
  // Enable interrupt in NVIC
  HAL_NVIC_SetPriority(USART1_IRQn, 1, 1);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

  // Enable UART receive interrupt
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
}

static void send_byte_impl(const u8 byte) {
  HAL_UART_Transmit(&huart1, &byte, 1, HAL_MAX_DELAY);
}

static void send_str_impl(const char *str) {
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
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

void USART1_IRQHandler() { HAL_UART_IRQHandler(&huart1); }

const _UartItMod _uart_it = {
    .setup = setup_impl,
    .send_str = send_str_impl,
    .send_byte = send_byte_impl,
    .has_new_byte = has_new_byte,
    .read_byte = read_byte_impl,
};