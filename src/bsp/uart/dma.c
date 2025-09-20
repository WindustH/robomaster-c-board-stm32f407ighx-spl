#include "bsp/uart/dma.h"
#include "stm32f4xx_conf.h"
#include <string.h>

// dma transmit buffer for uart data
static volatile u8 dma_tx_buffer[DMA_BUFFER_SIZE];
// dma receive buffer for uart data
static volatile u8 dma_rx_buffer[DMA_BUFFER_SIZE];
// dma transmit buffer for echo
static volatile u8 dma_echo_tx_buffer[DMA_BUFFER_SIZE];
static volatile u8 tx_in_progress = 0;

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

  // config nvic for rx dma interrupt
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void uart_send_data(const u8 *data, const u16 length) {
  while (tx_in_progress)
    ;
  tx_in_progress = 1;
  memcpy((void *)dma_tx_buffer, data, length);
  DMA_Cmd(DMA2_Stream7, DISABLE);
  DMA_SetCurrDataCounter(DMA2_Stream7, length);
  DMA_Cmd(DMA2_Stream7, ENABLE);
}

void uart_send_str_dma(const char *str) {
  u16 i;
  for (i = 0; str[i] != '\0'; i++)
    ;
  uart_send_data((u8 *)str, i);
}

void uart_process_received_data() {
  static u16 last_read_pos = 0;
  u16 current_pos = DMA_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA2_Stream2);
  if (current_pos != last_read_pos) {
    u16 len = 0;
    if (current_pos > last_read_pos) {
      len = current_pos - last_read_pos;
      memcpy((void *)&dma_echo_tx_buffer[0],
             (void *)&dma_rx_buffer[last_read_pos], len);
    } else {
      len = DMA_BUFFER_SIZE - last_read_pos;
      memcpy((void *)&dma_echo_tx_buffer[0],
             (void *)&dma_rx_buffer[last_read_pos], len);
      if (current_pos > 0) {
        memcpy((void *)&dma_echo_tx_buffer[len], (void *)&dma_rx_buffer[0],
               current_pos);
        len += current_pos;
      }
    }
    last_read_pos = current_pos;
    if (len > 0) {
      uart_send_str_dma("\nsend back:\n\t");
      uart_send_data((u8 *)dma_echo_tx_buffer, len);
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
    tx_in_progress = 0;
  }
}

// dma receive stream interrupt handler
// handles both half transfer and transfer complete interrupts for uart rx
void DMA2_Stream2_IRQHandler() {
  if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_HTIF2)) {
    DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_HTIF2);
    uart_process_received_data();
  }

  if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF2)) {
    DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF2);
    uart_process_received_data();
  }
}