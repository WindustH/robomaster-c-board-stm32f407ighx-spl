#include "mod/app.h"
#include "mod/bsp.h"
#include "type.h"

// Global monitor data structures
static volatile monRead mon_read = {0};
static volatile monWrite mon_write = {0};

static void setup_monitor() {
  mon_write.pidv_target = app.pidv.status()->target;
  mon_write.pidv_r = app.pidv.status()->r;
  mon_write.pidv_kp = app.pidv.status()->kp;
  mon_write.pidv_ki = app.pidv.status()->ki;
  mon_write.pidv_kd = app.pidv.status()->kd;
  mon_write.pidv_kpr = app.pidv.status()->kpr;
  mon_write.pidv_kir = app.pidv.status()->kir;
  mon_write.pidv_kdr = app.pidv.status()->kdr;
  mon_write.pidv_ol = app.pidv.status()->output_limit;
  mon_write.pidv_enabled = app.pidv.status()->enabled;

  mon_write.pidx_target = app.pidx.status()->target;
  mon_write.pidx_r = app.pidx.status()->r;
  mon_write.pidx_kp = app.pidx.status()->kp;
  mon_write.pidx_ki = app.pidx.status()->ki;
  mon_write.pidx_kd = app.pidx.status()->kd;
  mon_write.pidx_kpr = app.pidx.status()->kpr;
  mon_write.pidx_kir = app.pidx.status()->kir;
  mon_write.pidx_kdr = app.pidx.status()->kdr;
  mon_write.pidx_ol = app.pidx.status()->output_limit;
  mon_write.pidx_enabled = app.pidx.status()->enabled;
}

static void update_monitor() {
  mon_read.motor = *bsp.motor.status();
  mon_read.pidv = *app.pidv.status();
  mon_read.pidx = *app.pidx.status();

  app.pidv.set_r(mon_write.pidv_r);
  app.pidv.set_kp(mon_write.pidv_kp);
  app.pidv.set_ki(mon_write.pidv_ki);
  app.pidv.set_kd(mon_write.pidv_kd);
  app.pidv.set_kpr(mon_write.pidv_kpr);
  app.pidv.set_kir(mon_write.pidv_kir);
  app.pidv.set_kdr(mon_write.pidv_kdr);
  app.pidv.set_ol(mon_write.pidv_ol);
  if (!app.pidx.status()->enabled)
    app.pidv.set_target(mon_write.pidv_target);

  if (app.pidv.status()->enabled && !mon_write.pidv_enabled)
    app.pidv.disable();
  else if (!app.pidv.status()->enabled && mon_write.pidv_enabled)
    app.pidv.enable();

  app.pidx.set_r(mon_write.pidx_r);
  app.pidx.set_kp(mon_write.pidx_kp);
  app.pidx.set_ki(mon_write.pidx_ki);
  app.pidx.set_kd(mon_write.pidx_kd);
  app.pidx.set_kpr(mon_write.pidx_kpr);
  app.pidx.set_kir(mon_write.pidx_kir);
  app.pidx.set_kdr(mon_write.pidx_kdr);
  app.pidx.set_ol(mon_write.pidx_ol);
  app.pidx.set_target(mon_write.pidx_target);

  if (app.pidx.status()->enabled && !mon_write.pidx_enabled)
    app.pidx.disable();
  else if (!app.pidx.status()->enabled && mon_write.pidx_enabled)
    app.pidx.enable();
}

const _MonMod _mon = {.update = update_monitor, .setup = setup_monitor};