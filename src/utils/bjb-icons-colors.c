#include "bjb-bijiben.h"
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

/* TODO
 * this func will probably disappear but otherwise it has
 * to handle size & directories */
GdkPixbuf *
get_note_pixbuf()
{
  GdkPixbuf *note_pixbuf ;
  GError *error = NULL ;

  const gchar * icons_path = bijiben_get_bijiben_dir (); 

  gchar * full_path = g_strdup_printf ("%s/bijiben/icons/hicolor/16x16/actions/note.png",
                                       icons_path);

  note_pixbuf = gdk_pixbuf_new_from_file_at_size(full_path,
                                                 16,
                                                 16,
                                                 &error);
  g_free(full_path);
  
  if (error)
  {
    g_message("Error is %s",error->message);
    g_error_free (error);
    return NULL ;
  }
  
  if (!note_pixbuf)
  {
    g_message("Notes pixbuf error.");
    return NULL ;
  }

  return note_pixbuf ;  
}
