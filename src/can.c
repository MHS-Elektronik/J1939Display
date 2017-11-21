/*******************************************************************************
                            can.c  -  description
                             -------------------
    begin             : 28.02.2017
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
#include <math.h>
#include <gtk/gtk.h>
#include "util.h"
#include "can_drv.h"
#include "mhs_g_messages.h"
#include "mhs_msg_types.h"
#include "main.h"
#include "can.h"


/*************************************/
/*              SETUP                */
/*************************************/
#ifdef __WIN32__
  #deinfe DRIVER_FILE "mhstcan.dll"
  #define FULL_DRIVER_FILE NULL
#else
  #define DRIVER_FILE "libmhstcan.so"
  #define FULL_DRIVER_FILE "/opt/tiny_can/can_api/libmhstcan.so"
#endif
#define CAN_SPEED 500  // 500 kBit/s
#define DEVICE_SNR NULL  // "Snr=01000001"

                               //     Name |     ID     | PGN   | DLC | Int.  |
                               //          |            | Time  |     |       |
                               //----------+------------+-------+-----+-------+---------------------
#define J1939_ID1 0x18FEEE00   //   EET    | 0x18FEEE00 | 65262 |  8  | 1s    | Electronic Engine Temperature
#define J1939_ID2 0x18FEEF00   //   EFL/P  | 0x18FEEF00 | 65263 |  8  | 500ms | Engine Fluid Level/Pressure
#define J1939_ID3 0x0CF00400   //   EEC1   | 0x0CF00400 | 61444 |  8  | 10ms  | Electronic Engine Controller
#define J1939_ID4 0x18FEF600   //   IC1    | 0x18FEF600 | 56270 |  8  | 500ms | Inlet / Exhaust Conditions 1
#define J1939_ID5 0x18FEF200   //   LFE    | 0x18FEF200 | 65266 |  8  | 100ms | Fuel Economy (Liquid)

#define J1939_ID1_IDX  (1 | INDEX_SOFT_FLAG)
#define J1939_ID2_IDX  (2 | INDEX_SOFT_FLAG)
#define J1939_ID3_IDX  (3 | INDEX_SOFT_FLAG)
#define J1939_ID4_IDX  (4 | INDEX_SOFT_FLAG)
#define J1939_ID5_IDX  (5 | INDEX_SOFT_FLAG)


#define PNP_EVENT    0x00000001
#define SCAN_EVENT   0x00000002

static const uint32_t J1939IdTab[] = {J1939_ID1, J1939_ID2, J1939_ID3, J1939_ID4, J1939_ID5, 0xFFFFFFFF};
static const uint32_t J1939IdxTab[] = {J1939_ID1_IDX, J1939_ID2_IDX, J1939_ID3_IDX, J1939_ID4_IDX, J1939_ID5_IDX};

static const guint RxDMessageTimeout = 3000; // 3 Sek.


static guint64 LastRxDDataTime = 0;
static int32_t LastNumDevs = -1;


static gint FilterSetup(void)
{
uint32_t id;
gint res, i;
struct TMsgFilter msg_filter;

msg_filter.FilFlags = 0L;    // Alle Flags mit 0 Initialisieren
msg_filter.FilEFF = 1;       // Extended Frame Format
msg_filter.FilIdMode = 2;    // 2 = Single Id
msg_filter.FilMode = 1;      // 1 = Nachricht nicht enfernen
msg_filter.FilEnable = 1;    // Filter freigeben
for (i = 0; (id = J1939IdTab[i]) != 0xFFFFFFFF; i++)
  {
  msg_filter.Code = id;
  if ((res = (gint)CanSetFilter(J1939IdxTab[i], &msg_filter)) < 0)
    break;
  }
return(res);
}


