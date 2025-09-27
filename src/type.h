#pragma once
#include "def.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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
} motStat;

typedef enum { PID_POSITION, PID_VELOCITY, PID_CURRENT } pidMode;

typedef struct {
  f32 kp;           // Proportional gain
  f32 ki;           // Integral gain
  f32 kd;           // Derivative gain
  f32 dt;           // Time step (seconds)
  f32 output_limit; // Output saturation limit
  pidMode mode;     // Control mode

  f32 integral;   // Integral accumulator
  f32 prev_error; // Previous error for derivative
  f32 target;     // Target value
  u8 enabled;     // Controller enabled flag
} pidStat;
