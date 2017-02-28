#ifndef __UTIL_H__
#define __UTIL_H__

#include <glib.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
  extern "C" {
#endif

#define GetByte(x) *((uint8_t *)(x))
#define GetWord(x) *((uint16_t *)(x))
#define GetLong(x) *((uint32_t *)(x))

#define SetByte(x, v) *((uint8_t *)x) = (v)
#define SetWord(x, v) *((uint16_t *)x) = (v)
#define SetLong(x, v) *((uint32_t *)x) = (v)

#define lo(x)  (unsigned char)(x & 0xFF)
#define hi(x)  (unsigned char)((x >> 8) & 0xFF)

#define l_lo(x) (unsigned char)(x)
#define l_m1(x) (unsigned char)((x) >> 8)
#define l_m2(x) (unsigned char)((x) >> 16)
#define l_hi(x) (unsigned char)((x) >> 24)

#include <stdio.h>

#define safe_free(d) do { \
  if ((d)) \
    { \
    g_free((d)); \
    (d) = NULL; \
    } \
  } while(0)



#ifdef __WIN32__
// ****** Windows
#include <windows.h>
#define CALLBACK_TYPE CALLBACK
#define mhs_sleep(x) Sleep(x)
#else
// ****** Linux>
#include <unistd.h>
#define CALLBACK_TYPE
#define mhs_sleep(x) usleep((x) * 1000)
#endif

GtkWidget *create_menue_button(const gchar *stock, const gchar *text, const gchar *secondary);

#ifdef __cplusplus
  }
#endif

#endif
