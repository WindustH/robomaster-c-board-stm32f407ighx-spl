#include "utils/map.h"
#include <math.h>

u8 f32_to_u8(f32 x) {
  if (x < 0.0f)
    x = 0.0f;
  if (x > 1.0f)
    x = 1.0f;

  return (u8)lroundf(x * 255.0f);
}

u32 hue_to_rgb(f32 hue) {
  if (hue < 0.0)
    hue = 0.0;
  if (hue >= 1.0)
    hue = 0.0;

  f32 r = 0.0, g = 0.0, b = 0.0;
  f32 scaled_hue = hue * 6.0;
  i32 sector = (i32)scaled_hue;
  f32 fraction = scaled_hue - sector;

  switch (sector) {
  case 0:
    r = 1.0;
    g = fraction;
    b = 0.0;
    break;
  case 1:
    r = 1.0 - fraction;
    g = 1.0;
    b = 0.0;
    break;
  case 2:
    r = 0.0;
    g = 1.0;
    b = fraction;
    break;
  case 3:
    r = 0.0;
    g = 1.0 - fraction;
    b = 1.0;
    break;
  case 4:
    r = fraction;
    g = 0.0;
    b = 1.0;
    break;
  case 5:
  default:
    r = 1.0;
    g = 0.0;
    b = 1.0 - fraction;
    break;
  }

  u8 R_8bit = (u8)(r * 255.0 + 0.5);
  u8 G_8bit = (u8)(g * 255.0 + 0.5);
  u8 B_8bit = (u8)(b * 255.0 + 0.5);
  u32 color = (u32)(R_8bit << 16) | (u32)(G_8bit << 8) | (u32)B_8bit;

  return color;
}
