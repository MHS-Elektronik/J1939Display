/*******************************************************************************
                         rrd_tool_database.c  -  description
                             -------------------
    begin             : 07.09.2017
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
//#include <stdio.h>
//#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "util.h"
#include "can.h"
#include "setup.h"
#include "main.h"
#include "rrd_tool_database.h"

static const guint RRDtoolAddTime = 10000;  // 10 Sek.
static const guint RRDtoolGraph1UpdateTime = 600000;  // 10 Min. = 60000 * 10 = 600000
static const guint RRDtoolGraph2UpdateTime = 3600000; // 60 Min. = 60000 * 60 = 3600000

static const gchar RRDtoolDatabaseFilename[] = {"engine.rrd"};
static const gchar RRDtoolGraph1Filename[] = {"engine_graph1.png"};
static const gchar RRDtoolGraph2Filename[] = {"engine_graph2.png"};

static const gchar RRDtoolExec[] = {"rrdtool"};

static const gchar RRDtoolCreateCmd[] = {" create %s --step 15 "};  // # Sekunden (Update Intervall), 0,25 Min

static const gchar RRDtoolUpdateCmd[] = {" update %s "};

//DS:<Name>:<Typ>:<Heartbeat>:<Min>:<Max>
static const gchar RRDtoolDatabase[]  = {
   "DS:OilTemp:GAUGE:120:-40:240 "
   "DS:OilLevel:GAUGE:120:0:100 "
   "DS:OilPressure:GAUGE:120:0:10 "
   "DS:WaterTemp:GAUGE:120:-40:240 "
   "DS:FuelTemp:GAUGE:120:-40:240 "
   "DS:EngineTorque:GAUGE:120:0:100 "
   "DS:Rpm:GAUGE:120:0:6000 "
   "DS:AirTemp:GAUGE:120:-40:240 "
   "DS:TurbochargerPress:GAUGE:120:0:10 "
   "DS:FuelConsumptionInL:GAUGE:120:0:100 "
   "RRA:AVERAGE:0.5:1:240 "   // 15 Sek. * 240 = 3600Sek. (60 Min)
   "RRA:AVERAGE:0.5:20:288"}; // (15 Sek * 20) * 288 = 86400Sek. (24 Std.)


static const gchar RRDtoolGraphCmd[] = {" graph %s -a PNG "};

static const gchar RRDtoolGraph1[] = {
  "-w 800 -h 500 -t Temperaturen "
  "--end now --start end-3600 "  // end-<Sekunden>
  "--disable-rrdtool-tag "
  "--upper-limit 150 --lower-limit 0 "
  "--y-grid 20:1 "
  "--vertical-label \"Temperatur (°C)\" "
  "DEF:OilTemp=%s:OilTemp:AVERAGE "
  "DEF:WaterTemp=%s:WaterTemp:AVERAGE "
  "DEF:FuelTemp=%s:FuelTemp:AVERAGE "
  "DEF:AirTemp=%s:AirTemp:AVERAGE "
  "LINE1:OilTemp#00CC00:Öltemperatur "
  "LINE1:WaterTemp#00CCCC:Wassertemperatur "
  "LINE1:FuelTemp#FF8000:Kraftstofftemperatur "
  "LINE1:AirTemp#FF0000:Ladelufttemperatur"};

static const gchar RRDtoolGraph2[] = {
  "-w 800 -h 500 -t Temperaturen "
  "--end now --start end-86400 "  // end-<Sekunden>
  "--disable-rrdtool-tag "
  "--upper-limit 150 --lower-limit 0 "
  "--y-grid 20:1 "
  "--vertical-label \"Temperatur (°C)\" "
  "DEF:OilTemp=%s:OilTemp:AVERAGE "
  "DEF:WaterTemp=%s:WaterTemp:AVERAGE "
  "DEF:FuelTemp=%s:FuelTemp:AVERAGE "
  "DEF:AirTemp=%s:AirTemp:AVERAGE "
  "LINE1:OilTemp#00CC00:Öltemperatur "
  "LINE1:WaterTemp#00CCCC:Wassertemperatur "
  "LINE1:FuelTemp#FF8000:Kraftstofftemperatur "
  "LINE1:AirTemp#FF0000:Ladelufttemperatur"};  


static const gchar RRDtoolInsertData[] = {"N:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f"};


struct TRRDtool
  {
  guint64 LastEventTime;
  guint64 Graph1LastEventTime;
  guint64 Graph2LastEventTime;
  };

struct TRRDtool *RRDtool = NULL;


gint RRDtoolCreate(void)
{
gint res;
gchar *str, *exec, *filename;
GError *error;
struct TRRDtool *rrd_tool;

res = 0;
if (!Setup.EnableRRDtool)
  return(0);
if (!(rrd_tool = (struct TRRDtool *)g_malloc0(sizeof(struct TRRDtool))))
  return(-1);
error = NULL;
exec = g_strconcat(RRDtoolExec, RRDtoolCreateCmd, RRDtoolDatabase, NULL);
filename = g_build_filename(Setup.RRDtoolDbPath, RRDtoolDatabaseFilename, NULL);
str = g_strdup_printf(exec, filename);
if (g_spawn_command_line_async(str, &error) == FALSE)
  {
  g_debug("rrdtool create error: %s", error->message);
  g_error_free(error);
  res = -1;
  }
safe_free(exec);
safe_free(str);
safe_free(filename);
if (res < 0)
  safe_free(rrd_tool);
else
  RRDtool = rrd_tool;
return(res);
}


void RRDtoolDestroy(void)
{
safe_free(RRDtool);
}


static void RRDtoolGraph1Paint(void)
{
gchar *str, *cmd_str, *graph_str, *filename;
GError *error;

if (!RRDtool)
  return;
error = NULL;
filename = g_build_filename(Setup.RRDtoolDbPath, RRDtoolGraph1Filename, NULL);
cmd_str = g_strdup_printf(RRDtoolGraphCmd, filename);
safe_free(filename);
filename = g_build_filename(Setup.RRDtoolDbPath, RRDtoolDatabaseFilename, NULL);
graph_str = g_strdup_printf(RRDtoolGraph1, filename, filename, filename, filename);
str = g_strconcat(RRDtoolExec, cmd_str, graph_str, NULL);
if (g_spawn_command_line_async(str, &error) == FALSE)
  {
  g_debug("rrdtool update error: %s", error->message);
  g_error_free(error);
  }
safe_free(filename);
safe_free(str);
safe_free(cmd_str);
safe_free(graph_str);
}


static void RRDtoolGraph2Paint(void)
{
gchar *str, *cmd_str, *graph_str, *filename;
GError *error;

if (!RRDtool)
  return;
error = NULL;
filename = g_build_filename(Setup.RRDtoolDbPath, RRDtoolGraph2Filename, NULL);
cmd_str = g_strdup_printf(RRDtoolGraphCmd, filename);
safe_free(filename);
filename = g_build_filename(Setup.RRDtoolDbPath, RRDtoolDatabaseFilename, NULL);
graph_str = g_strdup_printf(RRDtoolGraph2, filename, filename, filename, filename);
str = g_strconcat(RRDtoolExec, cmd_str, graph_str, NULL);
if (g_spawn_command_line_async(str, &error) == FALSE)
  {
  g_debug("rrdtool update error: %s", error->message);
  g_error_free(error);
  }
safe_free(filename);
safe_free(str);
safe_free(cmd_str);
safe_free(graph_str);
}


void UpdateRRDtoolDatabase(struct TJ1939Data *d)
{
gchar *s, *str, *exec, *filename;
gchar c;
GError *error;

if (!RRDtool)
  return;
error = NULL;
if ((RRDtool->LastEventTime > TimeNow) ||
   ((TimeNow - RRDtool->LastEventTime) >= (RRDtoolAddTime * 1000)))
  {
  RRDtool->LastEventTime = TimeNow;
  exec = g_strconcat(RRDtoolExec, RRDtoolUpdateCmd, RRDtoolInsertData, NULL);
  filename = g_build_filename(Setup.RRDtoolDbPath, RRDtoolDatabaseFilename, NULL);
  str = g_strdup_printf(exec, filename, d->OilTemp, d->OilLevel, d->OilPressure, d->WaterTemp, d->FuelTemp,
                  d->EngineTorque,d->Rpm,d->AirTemp,d->TurbochargerPressure,d->FuelConsumptionInL);
  for (s = str; (c = *s); s++)
    {
    if (c == ',')
      *s = '.';
    }
  if (g_spawn_command_line_async(str, &error) == FALSE)
    {
    g_debug("rrdtool update error: %s", error->message);
    g_error_free(error);
    }
  safe_free(filename);
  safe_free(exec);
  safe_free(str);
  }
if ((RRDtool->Graph1LastEventTime > TimeNow) ||
   ((TimeNow - RRDtool->Graph1LastEventTime) >= (RRDtoolGraph1UpdateTime * 1000)))
  {
  RRDtool->Graph1LastEventTime = TimeNow;
  RRDtoolGraph1Paint();
  }
if ((RRDtool->Graph2LastEventTime > TimeNow) ||
   ((TimeNow - RRDtool->Graph2LastEventTime) >= (RRDtoolGraph2UpdateTime * 1000)))
  {
  RRDtool->Graph2LastEventTime = TimeNow;
  RRDtoolGraph2Paint();
  }  
}
