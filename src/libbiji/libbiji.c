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

/********************************************************************/
/************************** PART ONE ********************************/
/********************************************************************/

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

/********************************************************************/
/****************** PART TWO : access data **************************/
/********************************************************************/

/* Notes collection */
void note_book_append_new_note(BijiNoteBook *book,BijiNoteObj *note)
{    
  if (BIJI_IS_NOTE_BOOK(book))
  {
    if ( BIJI_IS_NOTE_OBJ(note) )
    {
      _biji_note_book_add_one_note(book,note);
    }
    else
    {
      g_message("You trying to add a car to a list of planes");  
    }
  }
  else
  {
    // TODO return biji_note_book_with_single_note(NoteObj *note)
    g_message("ERROR. NOT A BOOK.");
  }
}

gboolean 
biji_note_book_remove_note(BijiNoteBook *book,BijiNoteObj *note)
{
	g_return_val_if_fail(BIJI_IS_NOTE_BOOK(book),FALSE);
	g_return_val_if_fail(BIJI_IS_NOTE_OBJ(note),FALSE);

	return _note_book_remove_one_note(book,note);
}

BijiNoteObj * note_book_get_nth_data(BijiNoteBook *book,int nth)
{
  if (BIJI_IS_NOTE_BOOK(book))
  {
    return _biji_book_get_nth(book,nth);
  }
  else return NULL ;
}

GList * biji_note_book_get_notes(BijiNoteBook *book)
{
  return _biji_note_book_get_notes(book);
}

BijiNoteObj *
note_book_get_note_at_path(BijiNoteBook *book,gchar *path)
{
		gint i,len;
		len = note_book_get_notes_number (book);
		
		for (i=0 ; i<len; i++)
		{
			BijiNoteObj *o = _biji_book_get_nth(book,i);
			gchar *current = note_obj_get_path (o);
			if ( g_strcmp0 (current,path) == 0 )
				{
						return o ;
				}
		}
		return NULL ;
}


BijiNoteObj *
note_book_get_note(BijiNoteBook *book,gchar *title)
{
		gint i,len;
		len = note_book_get_notes_number (book);
		
		for (i=0 ; i<len; i++)
		{
			BijiNoteObj *o = _biji_book_get_nth(book,i);
			gchar *current = biji_note_get_title (o);
			if ( g_strcmp0 (current,title) == 0 )
				{
						return o ;
				}
		}
		return NULL ;
}

GList * 
biji_note_book_get_notes_with_tag(BijiNoteBook *book,gchar* tag)
{
  g_return_val_if_fail(BIJI_IS_NOTE_BOOK(book),NULL);
  return _biji_note_book_get_notes_with_tag(book,tag);
}

GList * 
biji_note_book_get_notes_with_tag_prefix(BijiNoteBook *book,gchar* tag)
{
  return _biji_note_book_get_notes_with_tag_prefix(book,tag);
}

GList * 
biji_note_book_get_no_tag_notes(BijiNoteBook *book)
{
  return _biji_note_book_get_no_tag_notes(book);
}

void 
biji_note_book_remove_tag(BijiNoteBook *book,gchar *tag)
{
  GList *notes = _biji_note_book_get_notes_with_tag(book,tag) ;

  // Remove the tag then save the note.
  g_list_foreach(notes,(GFunc)biji_note_obj_remove_tag,tag);
  g_list_foreach(notes,(GFunc)note_obj_save_note_using_buffer,NULL);
}

BijiNoteObj *
biji_note_book_get_tag_template(BijiNoteBook *book, gchar *tag)
{
  GList *notes = _biji_note_book_get_notes_with_tag(book,tag) ;

  if ( notes == NULL ) 
    return NULL ;

  gint i ;
  for ( i=0 ; i < g_list_length (notes) ; i++ )
  {
    BijiNoteObj *note = BIJI_NOTE_OBJ ( g_list_nth_data(notes,i));
    if ( note_obj_is_template(note))
     return note ;
  }

  return NULL ;
}
