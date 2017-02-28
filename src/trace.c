/*******************************************************************************
                           trace.c  -  description
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
#include <glib.h>
#include <gtk/gtk.h>
#include "util.h"
#include "can_drv.h"
#include "cbuf.h"
#include "etable.h"
#include "trace.h"


#define CAN_BUFFER_SIZE 1000000
#define RX_TEMP_BUFFER_SIZE 512

#define TRACE_SHOW_ALL           0
#define TRACE_SHOW_ONLY_KNOWN    1
#define TRACE_SHOW_ONLY_UNKNOWN  2

#define RX_EVENT     0x00000001

struct TTraceWin
  {
  GtkWidget *VBox;
  GtkWidget *Toolbar;
  GtkWidget *NeuButton;
  GtkWidget *CanStartStopButton;
  GtkWidget *FilterSelectComboBox;
  GtkWidget *ETable;
  TCanBuffer *Buffer;
  struct TCanMsg RxTempBuffer[RX_TEMP_BUFFER_SIZE];
  GThread *Thread;
  TMhsEvent *Event;
  guint CanStartStopClickId;
  guint Run;
  guint TraceFilter;
  };


static struct TTraceWin TraceWin;


static struct TETableDesc TableDescription[] = {
  // Colum - Name   | Colum - Template              |  Show, Color ()
  {"Id",             "XXXXXXXX ",                         1, {0, 0, 0, 0}},
  {"DLC",            "XX ",                               1, {0, 0, 0, 0}},
  {"Data (Hex)",     "XX XX XX XX XX XX XX XX ",          1, {0, 0, 0, 0}},
  {"Data (ASCII)",   "XXXXXXXX ",                         1, {0, 0, 0, 0}},
  {NULL, NULL, 0, {0, 0, 0, 0}}};


static gint GetLine(guint index, gpointer user_data, gchar *line, GdkColor *color, guint *flags)
{
struct TTraceWin *trace_win;
guint32 msg_len, i;
unsigned char ch, hex;
struct TCanMsg msg;

trace_win = (struct TTraceWin *)user_data;
if (CBufGetMsgByIndex(trace_win->Buffer, &msg, index))
  return(-1);
msg_len = msg.MsgLen;
// Id |
if (msg.MsgEFF)
  line += g_snprintf(line, 13, "%.8X\t", msg.Id);
else
  line += g_snprintf(line, 13, "     %.3X\t", msg.Id);
// Dlc |
line += g_snprintf(line, 4, "%u\t", msg_len);
// Bei RTR msg_len auf 0 setztzen, keine Daten
if (msg.MsgRTR)
  msg_len = 0;
// Hex |
for (i = 0; i < msg_len; i++)
  {
  ch = msg.MsgData[i];
  hex = ch >> 4;
  if (hex > 9)
    *line++ = 55 + hex;
  else
    *line++ = '0' + hex;
  hex = ch & 0x0F;
  if (hex > 9)
    *line++ = 55 + hex;
  else
    *line++ = '0' + hex;
  *line++ = ' ';
  }
*line++ = '\t';
// ASCII |
for (i = 0; i < msg_len; i++)
  {
  ch = msg.MsgData[i];
  if ((ch <= 32) || (ch >= 126))
    *line++ = '.';
  else
    *line++ = (char)ch;
  }
*line++ = '\t';
*line = '\0';  // Fehler vermeiden wenn keine Daten
return(0);
}


/****************/
/* Event Thread */
/****************/
static gpointer ThreadExecute(gpointer data)
{
struct TTraceWin *trace_win;
struct TCanMsg *msg;
gint size, save;
uint32_t event;

trace_win = (struct TTraceWin *)data;
do
  {
  event = CanExWaitForEvent(trace_win->Event, 0);
  if (event & 0x80000000)        // Beenden Event, Thread Schleife verlassen
    break;
  else if (event & RX_EVENT)     // CAN Rx Event
    {
    if (trace_win->Run)
      {
      while ((size = (gint)CanReceive(0x80000000, trace_win->RxTempBuffer, RX_TEMP_BUFFER_SIZE)) > 0)
        {
        for (msg = trace_win->RxTempBuffer; size; size--)
          {
          save = 0;
          if (trace_win->TraceFilter == TRACE_SHOW_ONLY_KNOWN)
            {
            if (msg->MsgFilHit)
              save = 1;
            }
          else if (trace_win->TraceFilter == TRACE_SHOW_ONLY_UNKNOWN)
            {
            if (!msg->MsgFilHit)
              save = 1;
            }
          else
            save = 1;
          if (save)
            CBufAddMsgs(trace_win->Buffer, msg, 1);
          msg++;
          }
        }
      }
    else
      CanReceiveClear(0x80000000);
    }
  }
while (1);
return(NULL);
}


