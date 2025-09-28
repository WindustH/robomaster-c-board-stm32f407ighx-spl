#pragma once
#include "type.h"

typedef struct {
  void (*setup)(volatile buf *const _app_rx_buf);
  void (*send_dat)(const u8 *data, const u16 length);
  void (*send_str)(const char *str);
  void (*bind_rx_callback)(void (*cb)(volatile u8 const *dat, const u16 len));
  void (*unbind_rx_callback)();
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
extern _UartMod _uart;
void init_uart_mod(void);

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
  void (*setup)();
  u8 (*add_job)(const proc p);
  void (*remove_job)(const u8 idx);
} _CronMod;
extern const _CronMod _cron;

typedef struct {
  void (*setup)(void);
  u8 (*send)(canTxH *pHeader, u8 aData[], u32 *pTxMailbox);
  u8 (*read)(canRxH *pHeader, u8 aData[]);
  void (*bind_rx_callback)(void (*cb)(canRxH *rx_header, u8 *data));
  void (*unbind_rx_callback)();
} _CanMod;

extern const _CanMod _can;

typedef struct {
  void (*setup)(void);
  u8 (*set)(u8 motor_id, i16 current);
  motStat (*status)(u8 motor_id);
  void (*send_ctrl_signal)();
  void (*update_status)(canRxH *rx_header, u8 *data);
} _MotorMod;

extern const _MotorMod _motor;

typedef struct {
  void (*init)();
  _UartMod uart;
  _LedMod led;
  _ClockMod clock;
  _CronMod cron;
  _CanMod can;
  _MotorMod motor;
} _BspMod;

extern _BspMod bsp;
