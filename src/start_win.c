/*******************************************************************************
                        msg_widget.c  -  description
                             -------------------
    begin             : 09.09.2017
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
#include "main.h"
#include "mhs_g_messages.h"
#include "mhs_msg_types.h"
#include "can_autobaud.h"
#include "msg_widget.h"
#include "start_win.h"


struct TStartWin
  {
  struct TCanAutobaud *CanAutobaud;

  GtkWidget *BaseWdg;
  GtkWidget *DriverWdg;
  GtkWidget *HardwareWdg;
  GtkWidget *AutoBaudWdg;
  };

struct TCanDriverInfo
  {
  gchar *FileName;
  gchar *Description;
  gchar *Hardware;
  gchar *Version;
  };

typedef gchar *(*TInfoTableGetValueFunc)(guint idx, gpointer *data);


static TMhsGScheduler *MainScheduler = NULL;


static const gchar *DriverInfoTableStr[] = {
  "Dateiname",
  "Beschreibung",
/*  "Unterstützte Hardware", <*> */
  "Treiber Version",
  NULL};

static const gchar *DeviceInfoTableStr[] = {
  "Hardware",
  "Seriennummer",
  "Firmware Version",
  NULL};


static void CanDriverInfoFree(struct TCanDriverInfo *driver_info)
{
safe_free(driver_info->FileName);
safe_free(driver_info->Description);
safe_free(driver_info->Hardware);
safe_free(driver_info->Version);
}


static void ExtractDriverInfo(const char *str, struct TCanDriverInfo *driver_info)
{
int match;
char *tmpstr, *s, *key, *val;

tmpstr = g_strdup(str);
s = tmpstr;
do
  {
  // Bezeichner auslesen
  key = get_item_as_string(&s, ":=", &match);
  if (match <= 0)
    break;
  // Value auslesen
  val = get_item_as_string(&s, ";", &match);
  if (match < 0)
    break;
  if (!g_ascii_strcasecmp(key, "Description"))
    {
    driver_info->Description = g_strdup(val);
    continue;
    }
  if (!g_ascii_strcasecmp(key, "Hardware"))
    {
    driver_info->Hardware = g_strdup(val);
    continue;
    }
  if (!g_ascii_strcasecmp(key, "Version"))
    {
    driver_info->Version = g_strdup(val);
    continue;
    }
  }
while(1);
safe_free(tmpstr);
}


static GtkWidget *CreateInfoTable(const gchar **description_list, TInfoTableGetValueFunc get_value_func, gpointer get_value_func_data)
{
GtkWidget *table, *widget;
gchar *desc, *str, *label_str;
guint i;

if (!description_list)
  return(NULL);
// **** Tabelle erzeugen
table = gtk_table_new(4, 2, FALSE);
gtk_table_set_row_spacings(GTK_TABLE(table), 4);
gtk_table_set_col_spacings(GTK_TABLE(table), 3);

for (i = 0; (desc = (gchar *)description_list[i]); i++)
  {
  // **** Bezeichner Label erzeugen
  label_str = g_strdup_printf("<span foreground=\"blue\"><big>%s</big></span>:", desc);
  widget = gtk_label_new(label_str);
  safe_free(label_str);
  gtk_table_attach(GTK_TABLE(table), widget, 0, 1, i, i+1,
                  (GtkAttachOptions)(GTK_FILL),
                  (GtkAttachOptions)(0), 0, 0);
  gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
  gtk_misc_set_alignment(GTK_MISC(widget), 1, 0);

  // **** Value Label erzeugen
  if (get_value_func)
    {
    str =(get_value_func)(i, get_value_func_data);
    label_str = g_strdup_printf("<span foreground=\"red\"><big>%s</big></span>", str);
    widget = gtk_label_new(label_str);
    gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
    gtk_widget_set_size_request(widget, 500, -1);  // <*> ?
    safe_free(str);
    safe_free(label_str);
    }
  gtk_table_attach(GTK_TABLE(table), widget, 1, 2, i, i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(0), 0, 0);
  gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
  gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);
  }
return(table);
}


static gchar *GetDriverInfoValue(guint idx, gpointer *data)
{
struct TCanDriverInfo *driver_info;

driver_info = (struct TCanDriverInfo *)data;
switch (idx)
  {
  case 0 : { return(g_strdup(driver_info->FileName)); }
  case 1 : { return(g_strdup(driver_info->Description)); }
//  case 2 : { return(g_strdup(driver_info->Hardware)); } <*> 
  case 2 : { return(g_strdup(driver_info->Version)); }
  default : return(NULL);
  }
}


static GtkWidget *CreateDriverInfoTable(struct TCanCore *can_core)
{
struct TCanDriverInfo driver_info;
GtkWidget *table;

driver_info.FileName = NULL;
driver_info.Description = NULL;
driver_info.Hardware = NULL;
driver_info.Version = NULL;
ExtractDriverInfo(can_core->DriverInfoStr, &driver_info);
driver_info.FileName = g_strdup(can_core->DriverFileName);
table = CreateInfoTable(DriverInfoTableStr, GetDriverInfoValue, (gpointer)&driver_info);
CanDriverInfoFree(&driver_info);
return(table);
}


