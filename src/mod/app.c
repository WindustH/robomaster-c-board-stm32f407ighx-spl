#include "mod/app.h"

static void _init_impl();
_AppMod app = {
    .init = _init_impl,
};

static void init_app_mod() {
  app.pidv = _pidv;
  app.pidx = _pidx;
  app.mon = _mon;
  app.tick = _tick;
}

static void _init_impl() { init_app_mod(); }