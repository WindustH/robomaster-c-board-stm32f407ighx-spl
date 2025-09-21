#pragma once
#include <stdbool.h>
#include <stdint.h>

#define DMA_BUFFER_SIZE 256
#define PROC_LIST_SIZE 32
#define TIM3_PRESCALER 419
#define TIM3_PERIOD 999

typedef float f32;
typedef double f64;
typedef uint32_t u32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef int32_t i32;
typedef int8_t i8;
typedef int16_t i16;

const f32 PI = 3.14159265359f;
const u32 TICK_PER_SECOND = 42000000U / (TIM3_PRESCALER * TIM3_PERIOD);
const f32 SECOND_PER_TICK = 1.0f / (f32)TICK_PER_SECOND;

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