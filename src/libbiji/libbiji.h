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

#ifndef _LIB_BIJI_H
#define _LIB_BIJI_H

#define _LIBBIJI_INSIDE_H

#include <gtk/gtk.h>

#include "biji-date-time.h"
#include "biji-note-book.h"
#include "biji-note-editor.h"

G_BEGIN_DECLS

#define BIJI_DEBUG(msg)   g_log(G_LOG_DOMAIN,G_LOG_LEVEL_DEBUG,msg);
#define BIJI_WARNING(msg) g_log(G_LOG_DOMAIN,G_LOG_LEVEL_WARNING,msg);

/**************** STEP ONE : gather the data **********************************/

/* Get a collection of notes from tomboy format files */
BijiNoteBook *biji_book_new_from_dir(gchar *tomboy_format_folder);

// Create a Note from a tomboy.note file
BijiNoteObj* biji_note_get_new_from_file (gchar* path); // delete

/* Get a blanck new note FIXME set path, dates, content */
BijiNoteObj* biji_note_get_new_from_string (gchar* title,gchar *folder);

/* Get all the tomboy notes */
BijiNoteBook *note_book_new_from_tomboy_dir();

/* Get all the Gnote notes */
GList* biji_collection_get_gnote_notes(); // FIXME

//TODO NoteBook *note_book_add_PIM_notes(NoteBook *book,gchar* vcf_file);
//TODO NoteBook *note_book_add_text_file_as_note(NoteBook *book,gchar* content);


/******************** STEP TWO : access the data ******************************/

// Collection accessors & signals
void note_book_append_new_note(BijiNoteBook *book,BijiNoteObj *note);
gboolean biji_note_book_remove_note(BijiNoteBook *book,BijiNoteObj *note);

// Best: use path (unique). Title is supposed to be unique too.
// note_book_get_nth should be deprecated ASAP.

// Return the list of notes. Free this list. Do not free the note.
GList * biji_note_book_get_notes(BijiNoteBook *book);

int note_book_get_notes_number(BijiNoteBook *book);

BijiNoteObj * note_book_get_note_at_path(BijiNoteBook *book,gchar *path);

BijiNoteObj * note_book_get_nth_data(BijiNoteBook *book,int nth);

BijiNoteObj * note_book_get_note(BijiNoteBook *book,gchar *title);

// Return the list of notes. Free this list. Do not free the note.
GList * biji_note_book_get_notes_with_tag(BijiNoteBook *book,gchar* tag);

// Return the list of notes. Free this list. Do not free the note.
GList * biji_note_book_get_notes_with_tag_prefix(BijiNoteBook *book,gchar* tag);

// Return the list of notes. Free this list. Do not free the note.
GList * biji_note_book_get_no_tag_notes(BijiNoteBook *book);

void biji_note_book_remove_tag(BijiNoteBook *book,gchar *tag);

BijiNoteObj * biji_note_book_get_tag_template(BijiNoteBook *book, gchar *tag);

// Single note accessor & signals
gchar *biji_note_get_title(BijiNoteObj* note_obj_ptr);
gchar *biji_note_get_raw_text(BijiNoteObj *note);
int set_note_title(BijiNoteObj* note_obj_ptr,gchar* title) ;
gchar * biji_note_get_content(BijiNoteObj* note_obj_ptr);
gboolean biji_note_obj_is_template(BijiNoteObj *note);
gchar *note_obj_get_path(BijiNoteObj *note);
gboolean biji_note_obj_has_tag(BijiNoteObj *note,gchar *tag);
GList *biji_note_obj_get_tags(BijiNoteObj *note);
gboolean biji_note_obj_add_tag(BijiNoteObj *note, gchar *tag);
gboolean biji_note_obj_remove_tag(BijiNoteObj *note,gchar *tag);
gchar *biji_note_obj_get_last_change_date(BijiNoteObj *note);
gchar *biji_note_obj_get_create_date(BijiNoteObj *note);

G_END_DECLS

#undef _LIBBIJI_INSIDE_H

#endif /*_LIB_BIJI_H*/
