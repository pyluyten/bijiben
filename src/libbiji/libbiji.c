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

/* Single Note */

gchar*
biji_note_get_title(BijiNoteObj *note_obj_ptr)
{
  if (BIJI_IS_NOTE_OBJ(note_obj_ptr))
  {
    return _biji_note_obj_get_title(note_obj_ptr);
  }
  else
  {
    return "This is NOT a NoteObj" ;
  }
}

gchar *
biji_note_get_raw_text(BijiNoteObj *note)
{
  g_return_val_if_fail(BIJI_IS_NOTE_OBJ(note),NULL);

  return _biji_note_obj_get_raw_text(note);
}

int
set_note_title(BijiNoteObj *note_obj_ptr,gchar *title)
{
  BijiNoteObj* n = BIJI_NOTE_OBJ(note_obj_ptr) ;
  _biji_note_obj_set_title(n,title);

  return 0 ;
}

gchar *
biji_note_get_content(BijiNoteObj *n)
{
  return note_obj_get_content (n);
}

gboolean
biji_note_obj_is_template(BijiNoteObj *note)
{
  return note_obj_is_template(note);
}

gchar *note_obj_get_path(BijiNoteObj *note)
{
  return get_note_path(note);
}

gboolean
biji_note_obj_has_tag(BijiNoteObj *note,gchar *tag)
{
  return _biji_note_obj_has_tag(note,tag);
}

GList *biji_note_obj_get_tags(BijiNoteObj *note)
{
  return _biji_note_obj_get_tags(note);
}

gboolean
biji_note_obj_add_tag(BijiNoteObj *note, gchar *tag)
{
  g_message("Biji note obj add tag - NEW way");

  // If the note has already the tag, return FALSE
  if  ( _biji_note_obj_has_tag(note,tag) )
  {
    return FALSE ;
  }

  _biji_note_obj_add_tag(note,tag);
  return TRUE ;
}

gboolean
biji_note_obj_remove_tag(BijiNoteObj *note,gchar *tag)
{
  // If the note has tag, remove it.
  if ( _biji_note_obj_has_tag(note,tag) )
  {
    GList *current = _biji_note_obj_get_tags(note);
    gint i ;
    gchar *to_remove = NULL ;
    
    for ( i = 0 ; i < g_list_length(current) ; i++ )
    {
      if ( g_strcmp0 (g_list_nth_data(current,i),tag) == 0 )
      {
        to_remove = g_list_nth_data(current,i);
      }
    }
    
    _biji_note_obj_set_tags(note,g_list_remove(current,to_remove));
    return TRUE ;
  }

  // Else return false
  return FALSE ;
}

gchar *biji_note_obj_get_last_change_date(BijiNoteObj *note)
{
  return biji_note_id_get_last_change_date(note_get_id(note));
}

gchar *biji_note_obj_get_create_date(BijiNoteObj *note)
{
  return biji_note_id_get_create_date(note_get_id(note));
}

/********************************************************************/
/****************** PART THREE : edit data ************************/
/********************************************************************/

/* Note Editor */
GtkTextView *
biji_text_view_new_from_note(BijiNoteObj *note)
{
  g_return_val_if_fail(BIJI_IS_NOTE_OBJ(note),NULL);
  return biji_gtk_view_new((gpointer)note);
}

gboolean 
biji_note_obj_is_opened(BijiNoteObj *note)
{
  return _biji_note_obj_is_opened(note) ;
}

void
biji_note_close(BijiNoteObj *note)
{
  _biji_note_obj_close_note(note);
}

gchar *
gtk_text_view_get_selection(GtkTextView *editor)
{
  GtkTextIter start;
  GtkTextIter end;

  GtkTextBuffer *buffer = gtk_text_view_get_buffer(editor);
  g_return_val_if_fail(GTK_IS_TEXT_BUFFER(buffer),NULL);

  if (gtk_text_buffer_get_selection_bounds(buffer, &start, &end))
    return gtk_text_buffer_get_text(buffer,&start,&end,FALSE); 

  return NULL ;
}


/*
static void
get_tag_name_and_append(GtkTextTag *iter,GSList **result)
{
  *(result) = g_slist_append(*(result), biji_note_tag_get_name(iter) );
}*/

// SOIT ON TROUVE LE TAG PAR SON NOM
// SOIT ON RETRIEVE TOUS LES TAG, CHERCHE LES NOMS ET COMPARE...
gboolean
gtk_text_view_selection_has_tag(GtkTextView *editor, gchar *name)
{
  GtkTextIter start,end;
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(editor);
  GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer) ,
                                              name) ;
    
  if ( gtk_text_buffer_get_selection_bounds(buffer,&start,&end) )
  {
    if ( gtk_text_iter_has_tag(&start,tag) || gtk_text_iter_has_tag(&start,tag))
    {
      return TRUE ;
    }
    return FALSE ;
  }
  return FALSE ;
}

void
biji_toggle_bold_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"bold");
}

void
biji_toggle_italic_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"italic");
}

void
biji_toggle_underline_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"underline");
}

void
biji_toggle_strike_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"strikethrough");
}

void
biji_toggle_monospace_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"monospace");
}

void
biji_toggle_highlight_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"highlight");
}

void biji_toggle_tag_by_name(GtkTextView *editor,gchar *name)
{
  buffer_selection_toggle_tag_by_name(gtk_text_view_get_buffer(editor),
                                      name);
}

void 
biji_remove_all_format(GtkTextView *editor)
{
  note_buffer_remove_tags(gtk_text_view_get_buffer(editor));
}

gboolean
biji_augment_font_size(GtkTextView *editor)
{
  return augment_font_size(gtk_text_view_get_buffer(editor));
}

gboolean 
biji_decrease_font_size(GtkTextView *editor)
{
  return decrease_font_size(gtk_text_view_get_buffer(editor));
}

void biji_augment_depth(GtkTextView *editor)
{
  GtkTextBuffer *buf = gtk_text_view_get_buffer(editor);
  augment_depth(BIJI_NOTE_BUFFER(buf));
}

void biji_decrease_depth(GtkTextView *editor)
{
  GtkTextBuffer *buf = gtk_text_view_get_buffer(editor);
  decrease_depth(BIJI_NOTE_BUFFER(buf));
}

