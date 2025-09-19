#include "bsp/uart/dma.h"
#include "stm32f4xx_conf.h"
#include <string.h>

// dma transmit buffer for uart data
static volatile u8 dma_tx_buffer[DMA_BUFFER_SIZE];
// dma receive buffer for uart data
static volatile u8 dma_rx_buffer[DMA_BUFFER_SIZE];

// init dma for uart
// config both tx and rx streams with appropriate settings
void setup_dma_uart() {
  DMA_InitTypeDef DMA_InitStructure;

  // enable dma2 clock
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

  // config dma for uart tx (stream7, channel4)
  DMA_DeInit(DMA2_Stream7);
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)dma_tx_buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = DMA_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream7, &DMA_InitStructure);

  // config dma for uart rx (stream2, channel4)
  // using circular mode for continuous reception
  DMA_DeInit(DMA2_Stream2);
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)dma_rx_buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = DMA_BUFFER_SIZE;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_Init(DMA2_Stream2, &DMA_InitStructure);

  // enable both dma streams
  DMA_Cmd(DMA2_Stream7, ENABLE);
  DMA_Cmd(DMA2_Stream2, ENABLE);

  // enable dma requests for uart
  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
  USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

  // initialize buffers to zero
  memset((void *)dma_tx_buffer, 0, DMA_BUFFER_SIZE);
  memset((void *)dma_rx_buffer, 0, DMA_BUFFER_SIZE);

  NVIC_InitTypeDef NVIC_InitStructure;

  // enable transfer complete interrupt for tx
  DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);

  // enable half transfer and transfer complete interrupts for rx
  DMA_ITConfig(DMA2_Stream2, DMA_IT_HT, ENABLE);
  DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);

  // configure nvic for tx dma interrupt
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // configure nvic for rx dma interrupt
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void uart_send_data(u8 *data, u16 length) {
  while (DMA_GetFlagStatus(DMA2_Stream7, DMA_FLAG_TCIF7) == RESET)
    ;
  DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7);
  DMA_Cmd(DMA2_Stream7, DISABLE);
  DMA_SetCurrDataCounter(DMA2_Stream7, length);
  DMA2_Stream7->M0AR = (u32)data;
  DMA_Cmd(DMA2_Stream7, ENABLE);
}

void execute_command(const char *command) {}

void uart_process_received_data(u8 half) {
  static char cmd_buffer[MAX_CMD_LENGTH];
  static u8 cmd_idx = 0;

  u16 start_pos, end_pos;

  if (half == 0) {
    // Process first half of buffer (0 to DMA_BUFFER_SIZE/2 - 1)
    start_pos = 0;
    end_pos = DMA_BUFFER_SIZE / 2;
  } else {
    // Process second half of buffer (DMA_BUFFER_SIZE/2 to DMA_BUFFER_SIZE - 1)
    start_pos = DMA_BUFFER_SIZE / 2;
    end_pos = DMA_BUFFER_SIZE;
  }

  for (u16 i = start_pos; i < end_pos; i++) {
    u8 received_byte = dma_rx_buffer[i];

    uart_send_data(&received_byte, 1);

    if (received_byte == '\n' || received_byte == '\r') {
      if (cmd_idx > 0) {
        cmd_buffer[cmd_idx] = '\0';
        execute_command(cmd_buffer);
        cmd_idx = 0;
      }
    } else if (cmd_idx < MAX_CMD_LENGTH - 1) {
      cmd_buffer[cmd_idx++] = received_byte;
    }
  }
}

// dma transmit stream interrupt handler
// handles transfer complete interrupt for uart tx
void DMA2_Stream7_IRQHandler() {
  // check if transfer complete interrupt occurred
  if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7)) {
    // clear the transfer complete interrupt flag
    DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
  }
}

// dma receive stream interrupt handler
// handles both half transfer and transfer complete interrupts for uart rx
void DMA2_Stream2_IRQHandler() {
  // check for half transfer interrupt
  if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_HTIF2)) {
    // clear the half transfer interrupt flag
    DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_HTIF2);

    // process the first half of the buffer
    uart_process_received_data(0);
  }

  // check for transfer complete interrupt
  if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF2)) {
    // clear the transfer complete interrupt flag
    DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF2);

    // process the second half of the buffer
    uart_process_received_data(1);
  }
}