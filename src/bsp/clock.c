#include "bsp.h"
#include "stm32f4xx.h"

void setup_impl() {
  // 1. Enable PWR clock and set voltage scaling
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CR |= PWR_CR_VOS; // Scale 1

  // 2. Enable HSE and wait for it to be ready
  RCC->CR |= RCC_CR_HSEON;
  while (!(RCC->CR & RCC_CR_HSERDY))
    ;

  // 3. Configure the PLL
  // Clear the register
  RCC->PLLCFGR &=
      ~(RCC_PLLCFGR_PLLSRC_Msk | RCC_PLLCFGR_PLLM_Msk | RCC_PLLCFGR_PLLN_Msk |
        RCC_PLLCFGR_PLLP_Msk | RCC_PLLCFGR_PLLQ_Msk);
  // Set PLL source to HSE, PLLM = 12, PLLN = 336, PLLP = 2, PLLQ = 7
  RCC->PLLCFGR = (RCC_PLLCFGR_PLLSRC_HSE) | (12 << RCC_PLLCFGR_PLLM_Pos) |
                 (336 << RCC_PLLCFGR_PLLN_Pos) |
                 (0 << RCC_PLLCFGR_PLLP_Pos) | // RCC_PLLP_DIV2
                 (7 << RCC_PLLCFGR_PLLQ_Pos);

  // Enable the PLL
  RCC->CR |= RCC_CR_PLLON;
  while (!(RCC->CR & RCC_CR_PLLRDY))
    ;

  // 4. Configure Flash Latency
  // Set flash latency to 5 wait states for 168MHz
  FLASH->ACR = FLASH_ACR_LATENCY_5WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |
               FLASH_ACR_DCEN;

  // 5. Configure Bus Prescalers
  // AHB prescaler = 1, APB1 prescaler = 4, APB2 prescaler = 2
  RCC->CFGR |=
      (RCC_CFGR_HPRE_DIV1) | (RCC_CFGR_PPRE1_DIV4) | (RCC_CFGR_PPRE2_DIV2);

  // 6. Switch System Clock to PLL
  RCC->CFGR |= RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
    ;
  SystemCoreClockUpdate();
}

const _ClockMod _clock = {
    .setup = setup_impl,
};