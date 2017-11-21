/*******************************************************************************
                            util.c  -  description
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
#include <string.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include "util.h"


#define SET_RESULT(result, value) if ((result)) *(result) = (value)

char *get_item_as_string(char **str, char *trenner, int *result)
{
int hit, cnt, l;
char *s, *t, *start, *end, *item;

if ((!str) || (!trenner))
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
s = *str;
if (!s)
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
// Führende Leerzeichen überspringen
if (*s == '\0')
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
SET_RESULT(result, 0);
end = s;
item = s;
start = s;

hit = 0;
for (s = end; *s; s++)
  {
  cnt = 0;
  for (t = trenner; *t; t++)
    {
    cnt++;
    if (*s == *t)
      {
      *s++ = '\0';
      end = s;
      SET_RESULT(result, cnt);
      hit = 1;
      break;
      }
    }
  if (hit)
    break;
  }
// Abschliesende Leerzeichen löschen
if ((l = strlen(item)))
  {
  s = item + (l-1);
  while ((l) && ((isspace((int)*s)) || (*s == '\n') || (*s == '\r')))
  {
    l--;
    s--;
  }
  if (l)
    s++;
  *s = 0;
  }
if (end == start)
  *str = start + strlen(end);
else
  *str = end;
return(item);
}


void UpdateGtk(void)
{
while (gtk_events_pending())
  gtk_main_iteration();
}


GtkWidget *create_menue_button(const gchar *stock, const gchar *text, const gchar *secondary)
{
GtkWidget *button, *align, *image, *hbox, *label, *vbox;

button = gtk_button_new ();
label = gtk_label_new (NULL);
gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), text);
gtk_label_set_mnemonic_widget (GTK_LABEL (label), button);

vbox = gtk_vbox_new (FALSE, 2);
if (stock)
  {
  image = gtk_image_new_from_stock (stock, GTK_ICON_SIZE_DIALOG);
  hbox = gtk_hbox_new (FALSE, 20);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  gtk_box_pack_end (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  }

align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

label = gtk_label_new (NULL);
gtk_label_set_text (GTK_LABEL (label), secondary);
gtk_box_pack_end (GTK_BOX (vbox), label, FALSE, FALSE, 0);

gtk_container_add (GTK_CONTAINER (button), align);
if (stock)
  gtk_container_add (GTK_CONTAINER (align), hbox);
else
  gtk_container_add (GTK_CONTAINER (align), vbox);
gtk_widget_show_all (align);

return(button);
}

