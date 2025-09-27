#include "bsp.h"
#include "stm32f4xx.h"
#include "type.h"

static u16 calc_brr(u32 bus_clock, u32 target_baud_rate) {
  // Validate inputs
  if (bus_clock == 0 || target_baud_rate == 0) {
    return 0; // Invalid input
  }

  // Calculate exact division factor
  f64 div_factor = (f64)bus_clock / (16.0 * target_baud_rate);

  // Extract integer and fractional parts
  u16 brr_int = (u16)div_factor;
  f64 frac_part = div_factor - brr_int;

  // Calculate 4-bit fractional part (multiplied by 16)
  u16 brr_frac = (u16)(frac_part * 16 + 0.5); // +0.5 for rounding

  // Limit fractional part to 4 bits (0-15)
  if (brr_frac > 15) {
    brr_frac = 15;
  }

  // Combine into BRR register value
  u16 brr = (brr_int << 4) | brr_frac;

  return brr;
}

static void setup_impl() {
  // 1. Enable clocks
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  // 2. GPIO Configuration
  // PA9 (TX)
  GPIOA->MODER &= ~GPIO_MODER_MODER9;
  GPIOA->MODER |= GPIO_MODER_MODER9_1;      // Alternate function
  GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR9; // Very high speed
  GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL9;
  GPIOA->AFR[1] |= (7 << GPIO_AFRH_AFSEL9_Pos); // AF7 for USART1

  // PB7 (RX)
  GPIOB->MODER &= ~GPIO_MODER_MODER7;
  GPIOB->MODER |= GPIO_MODER_MODER7_1;      // Alternate function
  GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7; // Very high speed
  GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR7;
  GPIOB->PUPDR |= GPIO_PUPDR_PUPDR7_0; // Pull-up
  GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL7;
  GPIOB->AFR[0] |= (7 << GPIO_AFRL_AFSEL7_Pos); // AF7 for USART1

  // 3. UART Configuration
  // Assuming a 168MHz APB2 clock, for a baud rate of 38400:
  // USARTDIV = 168,000,000 / (16 * 38400) = 273.4375
  // DIV_Mantissa = 273
  // DIV_Fraction = 0.4375 * 16 = 7
  // BRR value = (273 << 4) | 7 = 0x1117
  USART1->BRR = calc_brr(APB2_CLK, USART_BAUD_RATE);

  // Enable TX, RX, and the UART peripheral
  USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

_UartMod _uart = {
    .setup = setup_impl,
};

void init_uart_mod() {
  _uart.dma = _uart_dma;
  _uart.it = _uart_it;
}