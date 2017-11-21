#ifndef __MODBUS_IO_H__
#define __MODBUS_IO_H__

#include <glib.h>
#include "can.h"

#ifdef __cplusplus
  extern "C" {
#endif

gint ModbusIoCreate(void);
void ModbusIoDestroy(void);
void UpdateModbusIo(struct TJ1939Data *d);

#ifdef __cplusplus
  }
#endif

#endif
