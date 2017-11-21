#ifndef __MAIN_H__
#define __MAIN_H__

#include <glib.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
  extern "C" {
#endif

#define APP_INIT         0
#define APP_START        1
#define APP_START_FINISH 2
#define APP_RUN          3

extern gchar *BaseDir;
extern guint64 TimeNow;

void SetBackgroundImage(GtkWidget *window, const gchar *filename);
void SetAppStatus(gint app_status);

#ifdef __cplusplus
  }
#endif

#endif
