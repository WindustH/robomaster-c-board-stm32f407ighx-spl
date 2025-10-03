#include "mod/bsp.h"

static void init_bsp_mod() {
  bsp.uart = _uart;
  bsp.led = _led;
  bsp.clock = _clock;
  bsp.cron = _cron;
  bsp.can = _can;
  bsp.motor = _motor;
}

_BspMod bsp = {
    .init = init_bsp_mod,
};