/*******************************************************************************
                          lcd_win.c  -  description
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
#include "gtk-lcd.h"
#include "gtk-digital-show.h"
#include "can.h"
#include "main.h"
#include "lcd_win.h"

struct TJ1939Win
  {
  GtkWidget *OilTemp;
  GtkWidget *OilLevel;
  GtkWidget *OilPressure;
  GtkWidget *WaterTemp;
  GtkWidget *FuelTemp;
  GtkWidget *EngineTorque;
  GtkWidget *Rpm;
  GtkWidget *AirTemp;
  GtkWidget *TurbochargerPressure;
  GtkWidget *FuelConsumptionInL;
  GtkWidget *FuelConsumptionInKg;
  GtkWidget *OutputPower;
  };

static struct TJ1939Win J1939Win;

static GdkColor WarnBgColor = { 0, 0xFFFF, 0xFFFF, 0x6600 };  // hell-gelb
static GdkColor WarnFgColor = { 0, 0xFFFF, 0xFFFF, 0      };  // gelb

static GdkColor LimitBgColor = { 0, 0xFFFF, 0x80FF, 0x80FF };  // hell-rot
static GdkColor LimitFgColor = { 0, 0xFFFF, 0,      0x0000 };  // rot


GtkWidget *CreateJ1939LcdWin(void)
{
GtkWidget *table, *event_box;
gchar *filename;

event_box = gtk_event_box_new();
filename = g_build_filename(BaseDir, "background.jpg", NULL);
SetBackgroundImage(event_box, filename);
safe_free(filename);
                      // 4 Zeilen, 3 Spalten
table = gtk_table_new(4, 3, FALSE);
gtk_container_add(GTK_CONTAINER(event_box), table);
gtk_container_set_border_width(GTK_CONTAINER(table), 5);
gtk_table_set_col_spacings(GTK_TABLE(table), 5);
gtk_table_set_row_spacings(GTK_TABLE(table), 5);
// Zeile 1, Spalte 1
J1939Win.OilTemp = gtk_digital_show_new_with_config("<b> Öltemperatur </b>", "<b>°C</b>", 0, TRUE, -40.0, 150.0, 0.0, NULL, NULL);
g_object_set(G_OBJECT(J1939Win.OilTemp), "min_warn_fg_color", &WarnFgColor, "min_warn_bg_color", &WarnBgColor,
             "min_limit_fg_color", &LimitFgColor, "min_limit_bg_color", &LimitBgColor, "min_limit", -20.0, "min_warn", 30.0, NULL);
g_object_set(G_OBJECT(J1939Win.OilTemp), "max_warn_fg_color", &WarnFgColor, "max_warn_bg_color", &WarnBgColor,
             "max_limit_fg_color", &LimitFgColor, "max_limit_bg_color", &LimitBgColor, "max_warn", 110.0, "max_limit", 130.0, NULL);             
gtk_table_attach(GTK_TABLE(table), J1939Win.OilTemp, 0, 1, 0, 1, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 1, Spalte 2
J1939Win.OilLevel = gtk_digital_show_new_with_config("<b> Ölstand </b>", "<b>%</b>", 0, TRUE, 0.0, 100.0, 0.0, NULL, NULL);
g_object_set(G_OBJECT(J1939Win.OilLevel), "min_warn_fg_color", &WarnFgColor, "min_warn_bg_color", &WarnBgColor,
             "min_limit_fg_color", &LimitFgColor, "min_limit_bg_color", &LimitBgColor, "min_limit", 20.0, "min_warn", 40.0, NULL);
g_object_set(G_OBJECT(J1939Win.OilLevel), "max_warn_fg_color", &WarnFgColor, "max_warn_bg_color", &WarnBgColor,
             "max_limit_fg_color", &LimitFgColor, "max_limit_bg_color", &LimitBgColor, "max_warn", 90.0, "max_limit", 95.0, NULL); 
gtk_table_attach(GTK_TABLE(table), J1939Win.OilLevel, 1, 2, 0, 1, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 1, Spalte 3
J1939Win.OilPressure = gtk_digital_show_new_with_config("<b> Öldruck </b>", "<b>Bar</b>", 1, TRUE, 0.0, 10.0, 0.0, NULL, NULL);
g_object_set(G_OBJECT(J1939Win.OilPressure), "min_warn_fg_color", &WarnFgColor, "min_warn_bg_color", &WarnBgColor,
             "min_limit_fg_color", &LimitFgColor, "min_limit_bg_color", &LimitBgColor, "min_limit", 0.3, "min_warn", 0.5, NULL);
g_object_set(G_OBJECT(J1939Win.OilPressure), "max_warn_fg_color", &WarnFgColor, "max_warn_bg_color", &WarnBgColor,
             "max_limit_fg_color", &LimitFgColor, "max_limit_bg_color", &LimitBgColor, "max_warn", 9.0, "max_limit", 9.5, NULL);
gtk_table_attach(GTK_TABLE(table), J1939Win.OilPressure, 2, 3, 0, 1, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

// Zeile 2, Spalte 1
J1939Win.WaterTemp = gtk_digital_show_new_with_config("<b> Wassertemperatur </b>", "<b>°C</b>", 0, TRUE, -40.0, 120.0, 0.0, NULL, NULL);
g_object_set(G_OBJECT(J1939Win.WaterTemp), "min_warn_fg_color", &WarnFgColor, "min_warn_bg_color", &WarnBgColor,
             "min_limit_fg_color", &LimitFgColor, "min_limit_bg_color", &LimitBgColor, "min_limit", -20.0, "min_warn", 20.0, NULL);
g_object_set(G_OBJECT(J1939Win.WaterTemp), "max_warn_fg_color", &WarnFgColor, "max_warn_bg_color", &WarnBgColor,
             "max_limit_fg_color", &LimitFgColor, "max_limit_bg_color", &LimitBgColor, "max_warn", 100.0, "max_limit", 110.0, NULL);
gtk_table_attach(GTK_TABLE(table), J1939Win.WaterTemp, 0, 1, 1, 2, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 2, Spalte 2
J1939Win.FuelTemp = gtk_digital_show_new_with_config("<b> Kraftstofftemperatur </b>", "<b>°C</b>", 0, TRUE, -40.0, 120.0, 0.0, NULL, NULL);
g_object_set(G_OBJECT(J1939Win.FuelTemp), "min_warn_fg_color", &WarnFgColor, "min_warn_bg_color", &WarnBgColor,
             "min_limit_fg_color", &LimitFgColor, "min_limit_bg_color", &LimitBgColor, "min_limit", -20.0, "min_warn", 0.0, NULL);
g_object_set(G_OBJECT(J1939Win.FuelTemp), "max_warn_fg_color", &WarnFgColor, "max_warn_bg_color", &WarnBgColor,
             "max_limit_fg_color", &LimitFgColor, "max_limit_bg_color", &LimitBgColor, "max_warn", 60.0, "max_limit", 80.0, NULL);
gtk_table_attach(GTK_TABLE(table), J1939Win.FuelTemp, 1, 2, 1, 2, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 2, Spalte 3
J1939Win.EngineTorque = gtk_digital_show_new_with_config("<b> Drehmoment </b>", "<b>%</b>", 0, TRUE, 0.0, 100.0, 0.0, NULL, NULL);
g_object_set(G_OBJECT(J1939Win.FuelTemp), "max_warn_fg_color", &WarnFgColor, "max_warn_bg_color", &WarnBgColor,
             "max_limit_fg_color", &LimitFgColor, "max_limit_bg_color", &LimitBgColor, "max_warn", 90.0, "max_limit", 95.0, NULL);
gtk_table_attach(GTK_TABLE(table), J1939Win.EngineTorque, 2, 3, 1, 2, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 3, Spalte 1
J1939Win.Rpm = gtk_digital_show_new_with_config("<b> Drehzahl </b> ", "<b>1/min</b>", 0, TRUE, 0.0, 3000.0, 0.0, NULL, NULL);
g_object_set(G_OBJECT(J1939Win.Rpm), "min_warn_fg_color", &WarnFgColor, "min_warn_bg_color", &WarnBgColor,
             "min_limit_fg_color", &LimitFgColor, "min_limit_bg_color", &LimitBgColor, "min_limit", 400.0, "min_warn", 600.0, NULL);
g_object_set(G_OBJECT(J1939Win.Rpm), "max_warn_fg_color", &WarnFgColor, "max_warn_bg_color", &WarnBgColor,
             "max_limit_fg_color", &LimitFgColor, "max_limit_bg_color", &LimitBgColor, "max_warn", 2000.0, "max_limit", 2500.0, NULL);
gtk_table_attach(GTK_TABLE(table), J1939Win.Rpm, 0, 1, 2, 3, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 3, Spalte 2
J1939Win.AirTemp = gtk_digital_show_new_with_config("<b> Ladelufttemperatur </b>", "<b>°C</b>", 0, TRUE, -40.0, 120.0, 0.0, NULL, NULL);
g_object_set(G_OBJECT(J1939Win.AirTemp), "min_warn_fg_color", &WarnFgColor, "min_warn_bg_color", &WarnBgColor,
             "min_limit_fg_color", &LimitFgColor, "min_limit_bg_color", &LimitBgColor, "min_limit", -20.0, "min_warn", 0.0, NULL);
g_object_set(G_OBJECT(J1939Win.AirTemp), "max_warn_fg_color", &WarnFgColor, "max_warn_bg_color", &WarnBgColor,
             "max_limit_fg_color", &LimitFgColor, "max_limit_bg_color", &LimitBgColor, "max_warn", 60.0, "max_limit", 80.0, NULL);
gtk_table_attach(GTK_TABLE(table), J1939Win.AirTemp, 1, 2, 2, 3, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 3, Spalte 3
J1939Win.TurbochargerPressure = gtk_digital_show_new_with_config("<b> Ladedruck </b>", "<b>Bar</b>", 2, TRUE, 0.0, 4.0, 0.0, NULL, NULL);
gtk_table_attach(GTK_TABLE(table), J1939Win.TurbochargerPressure, 2, 3, 2, 3, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

// Zeile 3, Spalte 1
J1939Win.FuelConsumptionInL = gtk_digital_show_new_with_config("<b> Verbrauch </b>", "<b>l/h</b>", 1, TRUE, 0.0, 100.0, 0.0, NULL, NULL);
gtk_table_attach(GTK_TABLE(table), J1939Win.FuelConsumptionInL, 0, 1, 3, 4, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

J1939Win.FuelConsumptionInKg = gtk_digital_show_new_with_config("<b> kg </b>", "<b>h</b>", 1, TRUE, 0.0, 100.0, 0.0, NULL, NULL);
gtk_table_attach(GTK_TABLE(table), J1939Win.FuelConsumptionInKg, 1, 2, 3, 4, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

J1939Win.OutputPower = gtk_digital_show_new_with_config("<b> Motorleistung </b>", "<b>KW</b>", 1, TRUE, 0.0, 500.0, 0.0, NULL, NULL);
gtk_table_attach(GTK_TABLE(table), J1939Win.OutputPower, 2, 3, 3, 4, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
return(event_box);
}


void DestroyJ1939LcdWin(void)
{
}


void UpdateJ1939LcdWin(struct TJ1939Data *d)
{
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.OilTemp), d->OilTemp);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.OilLevel), d->OilLevel);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.OilPressure), d->OilPressure);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.WaterTemp), d->WaterTemp);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.FuelTemp), d->FuelTemp);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.EngineTorque), d->EngineTorque);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.Rpm), d->Rpm);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.AirTemp), d->AirTemp);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.TurbochargerPressure), d->TurbochargerPressure);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.FuelConsumptionInL), d->FuelConsumptionInL);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.FuelConsumptionInKg), d->FuelConsumptionInKg);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(J1939Win.OutputPower), d->OutputPower);
}
