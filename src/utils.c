#include "utils.h"
#include <math.h>

u8 f32_to_u8(f32 x) {
    if (x < 0.0f)
        x = 0.0f;
    if (x > 1.0f)
        x = 1.0f;

    return (u8)lroundf(x * 255.0f);
}