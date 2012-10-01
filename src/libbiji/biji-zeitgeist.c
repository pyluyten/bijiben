/* biji-zeitgeist.c
 * Copyright (C) Pierre-Yves LUYTEN 2011 <py@luyten.fr>
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
 * with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include "biji-zeitgeist.h"

#include <libbiji/libbiji.h>
#include "biji-zeitgeist.h"

static ZeitgeistLog *global_log;

static ZeitgeistLog *
get_log (void)
{
  if (global_log == NULL )
    global_log = g_object_new (ZEITGEIST_TYPE_LOG, NULL);

  return global_log ;
}

void 
insert_zeitgeist(BijiNoteObj *note, const char *action)
{
  gchar *uri;
  ZeitgeistEvent     *event;
  ZeitgeistSubject   *subject ;
  ZeitgeistLog       *log = get_log() ;

  uri = g_strdup_printf ("file://%s", note_obj_get_path (note));

  subject = zeitgeist_subject_new_full (uri,                            //URI
                                        ZEITGEIST_NFO_DOCUMENT,         //inter
                                        ZEITGEIST_NFO_FILE_DATA_OBJECT, //mani
                                        "application/x-note",           //mime
                                        "",                             //origin
                                        biji_note_get_title (note),     //text
                                        "") ;                           //storage

  g_free (uri);

  event = zeitgeist_event_new_full (action,
                                    ZEITGEIST_ZG_USER_ACTIVITY,
                                    "application://bijiben.desktop",
                                    subject,
                                    NULL );

  zeitgeist_log_insert_events_no_reply (log, event, NULL);
}
