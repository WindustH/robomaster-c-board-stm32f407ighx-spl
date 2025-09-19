#include "bsp/clock.h"
#include "stm32f4xx_conf.h"

void setup_clock() {
#include "stm32f4xx_rcc.h"

  // enable hse (high speed external) oscillator
  RCC_HSEConfig(RCC_HSE_ON);

  // wait for hse to be ready
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
    ;

  // configure pll
  RCC_PLLConfig(RCC_PLLSource_HSE, 12, 336, 2, 7);
  // parameters: source, m, n, p, q
  // sysclk = ((hse / m) * n) / p
  // for 168mhz: ((8mhz / 8) * 336) / 2 = 168mhz

  // enable pll
  RCC_PLLCmd(ENABLE);

  // wait for pll to be ready
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    ;

  // configure flash latency for high speed
  FLASH_SetLatency(FLASH_Latency_5);

  // configure ahb, apb1, apb2 prescalers
  RCC_HCLKConfig(RCC_SYSCLK_Div1); // ahb = sysclk
  RCC_PCLK1Config(RCC_HCLK_Div4);  // apb1 = hclk/4 (42mhz max)
  RCC_PCLK2Config(RCC_HCLK_Div2);  // apb2 = hclk/2 (84mhz max)

  // select pll as system clock source
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

  // wait until pll is used as system clock source
  while (RCC_GetSYSCLKSource() != 0x08)
    ;
}
