#include "bsp/setup.h"
#include "stm32f4xx.h"

void setup_clock() {
  // ahb prescaler->168Mhz
  RCC->CFGR &= ~RCC_CFGR_HPRE;
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

  // apb1 timer clock /4->42Mhz
  RCC->CFGR &= ~RCC_CFGR_PPRE1;
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

  // apb2 timer clock /2->84Mhz
  RCC->CFGR &= ~RCC_CFGR_PPRE2;
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
}
