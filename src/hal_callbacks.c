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

// Weak definitions for HAL callback functions that may be missing
__weak void HAL_TIMEx_CommutCallback(TIM_HandleTypeDef *htim) {
  // NOTE : This function Should not be modified, when the callback is needed,
  //        the HAL_TIMEx_CommutCallback could be implemented in the user file
  (void)htim;
}

__weak void HAL_TIMEx_BreakCallback(TIM_HandleTypeDef *htim) {
  // NOTE : This function Should not be modified, when the callback is needed,
  //        the HAL_TIMEx_BreakCallback could be implemented in the user file
  (void)htim;
}