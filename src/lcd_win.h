#ifndef __LCD_WIN_H__
#define __LCD_WIN_H__

#include <glib.h>
#include "can.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *CreateJ1939LcdWin(void);
void DestroyJ1939LcdWin(void);
void UpdateJ1939LcdWin(struct TJ1939Data *d);

#ifdef __cplusplus
  }
#endif

#endif