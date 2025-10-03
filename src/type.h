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
  u32 StdId;              // Standard Identifier
  u32 ExtId;              // Extended Identifier
  u32 IDE;                // Identifier Type
  u32 RTR;                // Remote Transmission Request
  u32 DLC;                // Data Length Code
  u32 TransmitGlobalTime; // Transmit Global Time
} canTxH;

typedef struct {
  u32 StdId;            // Standard Identifier
  u32 ExtId;            // Extended Identifier
  u32 IDE;              // Identifier Type
  u32 RTR;              // Remote Transmission Request
  u32 DLC;              // Data Length Code
  u32 FilterMatchIndex; // Filter Match Index
  u32 Timestamp;        // Timestamp
} canRxH;

// Motor feedback structure
typedef struct {
  i16 i;  // Actual current
  i16 v;  // Actual velocity (rpm)
  f32 th; // Actual position (angle)
  u8 T;   // Temperature
} motStat;

typedef struct {
  f32 kp;           // Proportional gain
  f32 ki;           // Integral gain
  f32 kd;           // Derivative gain
  f32 dt;           // Time step (seconds)
  f32 output_limit; // Output saturation limit

  f32 target; // Target value
  f32 p;
  f32 i;
  f32 d;

  f32 r;
  f32 kpr;
  f32 kir;
  f32 kdr;

  f32 prev_error; // Previous error for derivative

  u8 enabled; // Controller enabled flag
} pidStat;

typedef struct {
  motStat motor;
  pidStat pidv;
  pidStat pidx;
} monRead;

typedef struct {
  f32 pidv_target;
  f32 pidv_kp;
  f32 pidv_ki;
  f32 pidv_kd;
  f32 pidv_kpr;
  f32 pidv_kir;
  f32 pidv_kdr;
  f32 pidv_r;
  f32 pidv_ol;
  u8 pidv_enabled;

  f32 pidx_target;
  f32 pidx_kp;
  f32 pidx_ki;
  f32 pidx_kd;
  f32 pidx_kpr;
  f32 pidx_kir;
  f32 pidx_kdr;
  f32 pidx_r;
  f32 pidx_ol;
  u8 pidx_enabled;
} monWrite;