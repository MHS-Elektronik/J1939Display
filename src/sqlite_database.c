/*******************************************************************************
                         sqlite_database.c  -  description
                             -------------------
    begin             : 04.09.2017
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
#include <sqlite3.h>
#include "util.h"
#include "can.h"
#include "setup.h"
#include "main.h"
#include "sqlite_database.h"

static const guint SqliteAddTime = 5000;

static const gchar SqliteDatabaseFilename[] = {"engine.db"};

static const char SqlTableExistQuery[] = {"SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = 'engine';"};

static const char SqlTableCreateQuery[] = {"CREATE TABLE 'engine' ("
          "'Time'                 DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
          "'OilTemp'              REAL NOT NULL,"
          "'OilLevel'             REAL NOT NULL,"
          "'OilPressure'          REAL NOT NULL,"
          "'WaterTemp'            REAL NOT NULL,"
          "'FuelTemp'             REAL NOT NULL, "
          "'EngineTorque'         REAL NOT NULL,"
          "'Rpm'                  REAL NOT NULL,"
          "'AirTemp'              REAL NOT NULL,"
          "'TurbochargerPressure' REAL NOT NULL,"
          "'FuelConsumptionInL'   REAL NOT NULL)"};

static const char SqlTableInsertQuery[] = {"INSERT INTO 'engine' ("
          "'OilTemp','OilLevel','OilPressure','WaterTemp','FuelTemp','EngineTorque',"
          "'Rpm','AirTemp','TurbochargerPressure','FuelConsumptionInL') "
          "VALUES (%f, %f, %f, %f, %f, %f, %f, %f, %f, %f)"};


static sqlite3 *DB = NULL;
static guint64 LastEventTime = 0;

static sqlite3 *open_database(void)
{
gchar *filename;
sqlite3 *db;

filename = g_build_filename(Setup.SqliteDbPath, SqliteDatabaseFilename, NULL);
if (sqlite3_open(filename, &db) != SQLITE_OK)
  {
  //g_strdup_printf("Error opening database: %s\n", sqlite3_errmsg(db)); <*>
  safe_free(filename);
  return(NULL);
  }
else
  {
  safe_free(filename);
  return(db);
  }
}


static gint setup_table(sqlite3 *db)
{
sqlite3_stmt *stmt;
int count;
gint res;
gchar *err_str;

res = 0;
err_str = NULL;
if (sqlite3_prepare_v2(db, SqlTableExistQuery, -1, &stmt, NULL) != SQLITE_OK)
  {
  err_str = g_strdup_printf("SQLite Database check exist error: %s", sqlite3_errmsg(db));
  res = -1;
  }
if (!res)
  {
  if (sqlite3_step(stmt) != SQLITE_ROW)
    {
    err_str = g_strdup_printf("SQLite Database check exist error: %s", sqlite3_errmsg(db));
    res = -1;
    }
  else
    {
    count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    if (count == 1)
      return(0);
    }
  }
if (!res)
  {
  if (sqlite3_prepare_v2(db, SqlTableCreateQuery, -1, &stmt, NULL) != SQLITE_OK)
    {
    err_str = g_strdup_printf("SQLite Create Database error: %s", sqlite3_errmsg(db));
    res = -1;
    }
  }
if (!res)
  {
  if (sqlite3_step(stmt) != SQLITE_DONE)
    {
    err_str = g_strdup_printf("SQLite Create Database error: %s", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    res = -1;
    }
  sqlite3_finalize(stmt);
  }

safe_free(err_str);

return(res);
}


gint SqliteDBCreate(void)
{
if (!Setup.EnableSqlite)
  return(0);
if (!(DB = open_database()))
  return(-1);
if (setup_table(DB) < 0)
  {
  sqlite3_close(DB);
  DB = NULL;
  return(-1);
  }
else
  return(0);
}


void SqliteDBDestroy(void)
{
if (!DB)
  return;
sqlite3_close(DB);
DB = NULL;
}


void UpdateSqliteDatabase(struct TJ1939Data *d)
{
char *str, *errmsg;

if (!DB)
  return;
if ((LastEventTime > TimeNow) ||
   ((TimeNow - LastEventTime) >= (SqliteAddTime * 1000)))
  {
  LastEventTime = TimeNow;
  str = sqlite3_mprintf(SqlTableInsertQuery, d->OilTemp, d->OilLevel, d->OilPressure, d->WaterTemp, d->FuelTemp,
                  d->EngineTorque,d->Rpm,d->AirTemp,d->TurbochargerPressure,d->FuelConsumptionInL);
  if (sqlite3_exec(DB, str, NULL, 0, &errmsg) != SQLITE_OK)
    {
    sqlite3_free(errmsg);
    SqliteDBDestroy();
    }
  sqlite3_free(str);
  }
}

