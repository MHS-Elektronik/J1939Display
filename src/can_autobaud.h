#ifndef __CAN_AUTOBAUD_H__
#define __CAN_AUTOBAUD_H__

#include <glib.h>
#include "can.h"
#include "mhs_g_messages.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define RX_TEMP_BUFFER_SIZE 255

struct TCanAutobaud
  {
  struct TCanCore *CanCore;
  uint32_t AktivTabIndex;
  TMhsGScheduler *Scheduler;
  GThread *Thread;
  TMhsEvent *Event;
  struct TCanMsg RxTempBuffer[RX_TEMP_BUFFER_SIZE];
  };

struct TCanAutobaud *CreateCanAutobaud(TMhsGScheduler *scheduler, struct TCanCore *can_core);
void DestroyCanAutobaud(struct TCanAutobaud **autobaud_ref);
void StartCanAutobaud(struct TCanAutobaud *autobaud, uint16_t start_can_speed);
void StopCanAutobaud(struct TCanAutobaud *autobaud);

#ifdef __cplusplus
  }
#endif

#endif

