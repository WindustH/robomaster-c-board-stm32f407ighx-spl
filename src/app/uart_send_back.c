#include "uart_send_back.h"
#include "app.h"
#include "bsp.h"
#include "type.h"
#include "utils/modu.h"

static volatile buf *uart_rx_buf;

static void bind_buf_impl(volatile buf *const _rx_buf) {
  uart_rx_buf = _rx_buf;
}

static void send_back_daemon() {
  static u16 last_send_pos = 0;
  static u16 last_check_pos = 0;
  u16 current_pos = uart_rx_buf->len - 1;
  u16 pos = last_check_pos;
  last_check_pos = current_pos;

  if (current_pos != pos) {
    while (pos != current_pos) {
      if (uart_rx_buf->dat[pos] == ' ') {
        u32v4 span = modu_span(last_send_pos, pos, DMA_BUFFER_SIZE);
        bsp.uart.dma.send_str("# SENDBACK #\n\t");
        bsp.uart.dma.send_dat((const u8 *)&uart_rx_buf->dat[span.x],
                              span.y - span.x);
        bsp.uart.dma.send_dat((const u8 *)&uart_rx_buf->dat[span.u],
                              span.v - span.u);
        last_send_pos = pos + 1;
        if (last_send_pos >= DMA_BUFFER_SIZE) {
          last_send_pos = 0;
        }
      }

      pos++;
      if (pos >= DMA_BUFFER_SIZE) {
        pos = 0;
      }
    }
  }
}

const _SendbackMod _sendback = {.bind_buf = bind_buf_impl,
                                .daemon = send_back_daemon};