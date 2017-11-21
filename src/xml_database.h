#ifndef __XML_DATABASE_H__
#define __XML_DATABASE_H__

#include <glib.h>
#include "can.h"

#ifdef __cplusplus
  extern "C" {
#endif

gint XMLDatabaseCreate(void);
void XMLDatabaseDestroy(void);
void UpdateXMLDatabase(struct TJ1939Data *d);

#ifdef __cplusplus
  }
#endif

#endif
