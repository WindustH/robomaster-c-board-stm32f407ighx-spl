#include "app.h"

static void _init_impl();
_AppMod app = {
    .init = _init_impl,
};
static void init_app_mod() {
  app.sendback = _sendback;
  app.handle_cmd = _handle_cmd;
}

static void _init_impl() { init_app_mod(); }