static gchar *GetDeviceInfoValue(guint idx, gpointer *data)
{
struct TCanDeviceInfo *info;
guint ver, ver2;

info = (struct TCanDeviceInfo *)data;
switch (idx)
  {
  case 0 : { return(g_strdup(info->Description)); }
  case 1 : { return(g_strdup(info->SerialNumber)); }
  case 2 : {
           ver = info->FirmwareVersion / 1000;
           ver2 = info->FirmwareVersion % 1000;
           return(g_strdup_printf("%u.%02u", ver, ver2));
           }
  default : return(NULL);
  }
}


static GtkWidget *CreateDeviceInfoTable(struct TCanCore *can_core)
{
GtkWidget *table;

if (can_core->AppCanStatus < APP_CAN_OPEN)
  return(NULL);
table = CreateInfoTable(DeviceInfoTableStr, GetDeviceInfoValue, (gpointer)&can_core->DeviceInfo);
return(table);
}


static void SchedulerMsgsCB(TMhsGMessage *msg, gpointer user_data)
{
GtkWidget *widget;
int32_t num_devs;
gchar *str, *msg_str;
guint status;
gint err;
struct TStartWin *start_win;

start_win = (struct TStartWin *)user_data;
msg_str = (gchar *)msg->Data;

err = 0;
// ******* Tiny-CAN Status ********
if (msg->MessageType == MHS_MSG_PNP_EVENT)
  {
  num_devs = *((int32_t *)msg->Data);
  MsgWidgetDestroyCustomWidget(start_win->HardwareWdg);
  if (num_devs <= 0)
    {
    err = 1;
    UpdateMsgWidget(start_win->HardwareWdg, MSG_WDG_STATUS_WAIT, "<span size=\"x-large\">Tiny-CAN Status</span>\n"
                                                 "<b>Tiny-CAN nicht verbunden</b>");
    }
  else
    {
    UpdateMsgWidget(start_win->HardwareWdg, MSG_WDG_STATUS_BUSY, "<span size=\"x-large\">Tiny-CAN Status</span>\n"
                                               "<b>Initialisierung, Status unbekannt</b>");
    UpdateGtk();
    if (J1939CanOpen(start_win->CanAutobaud->CanCore) < 0)
      {
      err = 1;
      str = g_strdup_printf("<span size=\"x-large\">Tiny-CAN Status</span>\n"
                            "<b>Fehler beim Öffnen des CAN Devices</b>\n"
                            "%s", start_win->CanAutobaud->CanCore->LastErrorString);
      UpdateMsgWidget(start_win->HardwareWdg, MSG_WDG_STATUS_ERROR, str);
      safe_free(str);
      }
    else
      {
      UpdateMsgWidget(start_win->HardwareWdg, MSG_WDG_STATUS_OK, "<span size=\"x-large\">Tiny-CAN Status</span>\n"
                                               "<b>Verbindung zum Tiny-CAN hergestellt</b>");
      widget = CreateDeviceInfoTable(start_win->CanAutobaud->CanCore);
      MsgWidgetAddCustomWidget(start_win->HardwareWdg, widget);
      }
    }
  // ******* Automatische Baudratenerkennung *******
  if (err)
    {
    UpdateMsgWidget(start_win->AutoBaudWdg, MSG_WDG_STATUS_STOP, "<span size=\"x-large\">Automatische Baudratenerkennung</span>\n"
                                             "<b>CAN Schnittstelle nicht geöffnet</b>");
    }
  else
    {
    UpdateMsgWidget(start_win->AutoBaudWdg, MSG_WDG_STATUS_WAIT, "<span size=\"x-large\">Automatische Baudratenerkennung</span>\n"
                                             "<b>Baudratenerkennung wird gestartet ...</b>");
    }
  UpdateGtk();
  if (err)
    {
    J1939CanClose(start_win->CanAutobaud->CanCore);
    StopCanAutobaud(start_win->CanAutobaud);
    }
  else
    StartCanAutobaud(start_win->CanAutobaud, start_win->CanAutobaud->CanCore->CanSpeed);
  }
else
  {
  str = g_strdup_printf("<span size=\"x-large\">Automatische Baudratenerkennung</span>\n"
                  "<b>%s</b>", msg_str);
  switch (msg->MessageType)
    {
    case MHS_MSG_AUTOBAUD_START  : {
                                   status = MSG_WDG_STATUS_BUSY;
                                   break;
                                   }
    case MHS_MSG_AUTOBAUD_RUN    : {
                                   status = MSG_WDG_STATUS_BUSY;
                                   break;
                                   }
    case MHS_MSG_AUTOBAUD_OK     : {
                                   status = MSG_WDG_STATUS_OK;
                                   SetAppStatus(APP_START_FINISH);
                                   break;
                                   }
    case MHS_MSG_AUTOBAUD_ERROR  : {
                                   status = MSG_WDG_STATUS_STOP;
                                   break;
                                   }
    default                      : status = MSG_WDG_STATUS_STOP;
    }
  UpdateMsgWidget(start_win->AutoBaudWdg, status, str);
  safe_free(str);
  }
}


