#ifndef __UCFG_H__
#define __UCFG_H__
#include "common.h"
typedef struct {
  unsigned int ver;
  char ssid[SSID_LEN_MAX];
  char skey[32];
  char stype;
  char sip[4];
  unsigned short sport;
  unsigned short lport;
} cfg_t;

extern cfg_t cfg;
void ucfg_init(void);
void ucfg_save(void);
void ucfg_read(void);

#endif
