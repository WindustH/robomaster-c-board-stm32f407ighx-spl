#include "it.h"
#include "led.h"
#include "stm32f4xx.h"
#include "utils.h"
#include <math.h>

static volatile f32 brightness_phase = 0.0f;
static volatile f32 hue_phase = 0.0f;
static const f32 PI = 3.14159265359;
static const f32 brightness_step = PI / 200.0f;
static const f32 hue_step = 0.001f;

void TIM3_IRQHandler(void) {
  // check if update interrupt flag is set
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
    // clear the interrupt pending bit
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    // update phase
    brightness_phase += brightness_step;
    hue_phase += hue_step;

    // switch to next color
    if (brightness_phase > 3.5 * PI)
      brightness_phase -= 2 * PI;
    if (hue_phase > 1.0)
      hue_phase -= 1.0;

    u8 brightness = f32_to_u8((sinf(brightness_phase) + 1) / 2);
    u32 color = (brightness << 24) | hue_to_rgb(hue_phase);

    led_show(color);
  }
}

volatile u8 uart_rx_data = 0x00;
volatile u8 uart_rx_flag = 0;

void USART1_IRQHandler(void) {
  if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
    uart_rx_data = USART_ReceiveData(USART1);
    uart_rx_flag = 1;
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }
}

void DMA2_Stream7_IRQHandler(void) {
  if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7)) {
    DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
  }
}