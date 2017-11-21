#ifndef __START_WIN_H__
#define __START_WIN_H__

#include <gtk/gtk.h>

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *CreateStartWin(struct TCanCore *can_core);
void StartWinRun(GtkWidget *widget);
void StartWinStop(GtkWidget *widget);

#ifdef __cplusplus
  }
#endif

#endif