gint J1939CanInit(struct TCanCore *can_core)
{
char *str;
gint err;

can_core->AppCanStatus = APP_CAN_INIT;
safe_free(can_core->DriverFileName);
can_core->DriverFileName = g_strdup(DRIVER_FILE);
safe_free(can_core->DriverInfoStr);
safe_free(can_core->LastErrorString);
// **** Treiber DLL laden
if ((err = (gint)LoadDriver(FULL_DRIVER_FILE)) < 0)
  can_core->LastErrorString = g_strdup_printf("LoadDriver Error-Code:%d", err);
// **** Treiber DLL initialisieren
else if ((err = (gint)CanExInitDriver("CanCallThread=0")) < 0)
  can_core->LastErrorString = g_strdup_printf("CanExInitDriver Error-Code:%d", err);
else if ((err = (gint)CanExCreateDevice(&can_core->DeviceIndex, NULL)) < 0)
  can_core->LastErrorString = g_strdup_printf("CanExCreateDevice Error-Code:%d", err);
else if ((err = CanExCreateFifo(0x80000001, 1000, NULL, 0, 0xFFFFFFFF)))
  can_core->LastErrorString = g_strdup_printf("CanExCreateFifo Error-Code:%d", err);
// Empfangs FIFO  erzeugen
else if ((err = CanExCreateFifo(0x80000000, 10000, NULL, 0, 0xFFFFFFFF)))
  can_core->LastErrorString = g_strdup_printf("CanExCreateFifo Error-Code:%d", err);
// **** Filter konfigurieren
else if ((err = FilterSetup()) < 0)
  can_core->LastErrorString = g_strdup_printf("FilterSetup Error-Code:%d", err);
if (err < 0)
  can_core->AppCanStatus = APP_CAN_DRIVER_ERROR;
else
  {
  if ((str = CanDrvInfo()))
    can_core->DriverInfoStr = g_strdup(str);
  can_core->AppCanStatus = APP_CAN_CLOSE;
  }
return(err);
}


void J1939CanDown(struct TCanCore *can_core)
{
if (can_core->AppCanStatus >= APP_CAN_OPEN)
  J1939Close(can_core);
J1939PnPStop(can_core);
// **** Treiber entladen
UnloadDriver();
}


gint J1939CanOpen(struct TCanCore *can_core)
{
gint err;

if ((can_core->AppCanStatus == APP_CAN_INIT) || (can_core->AppCanStatus == APP_CAN_DRIVER_ERROR))
  return(-1);
safe_free(can_core->LastErrorString);
if (can_core->AppCanStatus >= APP_CAN_OPEN)
  J1939Close(can_core);
// **** Schnittstelle PC <-> USB-Tiny öffnen
if ((err = (gint)CanDeviceOpen(can_core->DeviceIndex, DEVICE_SNR)) < 0)
  can_core->LastErrorString = g_strdup_printf("CanDeviceOpen Error-Code:%d", err);
else if ((err = (gint)CanExGetDeviceInfo(can_core->DeviceIndex, &can_core->DeviceInfo, NULL, NULL)) < 0)
  {
  can_core->LastErrorString = g_strdup_printf("CanExGetDeviceInfo Error-Code:%d", err);
  J1939Close(can_core);
  }
if (err >= 0)
  {
  // **** Übertragungsgeschwindigkeit einstellen
  CanSetSpeed(can_core->DeviceIndex, can_core->CanSpeed);
  // **** CAN Bus Start
  CanSetMode(can_core->DeviceIndex, OP_CAN_START_LOM, CAN_CMD_CLEAR_NO_SW_FILTER_CHANGE);
  }
if (err)
  can_core->AppCanStatus = APP_CAN_OPEN_ERROR;
else
  can_core->AppCanStatus = APP_CAN_RUN;
return(err);
}


void J1939Close(struct TCanCore *can_core)
{
if ((can_core->AppCanStatus == APP_CAN_INIT) || (can_core->AppCanStatus == APP_CAN_DRIVER_ERROR))
  return;
CanDeviceClose(can_core->DeviceIndex);
can_core->AppCanStatus = APP_CAN_CLOSE;
}


