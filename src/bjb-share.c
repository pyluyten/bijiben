/* biji-share.c
 * Copyright (C) Pierre-Yves LUYTEN 2012 <py@luyten.fr>
 * 
 * bijiben is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * bijiben is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bjb-share.h"

static gchar *
mail_str ( gchar * string )
{
  return biji_str_replace (string, "\n", " ");
}

/* TODO find EOL for xdg-email */
gboolean
on_email_note_callback(GtkWidget *widget, BijiNoteObj *note)
{
  GError *error;    
  error = NULL;
  gchar *title_mail, *text_mail;

  title_mail = mail_str (biji_note_obj_get_title (note));
  text_mail = mail_str (biji_note_get_raw_text (note));

  gchar *execute[7] = { "xdg-email",
                        "--utf8",
                        "--subject",
                        title_mail,
                        "--body",
                        text_mail,
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

  g_free (title_mail);
  g_free (text_mail);

  return TRUE ;
}

