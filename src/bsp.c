#include "bsp.h"
#include "can.h"

static void _setup_impl();

_BspMod bsp = {
    .setup = _setup_impl,
};

static void setup_bsp_mod() {
  bsp.uart = _uart;
  bsp.led = _led;
  bsp.clock = _clock;
  bsp.cron = _cron;
  bsp.can = _can;
}

static void _setup_impl() {
  setup_uart_mod();
  setup_bsp_mod();
  _can.setup();
}