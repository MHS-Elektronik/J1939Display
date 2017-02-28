#ifndef __GAUGE_WIN_H__
#define __GAUGE_WIN_H__

#include <glib.h>
#include "can.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *CreateJ1939GaugeWin(void);
void DestroyJ1939GaugeWin(void);
void UpdateJ1939GaugeWin(struct TJ1939Data *d);

#ifdef __cplusplus
  }
#endif

#endif