#include "bsp/uart_dma.h"
#include "bsp.h"
#include "stm32f4xx_conf.h"
#include <string.h>

// dma transmit buffer for uart data
static volatile u8 dma_tx_buf[DMA_BUFFER_SIZE];
// dma receive buffer for uart data
static volatile u8 dma_rx_buf[DMA_BUFFER_SIZE];
// dma transmit buffer for echo
static volatile buf *app_rx_buf;
static volatile u8 tx_in_progress = 0;

// init dma for uart
// config both tx and rx streams with appropriate settings
static void setup_impl(volatile buf *const _app_rx_buf) {
  // Check for null pointer
  if (_app_rx_buf == NULL) {
    return;
  }

  app_rx_buf = _app_rx_buf;
  app_rx_buf->len = 0;

  DMA_InitTypeDef DMA_InitStructure;

  // enable dma2 clock
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

  // config dma for uart tx (stream7, channel4)
  DMA_DeInit(DMA2_Stream7);
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)dma_tx_buf;
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
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)dma_rx_buf;
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
  memset((void *)dma_tx_buf, 0, DMA_BUFFER_SIZE);
  memset((void *)dma_rx_buf, 0, DMA_BUFFER_SIZE);

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

static void send_dat_impl(const u8 *data, const u16 length) {
  // Check for null pointer and buffer overflow
  if (data == NULL || length == 0 || length > DMA_BUFFER_SIZE) {
    return;
  }

  while (tx_in_progress)
    ;
  tx_in_progress = 1;

  memcpy((void *)dma_tx_buf, data, length);
  DMA_Cmd(DMA2_Stream7, DISABLE);
  DMA_SetCurrDataCounter(DMA2_Stream7, length);
  DMA_Cmd(DMA2_Stream7, ENABLE);
}

static void send_str_impl(const char *str) {
  // Check for null pointer
  if (str == NULL) {
    return;
  }

  u16 i;
  for (i = 0; str[i] != '\0' && i < DMA_BUFFER_SIZE; i++)
    ;

  // Only send if string fits in buffer
  if (i < DMA_BUFFER_SIZE) {
    send_dat_impl((u8 *)str, i);
  }
}

static void save_rx_dat_to_buf() {
  static u16 last_read_pos = 0;
  u16 current_pos = DMA_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA2_Stream2);

  if (current_pos != last_read_pos) {
    u16 bytes_to_copy = current_pos - last_read_pos;

    // Check if we have enough space in the app buffer
    if ((app_rx_buf->len + bytes_to_copy) <= DMA_BUFFER_SIZE) {
      // With ht and tc interrupts, current_pos is always > last_read_pos
      // no dma buffer wrap-around to handle
      memcpy((void *)&app_rx_buf->dat[app_rx_buf->len],
             (void *)&dma_rx_buf[last_read_pos], bytes_to_copy);
      app_rx_buf->len += bytes_to_copy;
    }

    last_read_pos = current_pos;
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
    bsp.uart.dma.rxbuf_daemon();
  }

  if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF2)) {
    DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF2);
    bsp.uart.dma.rxbuf_daemon();
  }
}

const _UartDmaMod _uart_dma = {.send_dat = send_dat_impl,
                               .send_str = send_str_impl,
                               .setup = setup_impl,
                               .rxbuf_daemon = save_rx_dat_to_buf};