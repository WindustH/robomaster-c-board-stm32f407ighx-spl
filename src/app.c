#include "app.h"

static void _setup_impl();
_AppMod app = {
    .setup = _setup_impl,
};
static void setup_app_mod() {
  app.sendback = _sendback;
  app.handle_cmd = _handle_cmd;
}

static void _setup_impl() { setup_app_mod(); }