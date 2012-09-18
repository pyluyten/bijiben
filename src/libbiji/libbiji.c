/* libiji.h
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
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "libbiji.h"
#include "biji-note-buffer.h"
#include "biji-note-editor.h"
#include "biji-note-obj.h"
#include "biji-note-tag.h"
#include "biji-read-tomboy.h"


BijiNoteObj*
biji_note_get_new_from_file (gchar* path)
{
  BijiNoteObj* ret =  g_object_new(BIJI_TYPE_NOTE_OBJ,NULL);
  set_note_id_path(note_get_id(ret),path);

	// TODO : check file type. if tomboy implement below. 
	// Else use another module
  //set_note_status(ret,load_tomboy_note((gpointer)ret));
	load_tomboy_note((gpointer)ret);
  //g_message("status is %d",get_note_status(ret)) ;
  //if (get_note_status(ret))
	//{
	//	print_note_id(note_get_id(ret));
	//}
  return ret ;
}

BijiNoteObj*
biji_note_get_new_from_string (gchar* title, gchar *folder)
{
    BijiNoteObj *ret = g_object_new(BIJI_TYPE_NOTE_OBJ,NULL);
		BijiNoteID *id = note_get_id(ret);
    _biji_note_obj_set_title(ret,title);

		GRand *random = g_rand_new();
	  gint suffix = g_rand_int(random);
	  g_rand_free (random);
		set_note_id_path(id,g_strdup_printf ("%s/%i.note",folder,suffix));
		
    return ret ;
}

BijiNoteBook *
biji_book_new_from_dir(gchar *tomboy_format_folder)
{
  if (tomboy_format_folder != NULL)
  {
    return _biji_note_book_get_new_from_dir(tomboy_format_folder);
  }
  return NULL ;
}

BijiNoteBook *
note_book_new_from_tomboy_dir()
{
    gchar *tomboy_dir ;
    tomboy_dir = g_strdup_printf("%s/tomboy",g_get_user_data_dir());
    return _biji_note_book_get_new_from_dir(tomboy_dir);
}

int note_book_get_notes_number(BijiNoteBook *book)
{
  if (BIJI_IS_NOTE_BOOK(book))
  {
    return _biji_book_get_number(book);
  }
  else return 0 ;
}
