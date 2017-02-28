#ifndef __LCD_GAUGE_WIN_H__
#define __LCD_GAUGE_WIN_H__

#include <glib.h>
#include "can.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *CreateJ1939LcdGaugeWin(void);
void DestroyJ1939LcdGaugeWin(void);
void UpdateJ1939LcdGaugeWin(struct TJ1939Data *d);

#ifdef __cplusplus
  }
#endif

#endif
