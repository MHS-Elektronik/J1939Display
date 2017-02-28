#ifndef __TRACE_H__
#define __TRACE_H__

#include <glib.h>

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *CreateTraceWin(void);
void DestroyTraceWin(void);

#ifdef __cplusplus
  }
#endif

#endif
