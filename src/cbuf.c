/*******************************************************************************
                            cpuf.c  -  description
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
#include <glib.h>
#include <string.h>
#include "can_types.h"
#include "util.h"
#include "cbuf.h"

/*   <*>
#if GLIB_CHECK_VERSION(2,31,0)
  cfc_data_cond = g_malloc(sizeof(GCond));
  g_cond_init(cfc_data_cond);
  cfc_data_mtx = g_malloc(sizeof(GMutex));
  g_mutex_init(cfc_data_mtx);
#else
  cfc_data_cond = g_cond_new();
  cfc_data_mtx = g_mutex_new();
#endif */

// #ifdef GTK3 <*>
  #define CBufLockI(l) g_mutex_lock(&(l)->Mutex)
  #define CBufUnlockI(l) g_mutex_unlock(&(l)->Mutex)

  #define CBufLock(l) do {if ((l)->Callback) g_mutex_lock(&(l)->Mutex);} while(0)
  #define CBufUnlock(l) do {if ((l)->Callback) g_mutex_unlock(&(l)->Mutex);} while(0)
/*#else
  #define CBufLockI(l) g_static_mutex_lock(&(l)->Mutex)
  #define CBufUnlockI(l) g_static_mutex_unlock(&(l)->Mutex)

  #define CBufLock(l) do {if ((l)->Callback) g_static_mutex_lock(&(l)->Mutex);} while(0)
  #define CBufUnlock(l) do {if ((l)->Callback) g_static_mutex_unlock(&(l)->Mutex);} while(0)
#endif*/

static gpointer ThreadExecute(gpointer data);

static guint g_event_add_full(TCanBuffer *cbuf, gint priority, CBufGEventCb function, gpointer data, GDestroyNotify notify);
static gint g_set_events(guint id, guint events);


/*
******************** CBufCreate ********************
*/
TCanBuffer *CBufCreate(guint reserved_size, gboolean override, guint event_delay,
     guint event_timeout, guint event_limit, CBufGEventCb callback, gpointer user_data)
{
TCanBuffer *cbuf;

if (!reserved_size)
  return(NULL);
if ((cbuf = (TCanBuffer *)g_malloc0(sizeof(TCanBuffer))))
  {
  if (!(cbuf->Data = g_malloc(sizeof(struct TCanMsg) * reserved_size)))
    {
    safe_free(cbuf);
    return(NULL);
    }
  cbuf->Pos = 0;
  cbuf->Len = reserved_size;
  cbuf->Override = override;
  cbuf->EventDelay = event_delay;
  cbuf->EventTimeout = event_timeout;
  cbuf->EventLimit = event_limit;
  //cbuf->LastAddTime = -1; <*>
  cbuf->EventCount = 0;
  cbuf->Events = 0;
  cbuf->Callback = callback;
  if (callback)
    {
//#ifdef GTK3
    g_mutex_init(&cbuf->Mutex);
/*#else
    g_static_mutex_init(&cbuf->Mutex);
#endif */
    g_cond_init(&cbuf->Cond);
    cbuf->EventId = g_event_add_full(cbuf, G_PRIORITY_DEFAULT, callback, user_data, NULL);
    // Thread erzeugen
/*#ifdef __WIN32__
    cbuf->Thread = g_thread_create(ThreadExecute, cbuf, TRUE, NULL);
#else */
    cbuf->Thread = g_thread_new(NULL, ThreadExecute, cbuf);
//#endif
    }
  }
return(cbuf);
}


/*
******************** CBufDestroy ********************
*/
void CBufDestroy(TCanBuffer **cbuf)
{
TCanBuffer *buffer;

if (!cbuf)
  return;
buffer = *cbuf;
if (buffer)
  {
  if (buffer->Callback)
    {
    CBufSetEvents(buffer, 0x80000000);
    g_thread_join(buffer->Thread);
//#ifdef GTK3 <*>
    g_mutex_clear(&buffer->Mutex);
//#endif
    g_cond_clear(&buffer->Cond);
    g_source_remove(buffer->EventId);
    }
  safe_free(buffer->Data);
  g_free(buffer);
  }
*cbuf = NULL;
}


gint CBufAddMsgs(TCanBuffer *cbuf, struct TCanMsg *msgs, gint size)
{
gint f;
guint len, pos;

if ((!cbuf) || (!size) || (!msgs))
  return(-1);
CBufLock(cbuf);
len = cbuf->Len;
pos = cbuf->Pos;
if (cbuf->Override)
  {
  for (; size; size--)
    {
    memcpy(&cbuf->Data[pos], msgs++, sizeof(struct TCanMsg) * size);
    if (++pos >= len)
      {
      pos = 0;
      cbuf->Looped = TRUE;
      }
    }
  cbuf->Pos = pos;
  }
else
  {
  f = len - pos;
  if (size > f)
    size = f;
  if (size)
    {
    memcpy(&cbuf->Data[pos], msgs, sizeof(struct TCanMsg) * size);
    cbuf->Pos += size;
    }
  }
if (size > 0)
  {
  cbuf->EventCount += size;
  cbuf->Events |= 0x00000001;
  if (cbuf->Callback)
    g_cond_signal(&cbuf->Cond);
  }
CBufUnlock(cbuf);
return(size);
}


