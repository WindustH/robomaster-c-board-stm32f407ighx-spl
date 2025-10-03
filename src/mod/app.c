#include "mod/app.h"

static void init_app_mod() {
  app.pidv = _pidv;
  app.pidx = _pidx;
  app.mon = _mon;
  app.tick = _tick;
}

_AppMod app = {
    .init = init_app_mod,
};
