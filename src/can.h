#ifndef __CAN_H__
#define __CAN_H__

#include <glib.h>

#ifdef __cplusplus
  extern "C" {
#endif

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
  
gint J1939CanInit(void);
void J1939CanDown(void);
void J1939CanReadMessages(struct TJ1939Data *d);  

#ifdef __cplusplus
  }
#endif

#endif