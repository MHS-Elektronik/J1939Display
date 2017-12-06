/*******************************************************************************
                        can_autobaud.c  -  description
                             -------------------
    begin             : 09.09.2017
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
#include <glib.h>
#include <gtk/gtk.h>
#include "can_drv.h"
#include "can.h"
#include "util.h"
#include "mhs_g_messages.h"
#include "mhs_msg_types.h"
#include "can_autobaud.h"


struct TCanSpeedsTab
  {
  uint16_t CanSpeed;
  const gchar *CanSpeedStr;
  };


#define TestWaitTime 5000  // Wartezeit auf CAN Nachrichten in ms
#define ERROR_CNT_LIMIT 10
#define HIT_CNT_LIMIT   10

#define UsedCanSpeedsTabSize 9
// Tabelle mit CAN Übertragungsgeschwindigkeiten
static const struct TCanSpeedsTab UsedCanSpeedsTab[UsedCanSpeedsTabSize] = {
    {CAN_10K_BIT,  "10 kBit/s"},    // 10 kBit/s
    {CAN_20K_BIT,  "20 kBit/s"},    // 20 kBit/s
    {CAN_50K_BIT,  "50 kBit/s"},    // 50 kBit/s
    {CAN_100K_BIT, "100 kBit/s"},   // 100 kBit/s
    {CAN_125K_BIT, "125 kBit/s"},   // 125 kBit/s
    {CAN_250K_BIT, "250 kBit/s"},   // 250 kBit/s
    {CAN_500K_BIT, "500 kBit/s"},   // 500 kBit/s
    {CAN_800K_BIT, "800 kBit/s"},   // 800 kBit/s
    {CAN_1M_BIT,   "1 MBit/s"}};    // 1 MBit/s


#define RX_EVENT     0x00000001
#define STATUS_EVENT 0x00000002


static gpointer ThreadExecute(gpointer data);


struct TCanAutobaud *CreateCanAutobaud(TMhsGScheduler *scheduler, struct TCanCore *can_core)
{
struct TCanAutobaud *autobaud;

if (!(autobaud = (struct TCanAutobaud *)g_malloc0(sizeof(struct TCanAutobaud))))
  return(NULL);
autobaud->CanCore = can_core;
(void)CanExSetAsUByte(can_core->DeviceIndex, "CanErrorMsgsEnable", 1);
// Empfangs FIFO  erzeugen
//(void)CanExCreateFifo(INDEX_AUTOBAUD_FIFO, 1000, NULL, 0, 0xFFFFFFFF);
autobaud->Event = CanExCreateEvent();   // Event Objekt erstellen
// Events mit API Ereignissen verknüfpen
CanExSetObjEvent(can_core->DeviceIndex, MHS_EVS_STATUS, autobaud->Event, STATUS_EVENT);
CanExSetObjEvent(INDEX_AUTOBAUD_FIFO, MHS_EVS_OBJECT, autobaud->Event, RX_EVENT);
autobaud->Scheduler = scheduler;
return(autobaud);
}


void DestroyCanAutobaud(struct TCanAutobaud **autobaud_ref)
{
struct TCanAutobaud *autobaud;

if (!(autobaud = *autobaud_ref))
  return;
StopCanAutobaud(autobaud);
g_free(autobaud);
*autobaud_ref = NULL;
}


void StartCanAutobaud(struct TCanAutobaud *autobaud, uint16_t start_can_speed)
{
uint32_t idx;

StopCanAutobaud(autobaud);
autobaud->AktivTabIndex = 0;
for (idx = 0; idx < UsedCanSpeedsTabSize; idx++)
  {
  if (UsedCanSpeedsTab[idx].CanSpeed == start_can_speed)
    {
    autobaud->AktivTabIndex = idx;
    break;
    }
  }
// Terminate Event setzen
CanExResetEvent(autobaud->Event, 0x10); //MHS_TERMINATE);
// Thread erzeugen und starten
autobaud->Thread = g_thread_new(NULL, ThreadExecute, autobaud);
}


void StopCanAutobaud(struct TCanAutobaud *autobaud)
{
if (autobaud->Thread)
  {
  // Terminate Event setzen
  CanExSetEvent(autobaud->Event, 0x10); //MHS_TERMINATE);
  g_thread_join(autobaud->Thread);
  autobaud->Thread = NULL;
  }
}


/****************/
/* Event Thread */
/****************/
static gpointer ThreadExecute(gpointer data)
{
struct TCanAutobaud *autobaud;
struct TCanCore *can_core;
struct TCanMsg *msg;
TMhsGMessage *g_msg;
struct TDeviceStatus status;
uint16_t can_speed;
uint32_t event;
int32_t size, hit_cnt, error_cnt, first, setup, ok;
gchar *str;

autobaud = (struct TCanAutobaud *)data;
can_core = autobaud->CanCore;
first = 1;
setup = 1;
ok = 0;
str = NULL;
do
  {
  if (setup)
    {
    setup = 0;
    // **** Übertragungsgeschwindigkeit einstellen
    can_speed = UsedCanSpeedsTab[autobaud->AktivTabIndex].CanSpeed;
    CanSetSpeed(can_core->DeviceIndex, can_speed);
    // **** CAN Bus Start
    if (first)
      {
      first = 0;
      CanSetMode(can_core->DeviceIndex, OP_CAN_START_LOM, CAN_CMD_CLEAR_NO_SW_FILTER_CHANGE);
      }
    else
      CanSetMode(can_core->DeviceIndex, OP_CAN_NO_CHANGE, CAN_CMD_CLEAR_NO_SW_FILTER_CHANGE);
    CanReceiveClear(INDEX_AUTOBAUD_FIFO);
    hit_cnt = 0;
    error_cnt = 0;
    str = g_strdup_printf("[%u/%u] Baudrate %s testen, warte auf CAN Nachrichten ...",
         autobaud->AktivTabIndex+1, UsedCanSpeedsTabSize, UsedCanSpeedsTab[autobaud->AktivTabIndex].CanSpeedStr);
    g_msg = mhs_g_new_message_from_string(MHS_MSG_AUTOBAUD_START, str);
    mhs_g_message_post(autobaud->Scheduler, g_msg);
    safe_free(str);
    }

  event = CanExWaitForEvent(autobaud->Event, TestWaitTime);
  if (event & 0x10) //0x80000000)        // Beenden Event, Thread Schleife verlassen
    break;
  else if (event & STATUS_EVENT)
    {
    /*********************************/
    /*  Status abfragen & auswerten  */
    /*********************************/
    CanGetDeviceStatus(autobaud->CanCore->DeviceIndex, &status);

    if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
      {
      if (status.CanStatus == CAN_STATUS_BUS_OFF)
        CanSetMode(autobaud->CanCore->DeviceIndex, OP_CAN_RESET, CAN_CMD_NONE);
      }
    else
      {
      g_msg = mhs_g_new_message_from_string(MHS_MSG_AUTOBAUD_ERROR, "CAN Device nicht geöffnet");
      mhs_g_message_post(autobaud->Scheduler, g_msg);
      break;  // Thread beenden
      }
    }
  else if (event & RX_EVENT)     // CAN Rx Event
    {
    while ((size = CanReceive(INDEX_AUTOBAUD_FIFO, autobaud->RxTempBuffer, RX_TEMP_BUFFER_SIZE)) > 0)
      {
      for (msg = autobaud->RxTempBuffer; size; size--)
        {
        if (msg->MsgErr)
          error_cnt++;
        else
          hit_cnt++;
        msg++;
        }
      if (error_cnt >= ERROR_CNT_LIMIT)
        {
        setup = 1;
        break;
        }
      else if (hit_cnt >= HIT_CNT_LIMIT)
        {
        ok = 1;
        break;
        }
      }
    if (ok)
      {
      str = g_strdup_printf("[%u/%u] Baudrate %s erfolgreich dedektiert, %d CAN Nachrichten erfolgreich empfangen",
           autobaud->AktivTabIndex+1, UsedCanSpeedsTabSize, UsedCanSpeedsTab[autobaud->AktivTabIndex].CanSpeedStr, hit_cnt);
      can_core->CanSpeed = can_speed;
      g_msg = mhs_g_new_message_from_string(MHS_MSG_AUTOBAUD_OK, str);
      mhs_g_message_post(autobaud->Scheduler, g_msg);
      safe_free(str);
      break;   // Thread beenden
      }
    else
      {
      // <*> noch ändern nur das Tiny-CAN IV-XL kann Busfehler erkennen
      str = g_strdup_printf("[%u/%u] Baudrate %s testen, %d CAN Nachrichten erfolgreich empfangen, %d Busfehler erkannt",
           autobaud->AktivTabIndex+1, UsedCanSpeedsTabSize, UsedCanSpeedsTab[autobaud->AktivTabIndex].CanSpeedStr, hit_cnt, error_cnt);
      g_msg = mhs_g_new_message_from_string(MHS_MSG_AUTOBAUD_RUN, str);
      mhs_g_message_post(autobaud->Scheduler, g_msg);
      safe_free(str);
      }
    }
  else if (!event) // Timeout
    setup = 1;

  if (setup)
    {
    if (++autobaud->AktivTabIndex >= UsedCanSpeedsTabSize)
      autobaud->AktivTabIndex = 0;
    }
  }
while (1);
return(NULL);
}