gint CBufGetMsgByIndex(TCanBuffer *cbuf, struct TCanMsg *dest, guint index)
{
gint res;
guint len, pos;

if (!cbuf)
  return(-1);
CBufLock(cbuf);
res = 0;
len = cbuf->Len;
pos = cbuf->Pos;
if (cbuf->Override)
  {
  if (cbuf->Looped == FALSE)
    {
    if (index >= pos)
      res = -1;
    }
  else
    {
    if (index >= len)
      res = -1;
    else
      {
      index += pos;
      if (index >= len)
        index -= len;
      }
    }
  }
else
  {
  if (index >= pos)
    res = -1;
  }
if (res >= 0)
  memcpy(dest, &cbuf->Data[index], sizeof(struct TCanMsg));
CBufUnlock(cbuf);
return(res);
}


/*
******************** CBufDataClear ********************
*/
gint CBufDataClear(TCanBuffer *cbuf)
{
if (!cbuf)
  return(-1);
CBufLock(cbuf);
cbuf->Looped = FALSE;
cbuf->Pos = 0;
CBufUnlock(cbuf);
return(0);
}


/*
******************** CBufGetSize ********************
Funktion  : Maximale Puffergröße abfragen
*/
gint CBufGetSize(TCanBuffer *cbuf)
{
if (!cbuf)
  return(-1);
if (!cbuf->Data)
  return(-1);
if (cbuf->Override)
  {
  if (cbuf->Looped == FALSE)
    return((int)cbuf->Pos);
  else
    return((int)cbuf->Len);
  }
else
  return((int)cbuf->Pos);
}


void CBufSetEvents(TCanBuffer *cbuf, guint events)
{
if (!cbuf)
  return;
CBufLock(cbuf);
cbuf->Events |= events;
if (cbuf->Callback)
  g_cond_signal(&cbuf->Cond);
CBufUnlock(cbuf);
}


/****************/
/* Event Thread */
/****************/
static gpointer ThreadExecute(gpointer data)
{
TCanBuffer *cbuf;
gint hit;
guint event_limit;
gint64 time, end_time, event_timeout, event_delay;

cbuf = (TCanBuffer *)data;
event_timeout = (gint64)cbuf->EventTimeout * 1000;
event_delay = (gint64)cbuf->EventDelay * 1000;
event_limit = cbuf->EventLimit;
end_time = -1;
do
  {
  CBufLockI(cbuf);
  if (end_time == -1)
    g_cond_wait (&cbuf->Cond, &cbuf->Mutex);
  else
    (void)g_cond_wait_until(&cbuf->Cond, &cbuf->Mutex, end_time);
  end_time = -1;
  if (!(cbuf->Events & 0x80000000))
    {
    hit = 0;
    time = g_get_monotonic_time();
    if ((event_limit) && (cbuf->EventCount >= event_limit))
      hit = 1;
    else if ((cbuf->EventCount) && (time >= (cbuf->EventTime + event_timeout)))
      hit = 1;
    else if (cbuf->Events & 0x00000001)
      end_time = time + event_delay;
    else if (cbuf->EventCount)
      hit = 1;
    if (hit)
      {
      cbuf->EventCount = 0;
      cbuf->EventTime = time;
      // Event Callback triggern
      if (g_set_events(cbuf->EventId, 1))  // <*> events ?
        {
        CBufUnlockI(cbuf);
        break;
        }
      }
    cbuf->Events = 0;
    }
  CBufUnlockI(cbuf);
  }
while (!(cbuf->Events & 0x80000000));
return(NULL);
}


/******************/
/* Event Callback */
/******************/
typedef struct _CBufGEventSource CBufGEventSource;

struct _CBufGEventSource
  {
  GSource source;
  volatile guint events;
  TCanBuffer *cbuf;
  };

static gboolean g_event_prepare(GSource *source, gint *timeout);
static gboolean g_event_check(GSource *source);
static gboolean g_event_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);

GSourceFuncs cbuf_event_funcs =
  {
  g_event_prepare,
  g_event_check,
  g_event_dispatch,
  NULL,
  NULL,
  NULL
  };


static gboolean g_event_prepare(GSource *source, gint *timeout)
{
CBufGEventSource *event_source;

event_source = (CBufGEventSource *)source;
*timeout = -1;
if (event_source->events)
  return(TRUE);
else
  return(FALSE);
}


static gboolean g_event_check(GSource *source)
{
CBufGEventSource *event_source;

event_source = (CBufGEventSource *)source;
if (event_source->events)
  return(TRUE);
else
  return(FALSE);
}


static gboolean g_event_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
CBufGEventSource *event_source;
CBufGEventCb func;
guint events;

event_source = (CBufGEventSource *)source;
func = (CBufGEventCb)callback;
if ((events = g_atomic_int_and(&event_source->events, 0)))
  {
  if (events)
    return((*func)(event_source->cbuf, events, user_data));
  else
    return(TRUE);
  }
else
  return(TRUE);
}


static guint g_event_add_full(TCanBuffer *cbuf, gint priority, CBufGEventCb function, gpointer data, GDestroyNotify notify)
{
GSource *source;
guint id;
source = g_source_new(&cbuf_event_funcs, sizeof(CBufGEventSource));
((CBufGEventSource *)source)->cbuf = cbuf;
if (priority != G_PRIORITY_DEFAULT)
  g_source_set_priority (source, priority);

g_source_set_callback(source, (GSourceFunc)function, data, notify);
id = g_source_attach(source, NULL);
g_source_unref(source);

return(id);
}


static gint g_set_events(guint id, guint events)
{
CBufGEventSource *event_source;

if (!id)
  return(-1);
event_source = (CBufGEventSource *)g_main_context_find_source_by_id(NULL, id);
if (!event_source)
  return(-1);
(void)g_atomic_int_or(&event_source->events, events);
g_main_context_wakeup(NULL);
return(0);
}
