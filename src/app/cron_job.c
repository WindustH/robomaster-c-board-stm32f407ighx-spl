#include "app/cron_job.h"
#include "bsp/led.h"
#include "stm32f4xx_conf.h"
#include "utils/map.h"
#include <math.h>

void light_breathe() {
  const f32 PI = 3.14159265359;
  const f32 brightness_step = PI / 200.0f;
  const f32 hue_step = 0.001f;

  static f32 brightness_phase = 0.0f;
  static f32 hue_phase = 0.0f;
  // clear the interrupt pending bit
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

  // update phase
  brightness_phase += brightness_step;
  hue_phase += hue_step;

  // switch to next color
  if (brightness_phase > 3.5 * PI)
    brightness_phase -= 2 * PI;
  if (hue_phase > 1.0)
    hue_phase -= 1.0;

  u8 brightness = f32_to_u8((sinf(brightness_phase) + 1) / 2);
  u32 color = (brightness << 24) | hue_to_rgb(hue_phase);

  led_show(color);
}