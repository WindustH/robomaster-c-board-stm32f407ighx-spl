#pragma once
#include "type.h"

typedef struct {
  void (*bind_buf)(volatile buf *const __rx_buf);
  void (*daemon)();
} _SendbackMod;
extern const _SendbackMod _sendback;

typedef struct {
  void (*bind_buf)(volatile buf *const __rx_buf);
  void (*daemon)();
} _HandleCmdMod;
extern const _HandleCmdMod _handle_cmd;

typedef struct {
  void (*init)();
  _SendbackMod sendback;
  _HandleCmdMod handle_cmd;
} _AppMod;
extern _AppMod app;