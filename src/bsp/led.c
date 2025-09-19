#include "bsp/led.h"
#include "stm32f4xx_conf.h"

void setup_led() {
  // enable clock(42000000Hz) for timer 5 (pwm outpput)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  // enable clock for gpioh
  // (pins to control led)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);

  // gpio config for led
  GPIO_InitTypeDef GPIO_InitStructure;
  // pin 10,11,12 of port h (gpioh)
  // (pins to control led)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  // set to alternate function mode because these pins output pwm waves from
  // timer 5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  // set to high speed
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  // set to push-pull output type to drive led better
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  // enable pull-up resistor so the led is off by default
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  // apply conf to gpioh
  GPIO_Init(GPIOH, &GPIO_InitStructure);
  // connect timer 5 to gpioh
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource10, GPIO_AF_TIM5);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource11, GPIO_AF_TIM5);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource12, GPIO_AF_TIM5);

  // timer output compare structure (for pwm)
  TIM_OCInitTypeDef TIM_OCInitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  // config timer 5 for pwm output
  // auto-reload value
  // define the period of pwm
  TIM_TimeBaseStructure.TIM_Period = 255;
  // prescale the timer to 1Mhz
  TIM_TimeBaseStructure.TIM_Prescaler = 42 - 1;
  // no clock division
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  // count from 0 to period-1 and repeat
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  // apply conf to timer 5
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
  // config output compare for pwm
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  // duty cycle (led off)
  TIM_OCInitStructure.TIM_Pulse = 0;
  // high <-> active
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  // apply output config for 3 channels of timer 5
  TIM_OC1Init(TIM5, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Enable);
  TIM_OC2Init(TIM5, &TIM_OCInitStructure);
  TIM_OC2PreloadConfig(TIM5, TIM_OCPreload_Enable);
  TIM_OC3Init(TIM5, &TIM_OCInitStructure);
  TIM_OC3PreloadConfig(TIM5, TIM_OCPreload_Enable);
  // enable auto-reload preload for timer 5
  TIM_ARRPreloadConfig(TIM5, ENABLE);

  // enable pwm outputs of timer 5 (a basic timer), for led control
  TIM_CtrlPWMOutputs(TIM5, ENABLE);
  TIM_Cmd(TIM5, ENABLE);
  // set initial led state to off
  led_show(0x00000000);
}

void led_show(u32 argb) {
  u32 alpha;
  u32 red, green, blue;

  // extract channels
  alpha = (argb & 0xFF000000) >> 24;
  red = (argb & 0x00FF0000) >> 16;
  green = (argb & 0x0000FF00) >> 8;
  blue = (argb & 0x000000FF) >> 0;

  red = red * alpha / 255;
  green = green * alpha / 255;
  blue = blue * alpha / 255;

  // set timer 5 to output pwm wave
  TIM_SetCompare1(TIM5, blue);
  TIM_SetCompare2(TIM5, green);
  TIM_SetCompare3(TIM5, red);
}