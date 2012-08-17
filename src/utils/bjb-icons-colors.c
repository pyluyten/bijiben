#include "bjb-icons-colors.h"

GtkWidget *get_icon (gchar *icon)
{
  GIcon *gi ;
    
  gi = g_themed_icon_new_with_default_fallbacks (icon);
  return gtk_image_new_from_gicon (gi,GTK_ICON_SIZE_BUTTON);
}

GdkRGBA *
get_color(gchar *color)
{
  GdkRGBA *result = g_new0 ( GdkRGBA , 1 ) ;
    
  if ( gdk_rgba_parse(result,color) )
    return result ;

  else
    g_message("Critical : cannot get color %s",color);

  return NULL ;
}

GdkColor *
get_gdk_color (gchar *color)
{
  GdkColor * result = g_new0(GdkColor,1 );
                  
  if ( gdk_color_parse (color,result) )
        return result ;

  g_message("critical : get gdk color");
  return NULL ;
  
}