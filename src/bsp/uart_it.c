#include "bsp/uart_it.h"
#include "bsp.h"
#include "stm32f4xx_conf.h"

static void setup_impl() {
  // config nvic
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_Init(&NVIC_InitStructure);

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

static void send_byte_impl(const u8 byte) {
  USART_SendData(USART1, byte);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    ;
}
static void send_str_impl(const char *str) {
  u16 i;
  for (i = 0; str[i] != '\0'; i++)
    send_byte_impl(str[i]);
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
  if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
    uart_rx_data = USART_ReceiveData(USART1);
    uart_rx_flag = 1;
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }
}

const _UartItMod _uart_it = {
    .setup = setup_impl,
    .send_str = send_str_impl,
    .send_byte = send_byte_impl,
    .has_new_byte = has_new_byte,
    .read_byte = read_byte_impl,
};
