#include "bsp/cron.h"
#include "stm32f4xx.h"
#include "app/cron_job.h"

void setup_cron() {
  // enable apb1 peripherial clocks (42000000Hz) for timer 3
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  // config timer 3
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  // prescale timer 3 to 100kHz
  TIM_TimeBaseStructure.TIM_Prescaler = 420 - 1;
  // period 1000-1
  // 0.01 s/tick
  TIM_TimeBaseStructure.TIM_Period = 999;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

  // config nested vector interrupt controller for timer 3 interrupts
  NVIC_InitTypeDef NVIC_InitStructure;
  // config nvic for timer 3
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  // enable interrupt
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  // apply nvic config
  NVIC_Init(&NVIC_InitStructure);

  // start timer 3
  TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler() {
  // check if update interrupt flag is set
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
    light_breathe();
  }
}
