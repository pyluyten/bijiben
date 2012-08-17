#include "bjb-share.h"

static gchar *
mail_str ( gchar * string )
{   
  return g_strjoinv(" ", g_strsplit(string, "\n", -1));
}

/* TODO find EOL for xdg-email */
gboolean
on_email_note_callback(GtkWidget *widget, BijiNoteObj *note)
{
  GError *error ;    
  error = NULL ;

  gchar *execute[7] = { "xdg-email",
                        "--utf8",
                        "--subject",
                        mail_str(biji_note_get_title(note)),
                        "--body",
                        mail_str(biji_note_get_raw_text(note)),
                        NULL } ;
                        
  g_spawn_async_with_pipes ( NULL, 
                             execute,
                             NULL,
                             G_SPAWN_SEARCH_PATH,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             &error); 
  if ( error != NULL )
    g_message("error :%s",error->message);

  return TRUE ;
}