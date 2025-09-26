#include "stm32f4xx_hal.h"

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