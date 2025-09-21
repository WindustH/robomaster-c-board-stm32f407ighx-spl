#include "bsp/cron.h"
#include "app/cron_job.h"
#include "bsp.h"
#include "stm32f4xx_conf.h"
#include "type.h"
#include "ut/uthash.h"

static volatile procList proc_list;
static void setup_impl() {
  // enable apb1 peripherial clocks (42000000Hz) for timer 3
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  // config timer 3
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  // prescale timer 3 to 100kHz
  TIM_TimeBaseStructure.TIM_Prescaler = TIM3_PRESCALER;
  TIM_TimeBaseStructure.TIM_Period = TIM3_PERIOD;
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

static u8 add_cron_job_impl(const proc p) {
  for (u8 i = 0; i < PROC_LIST_SIZE; i++) {
    if (~proc_list.state & (1U << i)) {
      proc_list.procs[i] = p;
      proc_list.state |= (1U << i);
      return i;
    }
  }
  return PROC_LIST_SIZE;
}

static void remove_cron_job_impl(const u8 idx) {
  proc_list.state &= ~(1U << idx);
}

const _CronMod _cron = {.setup = setup_impl,
                        .add_job = add_cron_job_impl,
                        .remove_job = remove_cron_job_impl};

void TIM3_IRQHandler() {
  // check if update interrupt flag is set
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
    for (u8 i = 0; i < PROC_LIST_SIZE; i++)
      if (proc_list.state & (1U << i))
        proc_list.procs[i]();
  }
}