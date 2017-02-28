/*******************************************************************************
                            main.c  -  description
                             -------------------
    begin             : 28.02.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the MIT License <LICENSE.TXT or                 *
 *   http://opensource.org/licenses/MIT>                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include "can_drv.h"
#include "util.h"
#include "can.h"
#include "lcd_win.h"
#include "gauge_win.h"
#include "lcd_gauge_win.h"
#include "trace.h"
#include "main.h"



struct TMainWin
  {
  GtkWidget *Main;
  GtkWidget *EventBox;
  GtkWidget *Notebook;
  };

#define MENU_PAGE_INDEX 4

static struct TMainWin MainWin;
static struct TJ1939Data J1939Data;
guint EventTimerId;
gchar *BaseDir;


static void MenuCB(GtkButton *button, gpointer user_data);


static void PathsInit(gchar *prog)
{
BaseDir = g_path_get_dirname(prog);
g_chdir(BaseDir);
}


void SetBackgroundImage(GtkWidget *window, const gchar *filename)
{
GdkPixmap *background;
GdkPixbuf *pixbuf;
GtkStyle *style;
GError *error = NULL;

pixbuf = gdk_pixbuf_new_from_file (filename, &error);
if (error != NULL)
  {
  if (error->domain == GDK_PIXBUF_ERROR)
    g_print ("Pixbuf Related Error:\n");
  if (error->domain == G_FILE_ERROR)
    g_print ("File Error: Check file permissions and state:\n");
  g_printerr ("%s\n", error[0].message);
  }
gdk_pixbuf_render_pixmap_and_mask (pixbuf, &background, NULL, 0);

style = gtk_style_new();
style->bg_pixmap[0] = background;
gtk_widget_set_style(GTK_WIDGET(window), GTK_STYLE(style));
//gtk_window_set_transient_for(GTK_WINDOW (window),GTK_WINDOW(mainwindow));
}


static void FullScreenCB(GtkToggleButton *togglebutton, gpointer user_data)
{
if (gtk_toggle_button_get_active(togglebutton))
  gtk_window_fullscreen(GTK_WINDOW(MainWin.Main));
else
  gtk_window_unfullscreen(GTK_WINDOW(MainWin.Main));
}


/******************************************************************************/
/*                                 Menü Seite                                 */
/******************************************************************************/
static GtkWidget *MainMenu(void)
{
GtkWidget *widget, *box, *btn_bar, *image;

box = gtk_vbox_new(FALSE, 10);
gtk_container_set_border_width(GTK_CONTAINER(box), 5);

widget = create_menue_button(NULL, "<span weight=\"bold\" size=\"x-large\">Übersicht</span>", "J1939 Standert Werte anzeigen");
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)0);
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

widget = create_menue_button(NULL, "<span weight=\"bold\" size=\"x-large\">Übersicht</span>", "J1939 Digital Standert Werte anzeigen");
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)1);
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

widget = create_menue_button(NULL, "<span weight=\"bold\" size=\"x-large\">Übersicht 2</span>", "J1939 Analog Standert Werte anzeigen");
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)2);
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

widget = create_menue_button(NULL, "<span weight=\"bold\" size=\"x-large\">CAN-Trace</span>", "Anzeige der CAN Rohdaten");
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)3);
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

btn_bar = gtk_hbox_new(FALSE, 10);
gtk_box_pack_end(GTK_BOX(box), btn_bar, FALSE, FALSE, 0);

widget = gtk_toggle_button_new_with_label("Vollbild");
g_signal_connect((gpointer)widget, "toggled", G_CALLBACK(FullScreenCB), NULL);
gtk_box_pack_start(GTK_BOX(btn_bar), widget, FALSE, FALSE, 0);

widget = gtk_button_new();
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)1000);
image = gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_BUTTON);
gtk_container_add(GTK_CONTAINER(widget), image);
gtk_box_pack_end(GTK_BOX(btn_bar), widget, FALSE, FALSE, 0);

return(box);
}


