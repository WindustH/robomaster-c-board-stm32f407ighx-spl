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
  app.pidv.set_kp(30.0f);
  app.pidv.set_ki(60.0f);
  app.pidv.set_kd(0.0f);
  app.pidv.set_kpr(5.0f);
  app.pidv.set_kir(20.0f);
  app.pidv.set_kdr(0.0f);
  app.pidv.set_ol(MAX_CURRENT);
  app.pidv.set_target(0.0f);
  app.pidv.set_r(30);
  app.pidv.enable();
  bsp.cron.add_job(app.pidv.update);

  app.pidx.setup();
  app.pidx.set_kp(400.0f);
  app.pidx.set_ki(400.0f);
  app.pidx.set_kd(-3.0f);
  app.pidx.set_kpr(100.0f);
  app.pidx.set_kir(500.0f);
  app.pidx.set_kdr(0.0f);
  app.pidx.set_ol(6000.0f);
  app.pidx.set_target(0.0f);
  app.pidx.set_r(5);
  app.pidx.enable();
  bsp.cron.add_job(app.pidx.update);

  app.mon.setup();
  bsp.cron.add_job(app.mon.update);

  bsp.cron.add_job(app.tick.update);
  bsp.cron.add_job(bsp.motor.send_ctrl_signal);
  bsp.can.bind_rx_callback(bsp.motor.update_status);

  while (true) {
  }
}