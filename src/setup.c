#include "setup.h"
#include "led.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"
#include "string.h"

void setup_clock(void) {
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
void setup_led(void) {
  // enable clock(42000000Hz) for timer 5 (pwm outpput)
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
  TIM_TimeBaseStructure.TIM_Prescaler = 42 - 1;
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
  // enable apb1 peripherial clocks (42000000Hz) for timer 3
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  // config timer 3
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  // prescale timer 3 to 100kHz
  TIM_TimeBaseStructure.TIM_Prescaler = 420 - 1;
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
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  // setup gpio
  // for transmit
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
  // for receive
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

  USART_InitTypeDef USART_InitStructure;
  // baud rate
  USART_InitStructure.USART_BaudRate = 38400;
  USART_InitStructure.USART_HardwareFlowControl =
      USART_HardwareFlowControl_None;
  // transmit and receive mode
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  // no parity
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;

  USART_Init(USART1, &USART_InitStructure);
  USART_Cmd(USART1, ENABLE);
}
void setup_it_uart() {
  // config nvic
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_Init(&NVIC_InitStructure);

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

volatile u8 dma_tx_buffer[DMA_BUFFER_SIZE];
volatile u8 dma_rx_buffer[DMA_BUFFER_SIZE];

void setup_dma_uart() {
  DMA_InitTypeDef DMA_InitStructure;

  // 使能DMA2时钟
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

  // 配置DMA发送(USART1_TX - DMA2 Stream7 Channel4)
  DMA_DeInit(DMA2_Stream7);
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)dma_tx_buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = DMA_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream7, &DMA_InitStructure);

  // 配置DMA接收(USART1_RX - DMA2 Stream2 Channel4)
  DMA_DeInit(DMA2_Stream2);
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)dma_rx_buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = DMA_BUFFER_SIZE;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; // 循环模式用于持续接收
  DMA_Init(DMA2_Stream2, &DMA_InitStructure);

  // 使能DMA流
  DMA_Cmd(DMA2_Stream7, ENABLE);
  DMA_Cmd(DMA2_Stream2, ENABLE);

  // 启用UART发送DMA
  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

  // 启用UART接收DMA
  USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

  memset((void *)dma_tx_buffer, 0, DMA_BUFFER_SIZE);
  memset((void *)dma_rx_buffer, 0, DMA_BUFFER_SIZE);

  NVIC_InitTypeDef NVIC_InitStructure;

  // 配置DMA发送完成中断
  DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);

  // 配置NVIC
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}