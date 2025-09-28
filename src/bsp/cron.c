#include "mod/bsp.h"
#include "stm32f4xx.h"
#include "type.h"

static volatile procList proc_list = {.state = 0, .procs = {NULL}};

static void setup_cron() {
  // 1. Enable TIM3 clock
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

  // 2. Configure TIM3
  // Set prescaler and auto-reload period
  TIM3->PSC = TIM3_PRESCALER;
  TIM3->ARR = TIM3_PERIOD;

  // 3. Enable Update Interrupt
  TIM3->DIER |= TIM_DIER_UIE;

  // 4. Enable TIM3 interrupt in NVIC
  NVIC_SetPriority(TIM3_IRQn, 0);
  NVIC_EnableIRQ(TIM3_IRQn);

  // 5. Enable the timer
  TIM3->CR1 |= TIM_CR1_CEN;
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

const _CronMod _cron = {.setup = setup_cron,
                        .add_job = add_cron_job,
                        .remove_job = remove_cron_job};

void TIM3_IRQHandler() {
  // Check if the update interrupt flag is set
  if (TIM3->SR & TIM_SR_UIF) {
    // Clear the update interrupt flag
    TIM3->SR &= ~TIM_SR_UIF;

    // Execute cron jobs
    for (u8 i = 0; i < PROC_LIST_SIZE; i++) {
      if ((proc_list.state & (1U << i)) && (proc_list.procs[i] != NULL)) {
        proc_list.procs[i]();
      }
    }
  }
}