static gint EventTimerProc(gpointer data)
{
gint page;
(void)data;

page = gtk_notebook_get_current_page(GTK_NOTEBOOK(MainWin.Notebook));
J1939CanReadMessages(&J1939Data);
switch (page)
  {
  case 0 : {
           UpdateJ1939LcdGaugeWin(&J1939Data);
           break;
           }
  case 1 : {
           UpdateJ1939LcdWin(&J1939Data);
           break;
           }
  case 2 : {
           UpdateJ1939GaugeWin(&J1939Data);
           break;
           }
  }
return(TRUE);
}


static void MenuCB(GtkButton *button, gpointer user_data)
{
guint index;

index = (guint)user_data;
if (index == 1000)
  gtk_main_quit();
gtk_notebook_set_current_page(GTK_NOTEBOOK(MainWin.Notebook), index);
}


/*static void ShowMenuClickedCB(GtkButton *button, gpointer user_data) <*>
{
gtk_notebook_set_current_page(GTK_NOTEBOOK(MainWin.Notebook), MENU_PAGE_INDEX);
}*/


static gboolean on_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
gtk_notebook_set_current_page(GTK_NOTEBOOK(MainWin.Notebook), MENU_PAGE_INDEX);
return(FALSE);
}


static gboolean button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
gtk_notebook_set_current_page(GTK_NOTEBOOK(MainWin.Notebook), MENU_PAGE_INDEX);
return(FALSE);
}


int main(int argc, char *argv[])
{
GtkWidget *win, *vbox, *widget, *notebook; // *hbox, *tmp_image; <*>
gchar *filename;

/* Initialize GTK+ */
gtk_init (&argc, &argv);
PathsInit(argv[0]);
/* Create the main window */
win = gtk_window_new (GTK_WINDOW_TOPLEVEL);

MainWin.Main = win;
//gtk_container_set_border_width (GTK_CONTAINER (win), 8);
gtk_window_set_title(GTK_WINDOW (win), "Open J1939");
gtk_window_set_position(GTK_WINDOW (win), GTK_WIN_POS_CENTER);
//gtk_window_set_default_size(GTK_WINDOW(win), 800, 480);
gtk_window_set_default_size(GTK_WINDOW(win), 1606, 966);

win = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(MainWin.Main), win);
MainWin.EventBox = win;

g_signal_connect(MainWin.Main, "destroy", gtk_main_quit, NULL);
gtk_widget_set_events(win, GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK);
g_signal_connect(win, "key_press_event", G_CALLBACK(on_key_press), NULL);
g_signal_connect(win, "button_press_event", G_CALLBACK(button_press_event), NULL);
gtk_widget_realize(win);

if (J1939CanInit() >= 0)
   EventTimerId = g_timeout_add(300, (GtkFunction)EventTimerProc, NULL);

vbox = gtk_vbox_new(FALSE, 6);
gtk_container_add(GTK_CONTAINER (win), vbox);

notebook = gtk_notebook_new();

MainWin.Notebook = notebook;
gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
// 1. Seite -> J1939 View 1
widget = CreateJ1939LcdGaugeWin();
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 1. Seite -> J1939 View 2
widget = CreateJ1939LcdWin();
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 2. Seite -> J1939 View 3
widget = CreateJ1939GaugeWin();
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 3. Seite -> Trace
widget = CreateTraceWin();
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 4. Seite -> Menü
widget = MainMenu();
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);

gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

/*hbox = gtk_hbox_new(FALSE, 5); <*>

widget = gtk_label_new("J1939Main");
gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);

widget = gtk_button_new();
gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
tmp_image = gtk_image_new_from_stock(GTK_STOCK_EXECUTE, GTK_ICON_SIZE_LARGE_TOOLBAR);
gtk_container_add(GTK_CONTAINER(widget), tmp_image);
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(ShowMenuClickedCB), NULL);
gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 0);

gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0); */

/* Enter the main loop */
//gtk_widget_show_all(win);  // <*>
gtk_widget_show_all(MainWin.Main);
gtk_main();

if (EventTimerId)
  g_source_remove(EventTimerId);
DestroyJ1939LcdWin();
DestroyJ1939GaugeWin();
DestroyTraceWin(); // Muss vor J1939CanDown stehen
J1939CanDown();
safe_free(BaseDir);
return 0;
}
