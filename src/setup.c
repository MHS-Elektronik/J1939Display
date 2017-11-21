/***************************************************************************
                           setup.c  -  description
                             -------------------
    begin             : 07.10.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include <gtk/gtk.h>
#include "main.h"
#include "util.h"
#include "configfile.h"
#include "setup.h"


static const gchar CONFIG_FILE_NAME[] = "J1939Display.cfg";

struct TSetup Setup;

gint LoadConfigFile(void)
{
ConfigFile *cfgfile;
gchar *filename;

if (!(filename = g_build_filename(BaseDir, CONFIG_FILE_NAME, NULL)))
  return(-1);
if (!(cfgfile = cfg_open_file(filename)))
  {
  g_free(filename);
  return(-1);
  }
// Sektion GLOBAL
(void)cfg_read_int(cfgfile, "GLOBAL", "ShowFullscreen", &Setup.ShowFullscreen);
// Sektion XMLDATABASE
(void)cfg_read_int(cfgfile, "XMLDATABASE", "Enable", &Setup.EnableXMLDatabase);
safe_free(Setup.XMLDbPath);
(void)cfg_read_string(cfgfile, "XMLDATABASE", "DbPath", &Setup.XMLDbPath);
// Sektion MODBUS
(void)cfg_read_int(cfgfile, "MODBUS", "Enable", &Setup.EnableModbus);
(void)cfg_read_int(cfgfile, "MODBUS", "ModbusPort", &Setup.ModbusPort);
// Sektion RRDTOOL
(void)cfg_read_int(cfgfile, "RRDTOOL", "Enable", &Setup.EnableRRDtool);
safe_free(Setup.RRDtoolDbPath);
(void)cfg_read_string(cfgfile, "RRDTOOL", "DbPath", &Setup.RRDtoolDbPath);
// Sektion SQLITE
(void)cfg_read_int(cfgfile, "SQLITE", "Enable", &Setup.EnableSqlite);
safe_free(Setup.SqliteDbPath);
(void)cfg_read_string(cfgfile, "SQLITE", "DbPath", &Setup.SqliteDbPath);
cfg_free(cfgfile);
g_free(filename);
return(0);
}


void ConfigDestroy(void)
{
safe_free(Setup.XMLDbPath);
safe_free(Setup.RRDtoolDbPath);
safe_free(Setup.SqliteDbPath);
}





