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
#include "can_drv.h"
#include "can.h"


/*************************************/
/*              SETUP                */
/*************************************/
#ifdef __WIN32__
  #define DRIVER_FILE NULL
#else
  #define DRIVER_FILE "/opt/tiny_can/can_api/libmhstcan.so"
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



static const uint32_t J1939IdTab[] = {J1939_ID1, J1939_ID2, J1939_ID3, J1939_ID4, J1939_ID5, 0xFFFFFFFF};
static const uint32_t J1939IdxTab[] = {J1939_ID1_IDX, J1939_ID2_IDX, J1939_ID3_IDX, J1939_ID4_IDX, J1939_ID5_IDX};


static uint32_t DeviceIndex = INDEX_INVALID;


static gint FilterSetup(uint32_t device_index)
{
(void)device_index;
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


gint J1939CanInit(void)
{
gint err;

// **** Treiber DLL laden
if ((err = (gint)LoadDriver(DRIVER_FILE)) < 0)
  return(err);
// **** Treiber DLL initialisieren
if ((err = (gint)CanExInitDriver("CanCallThread=0")) < 0)
  return(err);
if ((err = (gint)CanExCreateDevice(&DeviceIndex, NULL)))
  return(err);
// Empfangs FIFO  erzeugen
if ((err = CanExCreateFifo(0x80000000, 10000, NULL, 0, 0xFFFFFFFF)))
  return(err);
// **** Schnittstelle PC <-> USB-Tiny öffnen
// COM Port 1 auswählen
if ((err = (gint)CanDeviceOpen(DeviceIndex, DEVICE_SNR)) < 0) 
  return(err);
// **** CAN Bus Start
CanSetMode(DeviceIndex, OP_CAN_STOP, CAN_CMD_ALL_CLEAR);
// **** Filter konfigurieren
if ((err = FilterSetup(DeviceIndex)) < 0)
  return(err);
// **** Übertragungsgeschwindigkeit auf 125kBit/s einstellen
CanSetSpeed(DeviceIndex, CAN_SPEED);
// **** CAN Bus Start
CanSetMode(DeviceIndex, OP_CAN_START, 0);
return(0);
}


void J1939CanDown(void)
{
// **** DLL entladen
UnloadDriver();
}


void J1939CanReadMessages(struct TJ1939Data *d)
{
gdouble value;
struct TCanMsg msg;

if (CanReceive(J1939_ID1_IDX, &msg, 1) > 0) // id = 0x18FEEE00
  {
  if (msg.MsgLen >= 4)
    {
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
    value = ((msg.MsgData[1] << 8) | msg.MsgData[0]) ; // Verbrauch
    d->FuelConsumptionInL = value * 0.05;                  // in l/h [##.##]
    //d->FuelConsumptionInKg = d->FuelConsumptionInL * 0.92; // in kg [##.##] <*>
    }
  }
}
