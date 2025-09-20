#pragma once
#include "type.h"
#define DMA_BUFFER_SIZE 256

void setup_dma_uart();
void uart_send_data(const u8 *data, const u16 length);
void uart_send_str_dma(const char *str);
void uart_process_received_data();
void DMA2_Stream7_IRQHandler();
void DMA2_Stream2_IRQHandler();