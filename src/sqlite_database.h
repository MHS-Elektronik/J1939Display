#ifndef __SQLITE_DATABASE_H__
#define __SQLITE_DATABASE_H__

#include <glib.h>
#include "can.h"

#ifdef __cplusplus
  extern "C" {
#endif

gint SqliteDBCreate(void);
void SqliteDBDestroy(void);
void UpdateSqliteDatabase(struct TJ1939Data *d);

#ifdef __cplusplus
  }
#endif

#endif



