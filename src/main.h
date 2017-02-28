#ifndef __MAIN_H__
#define __MAIN_H__

#include <glib.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
  extern "C" {
#endif

gchar *BaseDir;

void SetBackgroundImage(GtkWidget *window, const gchar *filename);

#ifdef __cplusplus
  }
#endif

#endif
