#include "bsp.h"
_BspMod bsp;

void init_bsp_mod() {
  bsp.uart = _uart;
  bsp.led = _led;
  bsp.clock = _clock;
  bsp.cron = _cron;
}