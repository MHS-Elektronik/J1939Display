/*******************************************************************************
                         xml_database.c  -  description
                             -------------------
    begin             : 02.09.2017
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
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "util.h"
#include "can.h"
#include "setup.h"
#include "main.h"
#include "xml_database.h"

static const guint XMLFilesRefreshTime = 4000;  // 4. Sek 

static const gchar DashboardXMLFilename[] = {"dashboard.xml"};
static const gchar StatusXMLFilename[] = {"status.xml"};

static const gchar DashboardXML[] = {
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
  "<dashboard>\n"
	"  <Rpm>%5.0f</Rpm>\n"
  "  <EngineTorque>%4.0f</EngineTorque>\n"
  "  <FuelConsumptionInL>%4.0f</FuelConsumptionInL>\n"
  "  <WaterTemp>%5.1f</WaterTemp>\n"
  "  <OilTemp>%5.1f</OilTemp>\n"
  "  <OilLevel>%5.1f</OilLevel>\n"
  "  <OilPressure>%5.2f</OilPressure>\n"
  "  <FuelTemp>%5.1f</FuelTemp>\n"
  "  <AirTemp>%5.1f</AirTemp>\n"
  "  <TurbochargerPressure>%5.2f</TurbochargerPressure>\n"
  "</dashboard>"};

static const gchar StatusXML[] = {
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
  "<status>\n"
	"  <Rpm>%5.0f</Rpm>\n"
  "  <EngineTorque>%4.0f</EngineTorque>\n"
  "  <FuelConsumptionInL>%4.0f</FuelConsumptionInL>\n"
  "  <WaterTemp>%5.1f</WaterTemp>\n"
  "  <OilTemp>%5.1f</OilTemp>\n"
  "  <OilLevel>%5.1f</OilLevel>\n"
  "  <OilPressure>%5.2f</OilPressure>\n"
  "  <FuelTemp>%5.1f</FuelTemp>\n"
  "  <AirTemp>%5.1f</AirTemp>\n"
  "  <TurbochargerPressure>%5.2f</TurbochargerPressure>\n"
  "</status>"};


struct TXmlDb
  {
  guint64 LastEventTime;
  };

struct TXmlDb *XmlDb = NULL;


gint XMLDatabaseCreate(void)
{
if (!Setup.EnableXMLDatabase)
  return(0);
if (!(XmlDb = (struct TXmlDb *)g_malloc0(sizeof(struct TXmlDb))))
  return(-1);
return(0);
}


void XMLDatabaseDestroy(void)
{
safe_free(XmlDb);
}


void UpdateXMLDatabase(struct TJ1939Data *d)
{
FILE *file;
gchar *str, *s, *filename;
gchar c;
int size;


if (!XmlDb)
  return;
str = NULL;
if ((XmlDb->LastEventTime > TimeNow) ||
   ((TimeNow - XmlDb->LastEventTime) >= (XMLFilesRefreshTime * 1000)))
  {
  XmlDb->LastEventTime = TimeNow;
  // Status
  str = g_strdup_printf(StatusXML, d->Rpm, d->EngineTorque, d->FuelConsumptionInL, d->WaterTemp,
                        d->OilTemp, d->OilLevel, d->OilPressure, d->FuelTemp, d->AirTemp,
                        d->TurbochargerPressure);
  filename = g_build_filename(Setup.XMLDbPath, StatusXMLFilename, NULL);
  if ((file = fopen(filename, "wb")))
	  {
    size = strlen(str);
    (void)fwrite(str, 1, size, file);
    fclose(file);
    }
  safe_free(filename);  
  safe_free(str);
  // Dashboard
  str = g_strdup_printf(DashboardXML, d->Rpm, d->EngineTorque, d->FuelConsumptionInL, d->WaterTemp,
                        d->OilTemp, d->OilLevel, d->OilPressure, d->FuelTemp, d->AirTemp,
                        d->TurbochargerPressure);
  for (s = str; (c = *s); s++)
    {
    if (c == ',')
      *s = '.';
    }
  filename = g_build_filename(Setup.XMLDbPath, DashboardXMLFilename, NULL);
  if ((file = fopen(filename, "wb")))
	  {
    size = strlen(str);
    (void)fwrite(str, 1, size, file);
    fclose(file);
    }
  safe_free(filename);  
  safe_free(str);  
  }
}
