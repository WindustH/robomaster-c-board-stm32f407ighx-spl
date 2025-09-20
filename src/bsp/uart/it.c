#include "bsp/uart/it.h"
#include "stm32f4xx_conf.h"

void setup_it_uart() {
  // config nvic
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_Init(&NVIC_InitStructure);

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void uart_send_byte(u8 byte) {
  USART_SendData(USART1, byte);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    ;
}
void uart_send_str(const char *str) {
  u16 i;
  for (i = 0; str[i] != '\0'; i++)
    uart_send_byte(str[i]);
}

volatile u8 uart_rx_data = 0x00;
volatile u8 uart_rx_flag = 0;

u8 uart_received_byte() {
  if (uart_rx_flag == 1) {
    uart_rx_flag = 0;
    return 1;
  }
  return 0;
}

u8 uart_read_byte() { return uart_rx_data; }

void USART1_IRQHandler() {
  if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
    uart_rx_data = USART_ReceiveData(USART1);
    uart_rx_flag = 1;
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }
}
