#include "bsp.h"
#include "stm32f4xx_hal.h"

static CAN_HandleTypeDef hcan1;

static void setup_impl(void) {
  hcan1.Instance = CAN1;

  // set to 1mbps
  hcan1.Init.Prescaler = 3;
  hcan1.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;

  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;

  // init can
  if (HAL_CAN_Init(&hcan1) != HAL_OK) {
    while (1)
      ;
  }

  // start can
  if (HAL_CAN_Start(&hcan1) != HAL_OK) {
    while (1)
      ;
  }

  // enable interrupt notification
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) !=
      HAL_OK) {
    while (1)
      ;
  }
}

static HAL_StatusTypeDef transmit_impl(CAN_TxHeaderTypeDef *pHeader, u8 aData[],
                                       u32 *pTxMailbox) {
  return HAL_CAN_AddTxMessage(&hcan1, pHeader, aData, pTxMailbox);
}

static HAL_StatusTypeDef receive_impl(CAN_RxHeaderTypeDef *pHeader,
                                      u8 aData[]) {
  return HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, pHeader, aData);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  u8 rxData[8];
  CAN_RxHeaderTypeDef rxHeader;

  // 在中断中立即读取，避免 FIFO 溢出
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
    // 处理数据（建议只做简单处理，或放入队列）
  }
}

const _CanMod _can = {
    .setup = setup_impl, .send = transmit_impl, .read = receive_impl};