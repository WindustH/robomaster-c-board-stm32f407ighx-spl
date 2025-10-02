#include "mod/app.h"
#include "mod/bsp.h"

// Global monitor data structures
static monRead mon_read = {0};
static monWrite mon_write = {0};

static void update_monitor() {
  mon_read.motors = *bsp.motor.status();

  volatile pidStat *pidv = app.pidv.status();
  if (pidv != NULL)
    mon_read.pidv = *pidv;

  app.pidv.set_kp(mon_write.pidv_kp);
  app.pidv.set_ki(mon_write.pidv_ki);
  app.pidv.set_kd(mon_write.pidv_kd);
  app.pidv.set_output_limit(mon_write.pidv_ol);
  app.pidv.set_target(mon_write.pidv_target);

  if (app.pidv.status()->enabled && !mon_write.pidv_enabled)
    app.pidv.disable();
  else if (!app.pidv.status()->enabled && mon_write.pidv_enabled)
    app.pidv.enable();

  volatile pidStat *pidx = app.pidx.status();
  if (pidx != NULL)
    mon_read.pidx = *pidx;

  app.pidx.set_kp(mon_write.pidx_kp);
  app.pidx.set_ki(mon_write.pidx_ki);
  app.pidx.set_kd(mon_write.pidx_kd);
  app.pidx.set_output_limit(mon_write.pidx_ol);
  app.pidx.set_target(mon_write.pidx_target);

  if (app.pidx.status()->enabled && !mon_write.pidx_enabled)
    app.pidx.disable();
  else if (!app.pidx.status()->enabled && mon_write.pidx_enabled)
    app.pidx.enable();
}

const _MonMod _mon = {
    .update = update_monitor, .read = &mon_read, .write = &mon_write};