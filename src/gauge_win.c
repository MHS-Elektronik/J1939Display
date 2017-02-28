/*******************************************************************************
                         gauge_win.c  -  description
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
#include "gauge.h"
#include "can.h"
#include "main.h"
#include "gauge_win.h"


struct TJ1939GaugeWin
  {
  GtkWidget *TorqueGauge;
  GtkWidget *RpmGauge;
  GtkWidget *WaterGauge;
  GtkWidget *OelGauge;
  GtkWidget *OelMapGauge;
  GtkWidget *OelLevelGauge;
  };

static struct TJ1939GaugeWin J1939GaugeWin;


GtkWidget *CreateJ1939GaugeWin(void)
{
GtkWidget *table, *event_box;
gchar *filename;

event_box = gtk_event_box_new();
filename = g_build_filename(BaseDir, "background.jpg", NULL);
SetBackgroundImage(event_box, filename);
safe_free(filename);
                      // 2 Zeilen, 4 Spalten
table = gtk_table_new(2, 4, FALSE);
gtk_container_add(GTK_CONTAINER(event_box), table);
gtk_container_set_border_width(GTK_CONTAINER(table), 5);
gtk_table_set_col_spacings(GTK_TABLE(table), 5);
gtk_table_set_row_spacings(GTK_TABLE(table), 5);
// Zeile 1, Spalte 1/2
J1939GaugeWin.TorqueGauge = mtx_gauge_face_new();
//gtk_widget_realize(J1939GaugeWin.TorqueGauge);
mtx_gauge_face_import_xml(MTX_GAUGE_FACE(J1939GaugeWin.TorqueGauge), "torque_gauge.xml");
gtk_table_attach(GTK_TABLE(table), J1939GaugeWin.TorqueGauge, 0, 2, 0, 1, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 1, Spalte 3/4
J1939GaugeWin.RpmGauge = mtx_gauge_face_new();
//gtk_widget_realize(J1939GaugeWin.RpmGauge);
mtx_gauge_face_import_xml(MTX_GAUGE_FACE(J1939GaugeWin.RpmGauge), "rpm_gauge.xml");
gtk_table_attach(GTK_TABLE(table), J1939GaugeWin.RpmGauge, 2, 4, 0, 1, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 2, Spalte 1
J1939GaugeWin.WaterGauge = mtx_gauge_face_new();
//gtk_widget_realize(J1939GaugeWin.WaterGauge);
mtx_gauge_face_import_xml(MTX_GAUGE_FACE(J1939GaugeWin.WaterGauge), "water_gauge.xml");
gtk_table_attach(GTK_TABLE(table), J1939GaugeWin.WaterGauge, 0, 1, 1, 2, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 2, Spalte 2
J1939GaugeWin.OelGauge = mtx_gauge_face_new();
//gtk_widget_realize(J1939GaugeWin.OelGauge);
mtx_gauge_face_import_xml(MTX_GAUGE_FACE(J1939GaugeWin.OelGauge), "oel_gauge.xml");
gtk_table_attach(GTK_TABLE(table), J1939GaugeWin.OelGauge, 1, 2, 1, 2, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 2, Spalte 3
J1939GaugeWin.OelMapGauge = mtx_gauge_face_new();
//gtk_widget_realize(J1939GaugeWin.OelMapGauge);
mtx_gauge_face_import_xml(MTX_GAUGE_FACE(J1939GaugeWin.OelMapGauge), "oel_map_gauge.xml");
gtk_table_attach(GTK_TABLE(table), J1939GaugeWin.OelMapGauge, 2, 3, 1, 2, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 2, Spalte 4
J1939GaugeWin.OelLevelGauge = mtx_gauge_face_new();
//gtk_widget_realize(J1939GaugeWin.OelLevelGauge);
mtx_gauge_face_import_xml(MTX_GAUGE_FACE(J1939GaugeWin.OelLevelGauge), "oel_level_gauge.xml");
gtk_table_attach(GTK_TABLE(table), J1939GaugeWin.OelLevelGauge, 3, 4, 1, 2, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

return(event_box);
}


void DestroyJ1939GaugeWin(void)
{
}


void UpdateJ1939GaugeWin(struct TJ1939Data *d)
{
if ((!d) ||
    (!J1939GaugeWin.TorqueGauge) ||
    (!J1939GaugeWin.RpmGauge) ||
    (!J1939GaugeWin.WaterGauge) ||
    (!J1939GaugeWin.OelGauge) ||
    (!J1939GaugeWin.OelMapGauge) ||
    (!J1939GaugeWin.OelLevelGauge))
  return;
mtx_gauge_face_set_value(MTX_GAUGE_FACE(J1939GaugeWin.TorqueGauge), d->EngineTorque);
mtx_gauge_face_set_value(MTX_GAUGE_FACE(J1939GaugeWin.RpmGauge), d->Rpm);
mtx_gauge_face_set_value(MTX_GAUGE_FACE(J1939GaugeWin.WaterGauge), d->WaterTemp);
mtx_gauge_face_set_value(MTX_GAUGE_FACE(J1939GaugeWin.OelGauge), d->OilTemp);
mtx_gauge_face_set_value(MTX_GAUGE_FACE(J1939GaugeWin.OelMapGauge), d->OilPressure);
mtx_gauge_face_set_value(MTX_GAUGE_FACE(J1939GaugeWin.OelLevelGauge), d->OilLevel);
}