static void StartWinDestroyCB(GtkWidget *widget, gpointer data)
{
struct TStartWin *start_win;
(void)widget;

start_win = (struct TStartWin *)data;
DestroyCanAutobaud(&start_win->CanAutobaud);
g_free(start_win);
}


GtkWidget *CreateStartWin(struct TCanCore *can_core)
{
gchar *str;
GtkWidget *event_box, *box, *widget;
struct TStartWin *start_win;
gint err;
gchar *filename;

if (!(start_win = (struct TStartWin *)g_malloc0(sizeof(struct TStartWin))))
  return(NULL);
event_box = gtk_event_box_new();
g_object_set_data(G_OBJECT(event_box), "start_win", start_win);
g_signal_connect(G_OBJECT(event_box), "destroy", G_CALLBACK(StartWinDestroyCB), start_win);

filename = g_build_filename(BaseDir, "background.jpg", NULL);
SetBackgroundImage(event_box, filename);
safe_free(filename);

box = gtk_vbox_new(FALSE, 3);
gtk_container_add(GTK_CONTAINER(event_box), box);
gtk_container_set_border_width(GTK_CONTAINER(box), 2);

err = 0;
// ******* System *******
if (can_core->AppCanStatus == APP_CAN_DRIVER_ERROR)
  {
  err = 1;
  str = g_strdup_printf("<span size=\"x-large\">System</span>\n"
                        "<b>Laden und Initialisieren des Tiny-CAN API Treibers fehlgeschlagen</b>\n"
                        "%s", can_core->LastErrorString);
  start_win->DriverWdg = CreateMsgWidget(MSG_WDG_STATUS_STOP, str);
  safe_free(str);
  }
else
  {
  start_win->DriverWdg = CreateMsgWidget(MSG_WDG_STATUS_OK, "<span size=\"x-large\">System</span>\n"
                                         "<b>Tiny-CAN API Treiber geladen und Initialisiert</b>");
  if (can_core->DriverInfoStr)
    {
    widget = CreateDriverInfoTable(can_core);
    MsgWidgetAddCustomWidget(start_win->DriverWdg, widget);
    }
  }
gtk_box_pack_start(GTK_BOX(box), start_win->DriverWdg, FALSE, FALSE, 0);
// ******* Tiny-CAN Status ********
if (err)
  {
  start_win->HardwareWdg = CreateMsgWidget(MSG_WDG_STATUS_STOP, "<span size=\"x-large\">Tiny-CAN Status</span>\n"
                                           "<b>Fataler Fehler: Treiber API nicht geladen</b>");
  }
else
  {
  start_win->HardwareWdg = CreateMsgWidget(MSG_WDG_STATUS_BUSY, "<span size=\"x-large\">Tiny-CAN Status</span>\n"
                                           "<b>Initialisierung, Status unbekannt</b>");
  }
gtk_box_pack_start(GTK_BOX(box), start_win->HardwareWdg, FALSE, FALSE, 0);
// ******* Automatische Baudratenerkennung *******
if (err)
  {
  start_win->AutoBaudWdg = CreateMsgWidget(MSG_WDG_STATUS_STOP, "<span size=\"x-large\">Automatische Baudratenerkennung</span>\n"
                                           "<b>Fataler Fehler: Treiber API nicht geladen</b>");
  }
else
  {
  start_win->AutoBaudWdg = CreateMsgWidget(MSG_WDG_STATUS_BUSY, "<span size=\"x-large\">Automatische Baudratenerkennung</span>\n"
                                           "<b>Tiny-CAN Hardware noch nicht geöffnet</b>");

  }
gtk_box_pack_start(GTK_BOX(box), start_win->AutoBaudWdg, FALSE, FALSE, 0);

start_win->BaseWdg = box;
if (!err)
  {
  MainScheduler = mhs_g_message_scheduler_create(SchedulerMsgsCB, (gpointer)start_win, TRUE);
  start_win->CanAutobaud = CreateCanAutobaud(MainScheduler, can_core);
  //(void)J1939PnPStart(can_core, MainScheduler); <*>
  //J1939PnPScan(can_core);
  }
gtk_widget_show_all(event_box);
return(event_box);
}


void StartWinRun(GtkWidget *widget)
{
struct TStartWin *start_win;

if (!(start_win = (struct TStartWin *)g_object_get_data(G_OBJECT(widget), "start_win")))
  return;
if (MainScheduler)
  {
  (void)J1939PnPStart(start_win->CanAutobaud->CanCore, MainScheduler);
  J1939PnPScan(start_win->CanAutobaud->CanCore);
  }
}


void StartWinStop(GtkWidget *widget)
{
struct TStartWin *start_win;

if (!(start_win = (struct TStartWin *)g_object_get_data(G_OBJECT(widget), "start_win")))
  return;
StopCanAutobaud(start_win->CanAutobaud);
}


