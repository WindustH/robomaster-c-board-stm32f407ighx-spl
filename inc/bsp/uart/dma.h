#pragma once
#include "type.h"
#define DMA_BUFFER_SIZE 256

void setup_dma_uart();
void uart_send_data(u8 *data, u16 length);
void uart_process_received_data();
void DMA2_Stream7_IRQHandler();
void DMA2_Stream2_IRQHandler();