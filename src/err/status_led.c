#include "mod/bsp.h"
#include "mod/err.h"

#define FLASH_DURATION_MS 500
#define PAUSE_DURATION_MS 200
#define CYCLE_PAUSE_MS 2000
typedef enum { FLASH, PAUSE, CYCLE_PAUSE } Phase;

static void convert_to_base3(u8 decimal, u8 *base3_digits) {
  for (u8 i = 0; i < 4; i++) {
    base3_digits[3 - i] = decimal % 3;
    decimal /= 3;
  }
}

static u32 color_to_argb(u8 color) {
  switch (color) {
  case 0:
    return 0x00FF0000;
  case 1:
    return 0x0000FF00;
  case 2:
    return 0x000000FF;
  default:
    return 0x00000000;
  }
}

static void update_light_color() {
  static u32 tick_cnt = 0;
  static Phase phase = CYCLE_PAUSE;
  tick_cnt++;
}