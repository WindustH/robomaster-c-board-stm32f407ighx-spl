#pragma once
#include "type.h"

typedef struct {
  void (*bind_buf)(volatile buf *const __rx_buf);
  void (*daemon)();
} _SendbackMod;
extern const _SendbackMod _sendback;

typedef struct {
  void (*setup)();
  _SendbackMod sendback;
} _AppMod;
extern _AppMod app;