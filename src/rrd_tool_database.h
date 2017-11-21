#ifndef __RRD_TOOL_DATABASE_H__
#define __RRD_TOOL_DATABASE_H__

#include <glib.h>
#include "can.h"

#ifdef __cplusplus
  extern "C" {
#endif

gint RRDtoolCreate(void);
void RRDtoolDestroy(void);
void UpdateRRDtoolDatabase(struct TJ1939Data *d);

#ifdef __cplusplus
  }
#endif

#endif