/*******************************************************************************
                          modbus_io.c  -  description
                             -------------------
    begin             : 05.09.2017
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
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <modbus.h>
#include "util.h"
#include "can.h"
#include "main.h"
#include "setup.h"
#include "mhs_file_event.h"
#include "modbus_io.h"

#define NB_CONNECTION  5
#define max(a, b) ((a)>(b)) ? (a) : (b)

static struct TModbusIo *ModbusIo = NULL;


typedef struct _TModClient TModClient;

struct _TModClient
  {
  TModClient *Next;
  struct sockaddr_in ClientAddr;
  int Fd;
  };

struct TModValues
  {             
  uint16_t OilTemp;
  uint16_t OilLevel;
  uint16_t OilPressure;
  uint16_t WaterTemp;
  uint16_t FuelTemp;
  uint16_t EngineTorque;
  uint16_t Rpm;
  uint16_t AirTemp;
  uint16_t TurbochargerPressure;
  uint16_t FuelConsumptionInL;
  uint16_t OutputPower;
  };

struct TModbusIo
  {
  int ServerSocket;
  fd_set Refset;
  fd_set Rdset;
  int FdMax;
  pthread_t Thread;
  GMutex Mutex;
  volatile int32_t Run;
  struct TFileEvent *FileEvent;
  TModClient *Clients;
  struct TModValues ModValues;
  modbus_t *Modbus;
  modbus_mapping_t *Mapping;
  uint8_t Query[MODBUS_TCP_MAX_ADU_LENGTH];
  };


#define ModbusIoLock(l) g_mutex_lock(&(l)->Mutex)
#define ModbusIoUnlock(l) g_mutex_unlock(&(l)->Mutex)


/***************************************************************/
/* time in ms                                                  */
/***************************************************************/
static void mhs_calc_abs_timeout(struct timespec *timeout, uint32_t time)
{
struct timespec now;
#ifdef __APPLE__
struct timeval tv;
#endif
uint32_t rem;

#ifdef __APPLE__
if (gettimeofday(&tv, NULL) < 0)
  return;
TIMEVAL_TO_TIMESPEC(&tv, &now);
#else
clock_gettime(CLOCK_REALTIME, &now);
#endif
timeout->tv_sec = now.tv_sec + (time / 1000);
rem = ((time % 1000) * 1000000);
if ((now.tv_nsec + rem) >= 1000000000)
  {
  timeout->tv_sec++; /* carry bit stored in tv_sec */
  timeout->tv_nsec = (now.tv_nsec + rem) - 1000000000;
  }
else
  timeout->tv_nsec = now.tv_nsec + rem;
}

/***************************************************************************/
/*                       Ein Object löschen                                */
/***************************************************************************/
static TModClient *CreateClient(struct TModbusIo *modbus_io)
{
TModClient *list, *new;

if (!(new = (TModClient *)g_malloc0(sizeof(TModClient))))
  return(NULL);
list = modbus_io->Clients;
if (!list)
  modbus_io->Clients = new;
else
  {
  while (list->Next)
    list = list->Next;
  list->Next = new;
  }
return(new);
}


static void DestroyClient(struct TModbusIo *modbus_io, int fd)
{
TModClient *list, *prev;

prev = NULL;
// Liste nach "obj" durchsuchen
for (list = modbus_io->Clients; list; list = list->Next)
  {
  if (list->Fd == fd)
    {
    if (prev)
      prev->Next = list->Next;
    else
      modbus_io->Clients = list->Next;
    g_free(list);
    break;
    }
  prev = list;
  }
}


static int OpenNewModbusConn(struct TModbusIo *modbus_io)
{
socklen_t addrlen;
TModClient *client;

if (!(client = CreateClient(modbus_io)))
  return(-1);
/* Handle new connections */
addrlen = sizeof(struct sockaddr_in);
client->Fd = accept(modbus_io->ServerSocket, (struct sockaddr *)&client->ClientAddr, &addrlen);
if (client->Fd == -1)
  {
  DestroyClient(modbus_io, -1);
  return(-1);
  }
else
  {
  FD_SET(client->Fd, &modbus_io->Refset);
  if (client->Fd > modbus_io->FdMax)
    /* Keep track of the maximum */
    modbus_io->FdMax = client->Fd;

  /*printf("New connection from %s:%d on socket %d\n",
           inet_ntoa(client->ClientAddr.sin_addr), client->ClientAddr.sin_port, newfd);*/
  }
return(0);
}


static void ModbusIoFree(struct TModbusIo *modbus_io)
{
if (modbus_io->ServerSocket != -1)
  close(modbus_io->ServerSocket);
if (modbus_io->Modbus)
  modbus_free(modbus_io->Modbus);
if (modbus_io->Mapping)
  modbus_mapping_free(modbus_io->Mapping);
safe_free(modbus_io);
}


static void ModbusIoUpdateMapping(struct TModbusIo *modbus_io)
{
guint adr;

adr = 0;
ModbusIoLock(modbus_io);
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.OilTemp;
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.OilLevel;
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.OilPressure;
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.WaterTemp;
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.FuelTemp;
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.EngineTorque;
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.Rpm;
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.AirTemp;
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.TurbochargerPressure;
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.FuelConsumptionInL;
modbus_io->Mapping->tab_input_registers[adr++] = modbus_io->ModValues.OutputPower;
ModbusIoUnlock(modbus_io);
}


