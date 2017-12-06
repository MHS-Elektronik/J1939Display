#ifndef __CAN_H__
#define __CAN_H__

#include <glib.h>
#include "can_drv.h"
#include "mhs_g_messages.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define APP_CAN_INIT          0
#define APP_CAN_CLOSE         1

#define APP_CAN_OPEN          2
#define APP_CAN_RUN           3
#define APP_CAN_RUN_RECEIVE   4

#define APP_CAN_DRIVER_ERROR  -1
#define APP_CAN_OPEN_ERROR    -2

#define CAN_CMD_CLEAR_NO_SW_FILTER_CHANGE  (CAN_CMD_FIFOS_CLEAR | CAN_CMD_HW_FILTER_CLEAR | CAN_CMD_TXD_PUFFERS_CLEAR) 

#define INDEX_TRACE_FIFO    0x80000000
#define INDEX_AUTOBAUD_FIFO 0x80000001

struct TCanCore
  {
  gchar *DriverFileName;
  gchar *DriverInfoStr;
  gchar *LastErrorString;
  uint32_t DeviceIndex;
  uint16_t CanSpeed;
  gint AppCanStatus;
  struct TCanDeviceInfo DeviceInfo;
  // PnP
  TMhsGScheduler *Scheduler;
  GThread *PnPThread;
  TMhsEvent *PnPEvent;
  int32_t DevicesListCount;
  struct TCanDevicesList *DevicesList;
  };


struct TJ1939Data
  {
  gdouble OilTemp;
  gdouble OilLevel;
  gdouble OilPressure;
  gdouble WaterTemp;
  gdouble FuelTemp;
  gdouble EngineTorque;
  gdouble Rpm;
  gdouble AirTemp;
  gdouble TurbochargerPressure;
  gdouble FuelConsumptionInL;
  gdouble FuelConsumptionInKg;
  gdouble OutputPower;
  };



gint J1939CanInit(struct TCanCore *can_core);
void J1939CanDown(struct TCanCore *can_core);
gint J1939CanOpen(struct TCanCore *can_core);
void J1939CanClose(struct TCanCore *can_core);
void J1939CanReadMessages(struct TCanCore *can_core, struct TJ1939Data *d);

void J1939PnPStop(struct TCanCore *can_core);
gint J1939PnPStart(struct TCanCore *can_core, TMhsGScheduler *scheduler);
void J1939PnPScan(struct TCanCore *can_core);

#ifdef __cplusplus
  }
#endif

#endif
