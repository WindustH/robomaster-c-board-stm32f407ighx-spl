#include "bsp.h"

static void _setup_impl();

_BspMod bsp = {
    .setup = _setup_impl,
};

static void setup_bsp_mod() {
  bsp.uart = _uart;
  bsp.led = _led;
  bsp.clock = _clock;
  bsp.cron = _cron;
}

static void _setup_impl() {
  setup_uart_mod();
  setup_bsp_mod();
}