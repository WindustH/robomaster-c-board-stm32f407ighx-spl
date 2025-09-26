#include "bsp/cron.h"
#include "app/cron_job.h"
#include "bsp.h"
#include "stm32f4xx_hal.h"
#include "type.h"

static TIM_HandleTypeDef htim3;
static volatile procList proc_list = {.state = 0, .procs = {NULL}};

static void setup_impl() {
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = TIM3_PRESCALER;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = TIM3_PERIOD;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
    while (1)
      ;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
    while (1)
      ;
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
    while (1)
      ;
  }

  // Start timer 3 interrupts
  if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK) {
    // Error handling
    while (1)
      ;
  }
}

static u8 add_cron_job(const proc p) {
  // Check for null pointer
  if (p == NULL) {
    return PROC_LIST_SIZE;
  }

  // Use interrupt control to prevent race condition
  for (u8 i = 0; i < PROC_LIST_SIZE; i++) {
    // Fix the bitwise operation to check if the bit is 0 (available)
    if (!(proc_list.state & (1U << i))) {
      proc_list.procs[i] = p;
      proc_list.state |= (1U << i);
      return i;
    }
  }
  return PROC_LIST_SIZE;
}

static void remove_cron_job(const u8 idx) {
  // Check bounds
  if (idx < PROC_LIST_SIZE) {
    // Use interrupt control to prevent race condition
    proc_list.state &= ~(1U << idx);
  }
}

const _CronMod _cron = {.setup = setup_impl,
                        .add_job = add_cron_job,
                        .remove_job = remove_cron_job};

void TIM3_IRQHandler() {
  HAL_TIM_IRQHandler(&htim3);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM3) {
    for (u8 i = 0; i < PROC_LIST_SIZE; i++) {
      if ((proc_list.state & (1U << i)) && (proc_list.procs[i] != NULL)) {
        proc_list.procs[i]();
      }
    }
  }
}