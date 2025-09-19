#pragma once
#include "type.h"

void setup_it_uart();
void uart_send_byte(const u8 byte);
void uart_send_str(const char *str);
u8 uart_received_byte();
u8 uart_read_byte();
void USART1_IRQHandler();