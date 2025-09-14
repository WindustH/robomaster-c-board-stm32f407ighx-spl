#include "it.h"
#include "led.h"
#include "math.h"
#include "stm32f4xx.h"
#include "utils.h"

volatile u8 color_index = 0;
volatile f32 phase = 0.0f;
const f32 PI = 3.14159265359;
// 100*0.01=1s to do a breath
const f32 step = PI / 100.0f;

void TIM3_IRQHandler(void) {
    // check if update interrupt flag is set
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        // clear the interrupt pending bit
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        // update phase
        phase += step;

        // switch to next color
        if (phase >= PI) {
            phase = 0.0f;

            color_index++;
            if (color_index >= 3)
                color_index = 0;
        }

        u8 brightness = f32_to_u8(sinf(phase));
        u32 color = 0xFF000000 | (brightness << (8 * color_index));

        led_show(color);
    }
}