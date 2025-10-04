#include "def.h"
#include "mod/app.h"
#include "mod/bsp.h"
#include "stm32f4xx.h"
#include "type.h"

int main() {
  bsp.init();
  app.init();

  bsp.clock.setup();
  bsp.led.setup();
  bsp.cron.setup();
  bsp.can.setup();
  bsp.motor.setup();
  app.pidv.setup();
  app.pidx.setup();

  app.pidv.set_kp(3000.0f);
  app.pidv.set_ki(0.0f);
  app.pidv.set_kd(0.0f);
  app.pidv.set_kpr(20.0f);
  app.pidv.set_kir(0.0f);
  app.pidv.set_kdr(0.0f);
  app.pidv.set_ol(MAX_CURRENT);
  app.pidv.set_target(0.0f);
  app.pidv.set_r(400);
  app.pidv.enable();
  bsp.cron.add_job(app.pidv.update);

  app.pidx.set_kp(3000.0f);
  app.pidx.set_ki(0.0f);
  app.pidx.set_kd(0.0f);
  app.pidx.set_kpr(500.0f);
  app.pidx.set_kir(0.0f);
  app.pidx.set_kdr(0.0f);
  app.pidx.set_ol(4800.0f);
  app.pidx.set_target(0.0f);
  app.pidx.set_r(1.5f * PI);
  app.pidx.enable();
  bsp.cron.add_job(app.pidx.update);

  app.mon.setup();
  bsp.cron.add_job(app.mon.update);

  // bsp.cron.add_job(app.tick.update);
  bsp.cron.add_job(bsp.motor.send_ctrl_signal);
  bsp.can.bind_rx_callback(bsp.motor.update_status);

  while (true) {
  }
}