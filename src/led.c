#include "led.h"
// for timers
#include "stm32f4xx_tim.h"

void led_show(u32 argb) {
    u32 alpha;
    u32 red, green, blue;

    // extract channels
    alpha = (argb & 0xFF000000) >> 24;
    red = (argb & 0x00FF0000) >> 16;
    green = (argb & 0x0000FF00) >> 8;
    blue = (argb & 0x000000FF) >> 0;

    red = red * alpha / 255;
    green = green * alpha / 255;
    blue = blue * alpha / 255;

    // set timer 5 to output pwm wave
    TIM_SetCompare1(TIM5, blue);
    TIM_SetCompare2(TIM5, green);
    TIM_SetCompare3(TIM5, red);
}