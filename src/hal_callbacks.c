#include "stm32f4xx_hal.h"

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle) {
  if (tim_baseHandle->Instance == TIM3) {
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* TIM3 interrupt Init */
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  } else if (tim_baseHandle->Instance == TIM5) {
    /* TIM5 clock enable */
    __HAL_RCC_TIM5_CLK_ENABLE();
  }
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle) {

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (canHandle->Instance == CAN1) {

    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  }
}