static void *thread_execute(void *data)
{
int rc, master_socket, event_fd;
struct TModbusIo *modbus_io;

// **** Thread Initialisieren
(void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
(void)pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

modbus_io = (struct TModbusIo *)data;
/* Clear the reference set of socket */
FD_ZERO(&modbus_io->Refset);
/* Add the server socket */
FD_SET(modbus_io->ServerSocket, &modbus_io->Refset);
/* Keep track of the max file descriptor */
event_fd = file_event_get_fd(modbus_io->FileEvent);
modbus_io->FdMax = max(event_fd, modbus_io->ServerSocket);

while (modbus_io->Run)
  {
  modbus_io->Rdset = modbus_io->Refset;
  if (select(modbus_io->FdMax+1, &modbus_io->Rdset, NULL, NULL, NULL) == -1)
    break;
  if (FD_ISSET(event_fd, &modbus_io->Rdset))
    //get_file_event(modbus_io->FileEvent)
    break;
  if (!modbus_io->Run)
    break;
  for (master_socket = 0; master_socket <= modbus_io->FdMax; master_socket++)
    {
    if (!FD_ISSET(master_socket, &modbus_io->Rdset))
      continue;

    if (master_socket == modbus_io->ServerSocket)
      {
      /* A client is asking a new connection */
      if (OpenNewModbusConn(modbus_io) < 0)
        continue;
      }
    else
      {
      modbus_set_socket(modbus_io->Modbus, master_socket);
      rc = modbus_receive(modbus_io->Modbus, modbus_io->Query);
      if (rc > 0)
        {
        ModbusIoUpdateMapping(modbus_io);
        modbus_reply(modbus_io->Modbus, modbus_io->Query, rc, modbus_io->Mapping);
        }
      else if (rc == -1)
        {
         /* This example server in ended on connection closing or
         * any errors. */
         //printf("Connection closed on socket %d\n", master_socket);
        close(master_socket);
        /* Remove from reference set */
        FD_CLR(master_socket, &modbus_io->Refset);
        if (master_socket == modbus_io->FdMax)
          modbus_io->FdMax--;
        DestroyClient(modbus_io, master_socket);
        }
      }
    }
  }
return(NULL);
}


gint ModbusIoCreate(void)
{
gint res;
struct TModbusIo *modbus_io;

if (!Setup.EnableModbus)
  return(0);
if (!(modbus_io = (struct TModbusIo *)g_malloc0(sizeof(struct TModbusIo))))
  return(-1);
g_mutex_init(&modbus_io->Mutex);  
res = 0;
modbus_io->ServerSocket = -1;
modbus_io->Thread = (pthread_t)-1;
ModbusIo = modbus_io;
if (!(modbus_io->Modbus = modbus_new_tcp("127.0.0.1", Setup.ModbusPort)))
  res = -1;
else
  {
  if (!(modbus_io->Mapping = modbus_mapping_new(0, 0, 10, 10)))
    {
    //fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
    res = -1;
    }
  else
    {
    if ((modbus_io->ServerSocket = modbus_tcp_listen(modbus_io->Modbus, NB_CONNECTION)) == -1)
      {
      //fprintf(stderr, "Unable to listen TCP connection\n");
      res = -1;
      }
    }
  }
if (!res)
  {
  if (!(modbus_io->FileEvent = create_file_event()))
    res = -1;
  }
if (!res)
  {
  modbus_io->Run = 1;
  //signal(SIGINT, close_sigint);
  if (pthread_create(&modbus_io->Thread, NULL, thread_execute, (void *)modbus_io))
    {
    modbus_io->Thread = (pthread_t)-1;
    modbus_io->Run = 0;
    res = -1;
    }
  }
if (res)
  ModbusIoDestroy();
  //ModbusIoFree(modbus_io);
return(0);
}


void ModbusIoDestroy(void)
{
struct timespec tabs;
TModClient *client, *next;

if (!ModbusIo)
  return;
if (ModbusIo->Thread != -1)
  {
  ModbusIo->Run = 0;
  (void)set_file_event(ModbusIo->FileEvent, 0xFF);
  mhs_calc_abs_timeout(&tabs, 1000);
  if (pthread_timedjoin_np(ModbusIo->Thread, NULL, &tabs))
    {
    if (pthread_cancel(ModbusIo->Thread) != ESRCH)
      (void)pthread_join(ModbusIo->Thread, NULL);
    }
  destroy_file_event(&ModbusIo->FileEvent);
  ModbusIo->Thread = (pthread_t)-1;
  }
for (client = ModbusIo->Clients; client; client = client->Next)
  close(client->Fd);

client = ModbusIo->Clients;
while (client)
  {
  next = client->Next;
  g_free(client);
  client = next;
  }
g_mutex_clear(&ModbusIo->Mutex);
ModbusIoFree(ModbusIo);
ModbusIo = NULL;
}


void UpdateModbusIo(struct TJ1939Data *d)
{
if (!ModbusIo)
  return;
ModbusIoLock(ModbusIo);
ModbusIo->ModValues.OilTemp = d->OilTemp;            
ModbusIo->ModValues.OilLevel = d->OilLevel;           
ModbusIo->ModValues.OilPressure = d->OilPressure;        
ModbusIo->ModValues.WaterTemp = d->WaterTemp;          
ModbusIo->ModValues.FuelTemp = d->FuelTemp;           
ModbusIo->ModValues.EngineTorque = d->EngineTorque;       
ModbusIo->ModValues.Rpm = d->Rpm;                
ModbusIo->ModValues.AirTemp = d->AirTemp;            
ModbusIo->ModValues.TurbochargerPressure = d->TurbochargerPressure;
ModbusIo->ModValues.FuelConsumptionInL = d->FuelConsumptionInL; 
ModbusIo->ModValues.OutputPower = d->OutputPower;         
ModbusIoUnlock(ModbusIo);
}
