#ifndef BJB_ICONS_H
#define BJB_ICONS_H

#include <gtk/gtk.h>

GtkWidget * get_icon (gchar *icon);

GdkRGBA * get_color(gchar *color);

GdkColor * get_gdk_color (gchar *color);

GdkPixbuf * get_note_pixbuf();

#endif
