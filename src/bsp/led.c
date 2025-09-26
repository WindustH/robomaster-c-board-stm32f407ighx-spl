#include "bsp.h"
#include "stm32f4xx_hal.h"

static TIM_HandleTypeDef htim5;

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *timHandle);

static void setup_impl() {
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 41;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 255;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK) {
    while (1)
      ;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK) {
    while (1)
      ;
  }
  if (HAL_TIM_PWM_Init(&htim5) != HAL_OK) {
    while (1)
      ;
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK) {
    while (1)
      ;
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
    while (1)
      ;
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
    while (1)
      ;
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
    while (1)
      ;
  }

  HAL_TIM_MspPostInit(&htim5);

  // Start PWM channels
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);

  // Set initial LED state to off
  bsp.led.show(0x00000000);
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *timHandle) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (timHandle->Instance == TIM5) {
    __HAL_RCC_GPIOH_CLK_ENABLE();
    /**TIM5 GPIO Configuration
    PH10     ------> TIM5_CH1
    PH11     ------> TIM5_CH2
    PH12     ------> TIM5_CH3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
  }
}

static void show_impl(const u32 argb) {
  u32 alpha;
  u32 red, green, blue;

  // Extract channels
  alpha = (argb & 0xFF000000) >> 24;
  red = (argb & 0x00FF0000) >> 16;
  green = (argb & 0x0000FF00) >> 8;
  blue = (argb & 0x000000FF) >> 0;

  red = red * alpha / 255;
  green = green * alpha / 255;
  blue = blue * alpha / 255;

  // Set timer 5 to output PWM wave
  __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, blue);
  __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, green);
  __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, red);
}

const _LedMod _led = {.setup = setup_impl, .show = show_impl};