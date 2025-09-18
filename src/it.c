#include "it.h"
#include "led.h"
#include <math.h>
#include "stm32f4xx.h"
#include "uart.h"
#include "utils.h"

volatile f32 brightness_phase = 0.0f;
volatile f32 hue_phase = 0.0f;
const f32 PI = 3.14159265359;
const f32 brightness_step = PI / 50.0f;
const f32 hue_step = 0.005f;

volatile u32 cnt_0 = 0;
const u32 ddl_0 = 100;

void TIM3_IRQHandler(void) {
    // check if update interrupt flag is set
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        // clear the interrupt pending bit
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        // update phase
        brightness_phase += brightness_step;
        hue_phase += hue_step;
        cnt_0++;

        if (cnt_0 >= ddl_0) {
            char *msg = "test\n";
            uart_send_str(msg);
            cnt_0 = 0;
        }
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