void J1939CanReadMessages(struct TCanCore *can_core, struct TJ1939Data *d)
{
gdouble value;
struct TCanMsg msg;
struct TDeviceStatus status;
guint hit;

hit = 0;
if (can_core->AppCanStatus < APP_CAN_RUN)
  return;

/*********************************/
/*  Status abfragen & auswerten  */
/*********************************/
CanGetDeviceStatus(can_core->DeviceIndex, &status);

if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
  {
  if (status.CanStatus == CAN_STATUS_BUS_OFF)
    CanSetMode(can_core->DeviceIndex, OP_CAN_RESET, CAN_CMD_NONE);
  }
else
  {
  can_core->AppCanStatus = APP_CAN_CLOSE;
  return;
  }

if (CanReceive(J1939_ID1_IDX, &msg, 1) > 0) // id = 0x18FEEE00
  {
  if (msg.MsgLen >= 4)
    {
    hit |= 0x0001;
    value = msg.MsgData[0];          // Wassertemperatur
    d->WaterTemp = value - 40;        // -40 - +215 °C

    value = msg.MsgData[1];          // Kraftstofftemperatur
    d->FuelTemp = value - 40;         // -40 - +215 °C

    value = ((msg.MsgData[3] << 8) | msg.MsgData[2]) ; // Öltemperatur
    d->OilTemp = (value * 0.03125) - 273;
    }
  }
if (CanReceive(J1939_ID2_IDX, &msg, 1) > 0) // id = 0x18FEEF00
  {
  if (msg.MsgLen >= 4)
    {
    hit |= 0x0002;
    value = msg.MsgData[2];  // Ölstand  [###.#]
    d->OilLevel = value * 0.4;

    value = msg.MsgData[3];  // Öldruck [##.##]
    d->OilPressure = value * 0.04;
    }
  }
if (CanReceive(J1939_ID3_IDX, &msg, 1) > 0) // id = 0x0CF00400
  {
  if (msg.MsgLen >= 5)
    {
    hit |= 0x0004;
    value = msg.MsgData[2];                 // Drehmoment
    d->EngineTorque = value - 125;
    value = ((msg.MsgData[4] << 8) | msg.MsgData[3]) ; // Drehzahl
    d->Rpm = value * 0.125;
    }
  }
if (CanReceive(J1939_ID4_IDX, &msg, 1) > 0) // id = 0x18FEF600
  {
  if (msg.MsgLen >= 3)
    {
    hit |= 0x0008;
    value = msg.MsgData[1];                 // Ladedruck
    d->TurbochargerPressure = value * 0.02;  // [##.##]

    value = msg.MsgData[2];                 // Ansauglufttemp (ladeluft)
    d->AirTemp = value - 40;                 // -40 - +215 °C
    }
  }
if (CanReceive(J1939_ID5_IDX, &msg, 1) > 0) // id = 0x18FEF200
  {
  if (msg.MsgLen >= 2)
    {
    hit |= 0x0010;
    value = ((msg.MsgData[1] << 8) | msg.MsgData[0]) ; // Verbrauch
    d->FuelConsumptionInL = value * 0.05;                  // in l/h [##.##]
    //d->FuelConsumptionInKg = d->FuelConsumptionInL * 0.92; // in kg [##.##] <*>
    }
  }
if (hit)
  {
  LastRxDDataTime = TimeNow;
  can_core->AppCanStatus = APP_CAN_RUN_RECEIVE;
  }
else
  {
  if ((TimeNow - LastRxDDataTime) >= (RxDMessageTimeout * 1000))
    can_core->AppCanStatus = APP_CAN_RUN;
  }
}


/********************/
/* PnP Event Thread */
/********************/
static gpointer PnPThreadExecute(gpointer data)
{
uint32_t event;
int32_t num_devs;
struct TCanCore *can_core;
TMhsGMessage *g_msg;

can_core = (struct TCanCore *)data;
do
  {
  event = CanExWaitForEvent(can_core->PnPEvent, 0);
  if (event & 0x80000000)        // Beenden Event, Thread Schleife verlassen
    break;
  else if (event & (PNP_EVENT | SCAN_EVENT))    // Pluy &  Play Event
    {
    can_core->DevicesListCount = 0;
    CanExDataFree((void **)&can_core->DevicesList);
    if ((num_devs = CanExGetDeviceList(&can_core->DevicesList, 0)) > 0)
      can_core->DevicesListCount = num_devs;
    if ((!(event & PNP_EVENT)) || (num_devs != LastNumDevs))
      {
      LastNumDevs = num_devs;
      g_msg = mhs_g_new_message(MHS_MSG_PNP_EVENT, &num_devs, sizeof(int32_t));
      mhs_g_message_post(can_core->Scheduler, g_msg);
      }
    }
  }
while (1);
return(NULL);
}


gint J1939PnPStart(struct TCanCore *can_core, TMhsGScheduler *scheduler)
{
LastNumDevs = -1;
can_core->Scheduler = scheduler;
can_core->PnPEvent = CanExCreateEvent();   // Event Objekt erstellen
CanExSetObjEvent(INDEX_INVALID, MHS_EVS_PNP, can_core->PnPEvent, PNP_EVENT);
// Thread erzeugen und starten
can_core->PnPThread = g_thread_new(NULL, PnPThreadExecute, can_core);
return(0);
}


void J1939PnPScan(struct TCanCore *can_core)
{
CanExSetEvent(can_core->PnPEvent, SCAN_EVENT);
}


void J1939PnPStop(struct TCanCore *can_core)
{
if (can_core->PnPThread)
  {
  // Terminate Event setzen
  CanExSetEvent(can_core->PnPEvent, MHS_TERMINATE);
  g_thread_join(can_core->PnPThread);
  can_core->PnPThread = NULL;
  }
}
