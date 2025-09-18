#include "setup.h"
#include "led.h"
// for gpio
#include "stm32f4xx_gpio.h"
// for clock control
#include "stm32f4xx_rcc.h"
// for timer
#include "stm32f4xx_tim.h"

void setup_led(void) {
    // enable clock for timer 5 (pwm outpput)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    // enable clock for gpioh
    // (pins to control led)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);

    // gpio config for led
    GPIO_InitTypeDef GPIO_InitStructure;
    // pin 10,11,12 of port h (gpioh)
    // (pins to control led)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    // set to alternate function mode because these pins output pwm waves from
    // timer 5
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    // set to high speed
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    // set to push-pull output type to drive led better
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    // enable pull-up resistor so the led is off by default
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    // apply conf to gpioh
    GPIO_Init(GPIOH, &GPIO_InitStructure);
    // connect timer 5 to gpioh
    GPIO_PinAFConfig(GPIOH, GPIO_PinSource10, GPIO_AF_TIM5);
    GPIO_PinAFConfig(GPIOH, GPIO_PinSource11, GPIO_AF_TIM5);
    GPIO_PinAFConfig(GPIOH, GPIO_PinSource12, GPIO_AF_TIM5);

    // timer output compare structure (for pwm)
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    // config timer 5 for pwm output
    // auto-reload value
    // define the period of pwm
    TIM_TimeBaseStructure.TIM_Period = 255;
    // prescale the timer to 1Mhz
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;
    // no clock division
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    // count from 0 to period-1 and repeat
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    // apply conf to timer 5
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
    // config output compare for pwm
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    // duty cycle (led off)
    TIM_OCInitStructure.TIM_Pulse = 0;
    // high <-> active
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    // apply output config for 3 channels of timer 5
    TIM_OC1Init(TIM5, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Enable);
    TIM_OC2Init(TIM5, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM5, TIM_OCPreload_Enable);
    TIM_OC3Init(TIM5, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM5, TIM_OCPreload_Enable);
    // enable auto-reload preload for timer 5
    TIM_ARRPreloadConfig(TIM5, ENABLE);

    // enable pwm outputs of timer 5 (a basic timer), for led control
    TIM_CtrlPWMOutputs(TIM5, ENABLE);
    TIM_Cmd(TIM5, ENABLE);
    // set initial led state to off
    led_show(0x00000000);
}

// use timer 3 to do cron job
void setup_schedule(void) {
    // enable apb1 bus clocks for timer 3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // config timer 3
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    // prescale timer 3 to 100kHz
    TIM_TimeBaseStructure.TIM_Prescaler = 1679;
    // period 1000-1
    // 0.01 s/tick
    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    // config nested vector interrupt controller for timer 3 interrupts
    NVIC_InitTypeDef NVIC_InitStructure;
    // config nvic for timer 3
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    // enable interrupt
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // apply nvic config
    NVIC_Init(&NVIC_InitStructure);

    // start timer 3
    TIM_Cmd(TIM3, ENABLE);
}

void setup_uart(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // setup gpio
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);

    USART_InitTypeDef USART_InitStructure;
    // baud rate
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;
    // transmit mode
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    // no parity
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}
