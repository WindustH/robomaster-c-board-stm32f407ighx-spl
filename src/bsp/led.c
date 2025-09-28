#include "mod/bsp.h"
#include "stm32f4xx.h"

static void setup_led() {
  // 1. Enable peripheral clocks
  RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;  // TIM5 clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN; // GPIOH clock

  // 2. Configure PH10, PH11, PH12 as Alternate Function for TIM5
  // Clear mode bits for pins 10, 11, 12
  GPIOH->MODER &=
      ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11 | GPIO_MODER_MODER12);
  // Set mode to Alternate Function (10)
  GPIOH->MODER |=
      (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1);
  // Configure output type as Push-Pull
  GPIOH->OTYPER &= ~(GPIO_OTYPER_OT10 | GPIO_OTYPER_OT11 | GPIO_OTYPER_OT12);
  // Configure speed as Low
  GPIOH->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR10 | GPIO_OSPEEDER_OSPEEDR11 |
                      GPIO_OSPEEDER_OSPEEDR12);
  // Configure no pull-up, pull-down
  GPIOH->PUPDR &=
      ~(GPIO_PUPDR_PUPDR10 | GPIO_PUPDR_PUPDR11 | GPIO_PUPDR_PUPDR12);
  // Configure Alternate Function AF2 for TIM5
  GPIOH->AFR[1] &=
      ~((0xF << GPIO_AFRH_AFSEL10_Pos) | (0xF << GPIO_AFRH_AFSEL11_Pos) |
        (0xF << GPIO_AFRH_AFSEL12_Pos));
  GPIOH->AFR[1] |=
      ((2 << GPIO_AFRH_AFSEL10_Pos) | (2 << GPIO_AFRH_AFSEL11_Pos) |
       (2 << GPIO_AFRH_AFSEL12_Pos));

  // 3. Configure TIM5 for PWM
  TIM5->PSC = 41;           // Prescaler
  TIM5->ARR = 255;          // Auto-reload value
  TIM5->CR1 = TIM_CR1_ARPE; // Set counter mode to up-counting, enable
                            // auto-reload preload

  // 4. Configure PWM channels (1, 2, 3)
  // Channel 1 (PH10) - PWM mode 1, preload enabled
  TIM5->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE);
  // Channel 2 (PH11) - PWM mode 1, preload enabled
  TIM5->CCMR1 |= (TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE);
  // Channel 3 (PH12) - PWM mode 1, preload enabled
  TIM5->CCMR2 |= (TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE);

  // 5. Enable channel outputs
  TIM5->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;

  // 6. Enable TIM5 counter
  TIM5->CR1 |= TIM_CR1_CEN;

  // Set initial LED state to off
  bsp.led.show(0x00000000);
}

static void change_color(const u32 argb) {
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
  TIM5->CCR1 = blue;
  TIM5->CCR2 = green;
  TIM5->CCR3 = red;
}

const _LedMod _led = {.setup = setup_led, .show = change_color};