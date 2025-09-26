#include "bsp/uart_dma.h"
#include "bsp.h"
#include "stm32f4xx_hal.h"
#include <string.h>

// External UART handle from uart.c
extern UART_HandleTypeDef huart1;

// DMA transmit buffer for UART data
static volatile u8 dma_tx_buf[DMA_BUFFER_SIZE];
// DMA receive buffer for UART data
static volatile u8 dma_rx_buf[DMA_BUFFER_SIZE];
// DMA transmit buffer for echo
static volatile buf *app_rx_buf;
static volatile u8 tx_in_progress = 0;

// Handle for DMA
static DMA_HandleTypeDef hdma_usart1_tx;
static DMA_HandleTypeDef hdma_usart1_rx;

// Init DMA for UART
// Config both tx and rx streams with appropriate settings
static void setup_impl(volatile buf *const _app_rx_buf) {
  // Check for null pointer
  if (_app_rx_buf == NULL) {
    return;
  }

  app_rx_buf = _app_rx_buf;
  app_rx_buf->len = 0;

  // Enable DMA clock
  __HAL_RCC_DMA2_CLK_ENABLE();

  // Config DMA for UART TX (stream7, channel4)
  hdma_usart1_tx.Instance = DMA2_Stream7;
  hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
  hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart1_tx.Init.Mode = DMA_NORMAL;
  hdma_usart1_tx.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK) {
    // Error handling
    while (1)
      ;
  }

  // Config DMA for UART RX (stream2, channel4)
  // Using circular mode for continuous reception
  hdma_usart1_rx.Instance = DMA2_Stream2;
  hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
  hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
  hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK) {
    // Error handling
    while (1)
      ;
  }

  // Associate DMA handles with UART handle
  __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);
  __HAL_LINKDMA(&huart1, hdmarx, hdma_usart1_rx);

  // Initialize buffers to zero
  memset((void *)dma_tx_buf, 0, DMA_BUFFER_SIZE);
  memset((void *)dma_rx_buf, 0, DMA_BUFFER_SIZE);

  // Enable transfer complete interrupt for TX
  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

  // Enable half transfer and transfer complete interrupts for RX
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

  // Start DMA reception
  HAL_UART_Receive_DMA(&huart1, (uint8_t *)dma_rx_buf, DMA_BUFFER_SIZE);
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
  HAL_UART_Transmit_DMA(&huart1, (uint8_t *)dma_tx_buf, length);
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
  u16 current_pos = DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);

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

// DMA transmit stream interrupt handler
// Handles transfer complete interrupt for UART TX
void DMA2_Stream7_IRQHandler() {
  HAL_DMA_IRQHandler(&hdma_usart1_tx);
}

// DMA receive stream interrupt handler
// Handles both half transfer and transfer complete interrupts for UART RX
void DMA2_Stream2_IRQHandler() {
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
}

// Callback for DMA transfer complete
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    tx_in_progress = 0;
  }
}

// Callback for DMA half transfer and transfer complete for RX
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    bsp.uart.dma.rxbuf_daemon();
  }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    bsp.uart.dma.rxbuf_daemon();
  }
}

const _UartDmaMod _uart_dma = {.send_dat = send_dat_impl,
                               .send_str = send_str_impl,
                               .setup = setup_impl,
                               .rxbuf_daemon = save_rx_dat_to_buf};