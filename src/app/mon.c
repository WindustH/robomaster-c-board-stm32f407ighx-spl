#include "mod/app.h"
#include "mod/bsp.h"
#include "type.h"

// Global monitor data structures
static volatile monRead mon_read = {0};
static volatile monWrite mon_write = {0};

static void setup_monitor() {
  mon_write.pidv_target = 0.0f;
  mon_write.pidv_kp = 19.0f;
  mon_write.pidv_ki = 2.6f;
  mon_write.pidv_kd = 0.2f;
  mon_write.pidv_ol = 1000.0f;
  mon_write.pidv_enabled = 1;

  mon_write.pidx_target = 0.0f;
  mon_write.pidx_kp = 29.0f;
  mon_write.pidx_ki = 1.6f;
  mon_write.pidx_kd = 7.0f;
  mon_write.pidx_ol = 1000.0f;
  mon_write.pidx_enabled = 1;
}

static void update_monitor() {
  mon_read.motor = *bsp.motor.status();
  mon_read.pidv = *app.pidv.status();
  mon_read.pidx = *app.pidx.status();

  app.pidv.set_kp(mon_write.pidv_kp);
  app.pidv.set_ki(mon_write.pidv_ki);
  app.pidv.set_kd(mon_write.pidv_kd);
  app.pidv.set_ol(mon_write.pidv_ol);
  if (!app.pidx.status()->enabled)
    app.pidv.set_target(mon_write.pidv_target);

  if (app.pidv.status()->enabled && !mon_write.pidv_enabled)
    app.pidv.disable();
  else if (!app.pidv.status()->enabled && mon_write.pidv_enabled)
    app.pidv.enable();

  app.pidx.set_kp(mon_write.pidx_kp);
  app.pidx.set_ki(mon_write.pidx_ki);
  app.pidx.set_kd(mon_write.pidx_kd);
  app.pidx.set_ol(mon_write.pidx_ol);
  app.pidx.set_target(mon_write.pidx_target);

  if (app.pidx.status()->enabled && !mon_write.pidx_enabled)
    app.pidx.disable();
  else if (!app.pidx.status()->enabled && mon_write.pidx_enabled)
    app.pidx.enable();
}

const _MonMod _mon = {.update = update_monitor, .setup = setup_monitor};