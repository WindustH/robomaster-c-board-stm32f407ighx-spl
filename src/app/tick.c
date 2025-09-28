#include "mod/app.h"
#include "mod/bsp.h"
#include "type.h"

static volatile u32 ticks_cnt = 0;

static void update_time() { ticks_cnt++; }
static u32 get_time() { return ticks_cnt; }

const _TickMod _tick = {.get = get_time, .update = update_time};