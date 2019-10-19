#ifndef __AT_UART_H__
#define __AT_UART_H__

void at_uart_init(void);
void at_uart_write(char ch);
int at_uart_read(void);

#endif
