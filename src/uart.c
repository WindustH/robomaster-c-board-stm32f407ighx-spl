#include "uart.h"
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