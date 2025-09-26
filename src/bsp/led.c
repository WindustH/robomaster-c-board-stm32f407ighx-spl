#include "bsp.h"
#include "stm32f4xx_hal.h"

static TIM_HandleTypeDef htim5;

static void setup_impl() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Enable clock for timer 5 and GPIOH
  __HAL_RCC_TIM5_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  // GPIO config for LED
  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  // Timer configuration for PWM output
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 42 - 1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 255;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim5) != HAL_OK) {
    // Error handling
    while (1)
      ;
  }

  // PWM configuration
  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
    // Error handling
    while (1)
      ;
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
    // Error handling
    while (1)
      ;
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
    // Error handling
    while (1)
      ;
  }

  // Start PWM channels
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);

  // Set initial LED state to off
  bsp.led.show(0x00000000);
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