#ifndef __AT_H__
#define __AT_H__

#define at_printf        Report
#define ATOK "OK\r\n"
#define ATERROR "ERR: Bad command\r\n"


int at_check(void);
void at_proc(char *p_msg);
void at_notify_data(char *buf, int len);
void ATCommandInit(void);

void AT_ReportedLinkStatus(void);

#endif