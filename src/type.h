#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define DMA_BUFFER_SIZE 256
#define PROC_LIST_SIZE 32
#define SYSCLK 168000000U
#define APB1_CLK 42000000U
#define APB2_CLK 84000000U
#define USART_BAUD_RATE 38400U
#define TIM3_PRESCALER 839
#define TIM3_PERIOD 99
#define MOTOR_CONTROL_ID_1_4 0x200U
#define MOTOR_CONTROL_ID_5_8 0x1FFU
#define MOTOR_FEEDBACK_BASE 0x200U

typedef float f32;
typedef double f64;
typedef uint32_t u32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef int32_t i32;
typedef int8_t i8;
typedef int16_t i16;

extern const f32 PI;
extern const u32 TICK_PER_SECOND;
extern const f32 SECOND_PER_TICK;

typedef void (*proc)(void);

typedef struct {
  u8 dat[DMA_BUFFER_SIZE];
  u16 len;
} buf;

typedef struct {
  proc procs[PROC_LIST_SIZE];
  u32 state;
} procList;

typedef struct {
  u32 x;
  u32 y;
  u32 u;
  u32 v;
} u32v4;

// CAN Header Types
typedef struct {
  uint32_t StdId;              // Standard Identifier
  uint32_t ExtId;              // Extended Identifier
  uint32_t IDE;                // Identifier Type
  uint32_t RTR;                // Remote Transmission Request
  uint32_t DLC;                // Data Length Code
  uint32_t TransmitGlobalTime; // Transmit Global Time
} canTxH;

typedef struct {
  uint32_t StdId;            // Standard Identifier
  uint32_t ExtId;            // Extended Identifier
  uint32_t IDE;              // Identifier Type
  uint32_t RTR;              // Remote Transmission Request
  uint32_t DLC;              // Data Length Code
  uint32_t FilterMatchIndex; // Filter Match Index
  uint32_t Timestamp;        // Timestamp
} canRxH;

// Motor feedback structure
typedef struct {
  i16 i;  // Actual current
  i16 v;  // Actual velocity (rpm)
  i16 th; // Actual position (angle)
  u8 T;   // Temperature
} motorFb;