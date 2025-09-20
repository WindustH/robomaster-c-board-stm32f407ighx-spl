
#pragma once
#include "type.h"

typedef struct {
  void (*setup)();
  void (*send_dat)(const u8 *data, const u16 length);
  void (*send_str)(const char *str);
  void (*proc_rx_dat)();
} _UartDmaMod;
extern const _UartDmaMod _uart_dma;

typedef struct {
  void (*setup)();
  void (*send_byte)(const u8 data);
  void (*send_str)(const char *str);
  u8 (*has_new_byte)();
  u8 (*read_byte)();
} _UartItMod;
extern const _UartItMod _uart_it;

typedef struct {
  void (*setup)();
  _UartDmaMod dma;
  _UartItMod it;
} _UartMod;
extern const _UartMod _uart;

typedef struct {
  void (*setup)();
  void (*show)(const u32 argb);
} _LedMod;
extern const _LedMod _led;

typedef struct {
  void (*setup)();
} _ClockMod;
extern const _ClockMod _clock;

typedef struct {
  _UartMod uart;
  _LedMod led;
  _ClockMod clock;
} _BspMod;

extern const _BspMod bsp;
