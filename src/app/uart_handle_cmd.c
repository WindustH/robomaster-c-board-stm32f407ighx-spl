#include "app.h"
#include "bsp.h"
#include "type.h"
#include "uart_send_back.h"
#include "utils/modu.h"

static volatile buf *uart_rx_buf;

static void bind_buf_impl(volatile buf *const _rx_buf) {
  // Check for null pointer
  if (_rx_buf == NULL) {
    return;
  }

  uart_rx_buf = _rx_buf;
}

static void handle_cmd_daemon() {
  static u16 last_send_pos = 0;
  static u16 last_check_pos = 0;

  // Fix potential underflow when uart_rx_buf->len is 0
  u16 current_pos = (uart_rx_buf->len > 0) ? uart_rx_buf->len - 1 : 0;
  u16 pos = last_check_pos;
  last_check_pos = current_pos;

  if (current_pos != pos) {
    while (pos != current_pos) {
      // Add bounds checking for array access
      if (pos < DMA_BUFFER_SIZE &&
          modu_distance(last_send_pos, pos, DMA_BUFFER_SIZE) == 3) {
        u8 offset = 16;
        u32 color = 0;
        while (last_send_pos != pos) {
          // Add bounds checking for array access
          if (last_send_pos < DMA_BUFFER_SIZE) {
            color |= uart_rx_buf->dat[last_send_pos] << offset;
          }
          offset -= 8;
          last_send_pos++;
          if (last_send_pos >= DMA_BUFFER_SIZE)
            last_send_pos = 0;
        }
        color |= 0xff000000;
        bsp.uart.dma.send_dat((u8 *)&color, 4);
        bsp.led.show(color);
      }

      pos++;
      if (pos >= DMA_BUFFER_SIZE)
        pos = 0;
    }
  }
}

const _HandleCmdMod _handle_cmd = {.bind_buf = bind_buf_impl,
                                   .daemon = handle_cmd_daemon};