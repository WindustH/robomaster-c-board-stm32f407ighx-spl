#include "bsp.h"
#include "stm32f4xx.h"
#include <string.h>

// DMA transmit buffer for UART data
static volatile u8 dma_tx_buf[DMA_BUFFER_SIZE];
// DMA receive buffer for UART data
static volatile u8 dma_rx_buf[DMA_BUFFER_SIZE];
// DMA transmit buffer for echo
static volatile buf *app_rx_buf;
static volatile u8 tx_in_progress = 0;

// Init DMA for UART
// Config both tx and rx streams with appropriate settings
static void setup_impl(volatile buf *const _app_rx_buf) {
  // Check for null pointer
  if (_app_rx_buf == NULL) {
    return;
  }

  app_rx_buf = _app_rx_buf;
  app_rx_buf->len = 0;

  // 1. Enable DMA2 clock
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

  // 2. Configure DMA for UART TX (DMA2, Stream 7, Channel 4)
  // Disable the stream first
  DMA2_Stream7->CR &= ~DMA_SxCR_EN;
  // Wait for EN bit to be cleared
  while (DMA2_Stream7->CR & DMA_SxCR_EN)
    ;
  // Set peripheral port register address
  DMA2_Stream7->PAR = (u32)&USART1->DR;
  // Set memory address
  DMA2_Stream7->M0AR = (u32)dma_tx_buf;
  // Set number of data items to transfer
  DMA2_Stream7->NDTR = 0; // Will be set before each transfer
  // Configure stream: channel 4, memory-to-peripheral, memory increment,
  // byte data alignment, normal mode, high priority, transfer complete
  // interrupt
  DMA2_Stream7->CR = (4 << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_DIR_0 |
                     DMA_SxCR_MINC | DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0 |
                     DMA_SxCR_PL_1 | DMA_SxCR_TCIE;

  // 3. Configure DMA for UART RX (DMA2, Stream 2, Channel 4)
  // Disable the stream first
  DMA2_Stream2->CR &= ~DMA_SxCR_EN;
  // Wait for EN bit to be cleared
  while (DMA2_Stream2->CR & DMA_SxCR_EN)
    ;
  // Set peripheral port register address
  DMA2_Stream2->PAR = (u32)&USART1->DR;
  // Set memory address
  DMA2_Stream2->M0AR = (u32)dma_rx_buf;
  // Set number of data items to transfer
  DMA2_Stream2->NDTR = DMA_BUFFER_SIZE;
  // Configure stream: channel 4, peripheral-to-memory, memory increment,
  // byte data alignment, circular mode, high priority, half & full transfer
  // interrupts
  DMA2_Stream2->CR = (4 << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MINC |
                     DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0 | DMA_SxCR_CIRC |
                     DMA_SxCR_PL_1 | DMA_SxCR_TCIE | DMA_SxCR_HTIE;

  // 4. Enable DMA interrupts in NVIC
  NVIC_SetPriority(DMA2_Stream7_IRQn, 0);
  NVIC_EnableIRQ(DMA2_Stream7_IRQn);
  NVIC_SetPriority(DMA2_Stream2_IRQn, 1);
  NVIC_EnableIRQ(DMA2_Stream2_IRQn);

  // 5. Enable DMA mode in UART
  USART1->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;

  // 6. Start DMA reception
  DMA2_Stream2->CR |= DMA_SxCR_EN;
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

  // Disable the stream
  DMA2_Stream7->CR &= ~DMA_SxCR_EN;
  while (DMA2_Stream7->CR & DMA_SxCR_EN)
    ;

  // Set data length
  DMA2_Stream7->NDTR = length;

  // Clear transfer complete flag
  DMA2->HIFCR = DMA_HIFCR_CTCIF7;

  // Enable the stream
  DMA2_Stream7->CR |= DMA_SxCR_EN;
}

static void send_str_impl(const char *str) {
  // Check for null pointer
  if (str == NULL) {
    return;
  }

  u16 len = strlen(str);

  // Only send if string fits in buffer
  if (len > 0 && len < DMA_BUFFER_SIZE) {
    send_dat_impl((u8 *)str, len);
  }
}

static void save_rx_dat_to_buf() {
  static u16 last_read_pos = 0;
  u16 current_pos = DMA_BUFFER_SIZE - DMA2_Stream2->NDTR;

  if (current_pos != last_read_pos) {
    u16 bytes_to_copy;
    if (current_pos > last_read_pos) {
      bytes_to_copy = current_pos - last_read_pos;
    } else { // Wrap around
      bytes_to_copy = DMA_BUFFER_SIZE - last_read_pos + current_pos;
    }

    // Check if we have enough space in the app buffer
    if ((app_rx_buf->len + bytes_to_copy) <= DMA_BUFFER_SIZE) {
      if (current_pos > last_read_pos) {
        memcpy((void *)&app_rx_buf->dat[app_rx_buf->len],
               (void *)&dma_rx_buf[last_read_pos], bytes_to_copy);
      } else {
        u16 first_chunk = DMA_BUFFER_SIZE - last_read_pos;
        memcpy((void *)&app_rx_buf->dat[app_rx_buf->len],
               (void *)&dma_rx_buf[last_read_pos], first_chunk);
        app_rx_buf->len += first_chunk;
        memcpy((void *)&app_rx_buf->dat[app_rx_buf->len], (void *)dma_rx_buf,
               current_pos);
        bytes_to_copy = first_chunk + current_pos;
      }
      app_rx_buf->len += bytes_to_copy;
    }

    last_read_pos = current_pos;
  }
}

// DMA transmit stream interrupt handler
void DMA2_Stream7_IRQHandler() {
  if (DMA2->HISR & DMA_HISR_TCIF7) {
    // Clear transfer complete flag
    DMA2->HIFCR = DMA_HIFCR_CTCIF7;
    tx_in_progress = 0;
  }
}

// DMA receive stream interrupt handler
void DMA2_Stream2_IRQHandler() {
  if (DMA2->LISR & (DMA_LISR_TCIF2 | DMA_LISR_HTIF2)) {
    // Clear flags
    DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2;
    bsp.uart.dma.rxbuf_daemon();
  }
}

const _UartDmaMod _uart_dma = {.send_dat = send_dat_impl,
                               .send_str = send_str_impl,
                               .setup = setup_impl,
                               .rxbuf_daemon = save_rx_dat_to_buf};