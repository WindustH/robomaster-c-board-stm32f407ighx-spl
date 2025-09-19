#include "uart_dma.h"
#include "setup.h"
#include "stm32f4xx.h"
#define MAX_CMD_LENGTH 50

volatile u16 lastReceivedCount = 0;
extern u8 dma_tx_buffer[DMA_BUFFER_SIZE];
extern u8 dma_rx_buffer[DMA_BUFFER_SIZE];

void uart_send_data(u8 *data, u16 length) {
  while (DMA_GetFlagStatus(DMA2_Stream7, DMA_FLAG_TCIF7) == RESET)
    ;
  DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7);
  DMA_Cmd(DMA2_Stream7, DISABLE);
  DMA_SetCurrDataCounter(DMA2_Stream7, length);
  DMA2_Stream7->M0AR = (u32)data;
  DMA_Cmd(DMA2_Stream7, ENABLE);
}

void HandleCommand(const char *command) {}

void uart_process_received_data(void) {
  // 获取当前DMA计数器值
  uint16_t currentCounter = DMA_GetCurrDataCounter(DMA2_Stream2);

  // 计算新接收到的字节数
  uint16_t newDataCount =
      (lastReceivedCount >= currentCounter)
          ? (lastReceivedCount - currentCounter)
          : (DMA_BUFFER_SIZE - currentCounter + lastReceivedCount);

  if (newDataCount > 0) {
    // 更新最后一次计数
    lastReceivedCount = currentCounter;

    // 计算接收数据的起始位置
    uint16_t startPos =
        (DMA_BUFFER_SIZE - currentCounter - newDataCount) % DMA_BUFFER_SIZE;

    // 临时缓冲区用于存储完整命令
    static char commandBuffer[MAX_CMD_LENGTH];
    static uint8_t cmdIndex = 0;

    // 处理新接收到的数据
    for (uint16_t i = 0; i < newDataCount; i++) {
      uint16_t pos = (startPos + i) % DMA_BUFFER_SIZE;
      uint8_t receivedChar = dma_rx_buffer[pos];

      // 示例1: 回显接收到的字符
      // USART_SendData_DMA(&receivedChar, 1);

      // 示例2: 命令解析 (以换行符为结束标志)
      if (receivedChar == '\n' || receivedChar == '\r') {
        if (cmdIndex > 0) // 确保有命令内容
        {
          commandBuffer[cmdIndex] = '\0'; // 添加字符串结束符
          HandleCommand(commandBuffer);   // 处理命令
          cmdIndex = 0;                   // 重置索引
        }
      } else if (cmdIndex < MAX_CMD_LENGTH - 1) // 防止缓冲区溢出
      {
        commandBuffer[cmdIndex++] = receivedChar;
      }
    }

    // 示例3: 原始数据处理 (直接转发)
    // USART_SendData_DMA(&RxBuffer[startPos], newDataCount);

    // 示例4: 数据包解析 (假设有固定长度包头)
    /*
    if(newDataCount >= 4) { // 假设包头4字节
        uint8_t header[4];
        for(int i=0; i<4; i++) {
            uint16_t pos = (startPos + i) % BUFFER_SIZE;
            header[i] = RxBuffer[pos];
        }
        // 解析包头...
    }
    */
  }
}