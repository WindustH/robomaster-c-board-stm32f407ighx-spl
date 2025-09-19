#pragma once
#include "type.h"
#define DMA_BUFFER_SIZE 256
#define MAX_CMD_LENGTH 50

void setup_dma_uart();
void uart_send_data(u8 *data, u16 length);
void uart_process_received_data(u8 half);
void DMA2_Stream7_IRQHandler();
void DMA2_Stream2_IRQHandler();