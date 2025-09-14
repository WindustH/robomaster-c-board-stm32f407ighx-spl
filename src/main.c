#include "led.h"
#include "setup.h"
#include "stm32f4xx.h"

int main(void) {
    setup_system_clocks();
    setup_timers();
    setup_gpio();

    // enable pwm outputs of timer 5 (a basic timer), for led control
    TIM_CtrlPWMOutputs(TIM5, ENABLE);
    TIM_Cmd(TIM5, ENABLE);
    // set initial led state to off
    led_show(0x00000000);

    //  main loop
    while (true) {
    }
}
