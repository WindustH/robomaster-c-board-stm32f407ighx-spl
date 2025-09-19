#include "stm32f4xx.h"
#include "type.h"

void uart_send_byte(const u8 byte);
void uart_send_str(const char *str);
u8 uart_received_byte(void);
u8 uart_read_byte(void);