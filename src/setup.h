#ifndef __SETUP_H__
#define __SETUP_H__

struct TSetup
  {
  int ShowFullscreen;
  int EnableXMLDatabase;
  gchar *XMLDbPath;
  int EnableModbus;
  int ModbusPort;
  int EnableRRDtool;
  gchar *RRDtoolDbPath;
  int EnableSqlite;
  gchar *SqliteDbPath;
  };
  
extern struct TSetup Setup;
  
gint LoadConfigFile(void);
void ConfigDestroy(void);

#endif