static gint EventProc(TCanBuffer *cbuf, guint events, gpointer user_data)
{
struct TTraceWin *trace_win;

trace_win = (struct TTraceWin *)user_data;
etable_set_row_size(ETABLE(trace_win->ETable), CBufGetSize(cbuf));
return(TRUE);  // <*>
}


static void SetStartStop(struct TTraceWin *trace_win)
{
GtkWidget *image;
gint iconsize;

g_signal_handler_block(G_OBJECT(TraceWin.CanStartStopButton), TraceWin.CanStartStopClickId);
// **** Menue Items updaten
iconsize = gtk_toolbar_get_icon_size(GTK_TOOLBAR(TraceWin.Toolbar));
if (trace_win->Run)
  {
  // Button
  gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(TraceWin.CanStartStopButton), TRUE);
  image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP, iconsize);
  gtk_widget_show(image);
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), image);
  gtk_tool_button_set_label(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), "Stop");
  }
else
  {
  // Button
  gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(TraceWin.CanStartStopButton), FALSE);
  image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, iconsize);
  gtk_widget_show(image);
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), image);
  gtk_tool_button_set_label(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), "Start");
  }
g_signal_handler_unblock(G_OBJECT(TraceWin.CanStartStopButton), TraceWin.CanStartStopClickId);
}


static void NeuButtonClickCB(GtkButton *button, gpointer user_data)
{
struct TTraceWin *trace_win;

trace_win = (struct TTraceWin *)user_data;
etable_set_row_size(ETABLE(trace_win->ETable), 0);
CBufDataClear(trace_win->Buffer);
}


static void CanStartStopButtonClickCB(GtkButton *button, gpointer user_data)
{
struct TTraceWin *trace_win;

trace_win = (struct TTraceWin *)user_data;
if (trace_win->Run)
  trace_win->Run = 0;
else
  trace_win->Run = 1;
SetStartStop(trace_win);
}


static void FilterSelectComboBoxCB(GtkWidget *combo, gpointer user_data)
{
struct TTraceWin *trace_win;

trace_win = (struct TTraceWin *)user_data;
trace_win->TraceFilter = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
}


