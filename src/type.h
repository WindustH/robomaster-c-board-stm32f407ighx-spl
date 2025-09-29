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

// Monitor read structure - all data we want to read from the system
typedef struct {
  // Motor status for all 8 motors
  motStat motors[8];

  // PID status for all 8 motors
  pidStat pids[8];

  // System timestamp
  u32 timestamp;
} monRead;

// Monitor write structure - all data we want to write to the system
typedef struct {
  // PID target values for all 8 motors
  f32 pid_targets[8];

  // PID enable/disable flags for all 8 motors
  u8 pid_enabled[8];

  // PID parameters for all 8 motors
  f32 pid_kp[8];
  f32 pid_ki[8];
  f32 pid_kd[8];
  f32 pid_output_limit[8];
  pidMode pid_mode[8];

  // Control flags
  u8 use_pid_control; // 1 = use PID control, 0 = use direct current control
} monWrite;