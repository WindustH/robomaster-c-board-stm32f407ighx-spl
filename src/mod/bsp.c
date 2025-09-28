#include "mod/bsp.h"

static void _init_impl();

_BspMod bsp = {
    .init = _init_impl,
};

static void init_bsp_mod() {
  bsp.uart = _uart;
  bsp.led = _led;
  bsp.clock = _clock;
  bsp.cron = _cron;
  bsp.can = _can;
  bsp.motor = _motor;
}

static void _init_impl() {
  init_uart_mod();
  init_bsp_mod();
}