GtkWidget *CreateTraceWin(void)
{
GtkWidget *widget, *image, *item;
gint icon_size;

//CBufCreate(guint reserved_size, gboolean override, guint event_delay,
//     guint event_timeout, guint event_limit, CBufGEventCb callback, gpointer user_data)
if (!(TraceWin.Buffer = CBufCreate(CAN_BUFFER_SIZE, FALSE, 100, 100, 0, EventProc, (gpointer)&TraceWin)))
  return(NULL);
// **** VBox
TraceWin.VBox = gtk_vbox_new (FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(TraceWin.VBox), 0);
// **** Toolbar erzeugen
TraceWin.Toolbar = gtk_toolbar_new ();
gtk_box_pack_start(GTK_BOX(TraceWin.VBox), TraceWin.Toolbar, FALSE, FALSE, 0);
gtk_toolbar_set_style(GTK_TOOLBAR(TraceWin.Toolbar), GTK_TOOLBAR_BOTH);
icon_size = gtk_toolbar_get_icon_size(GTK_TOOLBAR(TraceWin.Toolbar));
// **** Button "Neu"
image = gtk_image_new_from_stock ("gtk-clear", icon_size);
TraceWin.NeuButton = GTK_WIDGET(gtk_tool_button_new(image, "New"));
gtk_container_add(GTK_CONTAINER(TraceWin.Toolbar), TraceWin.NeuButton);
(void)g_signal_connect(TraceWin.NeuButton, "clicked", G_CALLBACK(NeuButtonClickCB), &TraceWin);
// **** Button "Start/Stop"
TraceWin.CanStartStopButton = GTK_WIDGET(gtk_toggle_tool_button_new());
gtk_tool_button_set_label(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), "Start");
image = gtk_image_new_from_stock ("gtk-media-play", icon_size);
gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), image);
gtk_widget_show (TraceWin.CanStartStopButton);
gtk_container_add(GTK_CONTAINER(TraceWin.Toolbar), TraceWin.CanStartStopButton);
TraceWin.CanStartStopClickId = g_signal_connect(TraceWin.CanStartStopButton, "clicked", G_CALLBACK(CanStartStopButtonClickCB), &TraceWin);
// **** Seperator
widget = (GtkWidget*) gtk_separator_tool_item_new ();
gtk_container_add (GTK_CONTAINER (TraceWin.Toolbar), widget);
// **** Test
widget = gtk_label_new("Anzeigen: ");
item = GTK_WIDGET(gtk_tool_item_new());
gtk_container_add(GTK_CONTAINER(item), GTK_WIDGET(widget));
gtk_container_add(GTK_CONTAINER(TraceWin.Toolbar), GTK_WIDGET(item));
// **** ComboBox
TraceWin.FilterSelectComboBox = gtk_combo_box_new_text();
gtk_combo_box_append_text(GTK_COMBO_BOX(TraceWin.FilterSelectComboBox), "Alle");
gtk_combo_box_append_text(GTK_COMBO_BOX(TraceWin.FilterSelectComboBox), "Nur bekannte");
gtk_combo_box_append_text(GTK_COMBO_BOX(TraceWin.FilterSelectComboBox), "Nur Unbekannte");
gtk_combo_box_set_active(GTK_COMBO_BOX(TraceWin.FilterSelectComboBox), 0);
(void)g_signal_connect(G_OBJECT(TraceWin.FilterSelectComboBox), "changed", G_CALLBACK(FilterSelectComboBoxCB), &TraceWin);
item = GTK_WIDGET(gtk_tool_item_new());
gtk_container_add(GTK_CONTAINER(item), GTK_WIDGET(TraceWin.FilterSelectComboBox));
gtk_container_add(GTK_CONTAINER(TraceWin.Toolbar), GTK_WIDGET(item));
// **** Tabelle
widget = etable_new(GetLine, &TraceWin, TableDescription);
g_object_set(widget, "auto_scroll", TRUE, NULL);
gtk_box_pack_start(GTK_BOX(TraceWin.VBox), widget, TRUE, TRUE, 0);
TraceWin.ETable = widget;
// Empfangs FIFO  erzeugen
//(void)CanExCreateFifo(0x80000000, 10000, NULL, 0, 0xFFFFFFFF); // <*> Fehler auswertung
TraceWin.Event = CanExCreateEvent();   // Event Objekt erstellen
// Events mit API Ereignissen verknüfpen
CanExSetObjEvent(0x80000000, MHS_EVS_OBJECT, TraceWin.Event, RX_EVENT);
// Thread erzeugen
/*#ifdef __WIN32__ <*>
TraceWin.Thread = g_thread_create(ThreadExecute, &TraceWin, TRUE, NULL);
#else*/
TraceWin.Thread = g_thread_new(NULL, ThreadExecute, &TraceWin);
//#endif
return(TraceWin.VBox);
}


void DestroyTraceWin(void)
{
if (TraceWin.Thread)
  {
  // Terminate Event setzen
  CanExSetEvent(TraceWin.Event, MHS_TERMINATE);
  g_thread_join(TraceWin.Thread);
  TraceWin.Thread = NULL;
  }
CBufDestroy(&TraceWin.Buffer);
}

