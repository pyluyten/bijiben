/* Libbiji comes from file(s) to note(s) (or notebook)
 * Notes (or books) allows to show a gtkview to edit them, autosave
 * and make coffee.
 *
 * All public func for libbiji are there.DO NOT USE FUNCT FROM OTHER PARTS.
 * 
 * A NoteObj is a note, containing : NoteID, gboolean "opened"...Undoer
 * NoteID are : title, (absolute) path, tags, content
 * 
 * A NoteBook is a collection of notes.    
 *
 */

#ifndef _LIB_BIJI_H
#define _LIB_BIJI_H

#include <gtk/gtk.h>
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

/****************** STEP THREE : EDIT DATA         ****************************/

// 1. An idea would be to provide a frame containing this text view  to keep 
//    GtkTextView (and buffer) private.but that's not a good idea.
//    
// 2. When you do get a new tet view, then close note.
GtkTextView *biji_text_view_new_from_note(BijiNoteObj *note);

gboolean biji_note_obj_is_opened(BijiNoteObj *note);

// This saves the note and might watch how many view are opened.
void biji_note_close(BijiNoteObj *note) ; 

// Free the gchar value unless NULL
gchar *gtk_text_view_get_selection(GtkTextView *editor);

// Get the list of strings , gs_list_free_full it with gfree each name.
gboolean gtk_text_view_selection_has_tag(GtkTextView *editor, gchar *name);

void biji_toggle_tag_by_name(GtkTextView *editor,gchar *name);
void biji_toggle_bold_tag(GtkTextView *editor);
void biji_toggle_italic_tag(GtkTextView *editor);
void biji_toggle_strike_tag(GtkTextView *editor);
void biji_toggle_underline_tag(GtkTextView *editor);
void biji_toggle_highlight_tag(GtkTextView *editor);
void biji_toggle_monospace_tag(GtkTextView *editor);
void biji_remove_all_format(GtkTextView *editor);


// Return True if the size was changed, False is nothing done.
gboolean biji_augment_font_size(GtkTextView *editor);
gboolean biji_decrease_font_size(GtkTextView *editor);

void biji_augment_depth(GtkTextView *editor);
void biji_decrease_depth(GtkTextView *editor);

// DEBUG BLOCK
void biji_print_note(GtkTextBuffer *buf);

G_END_DECLS

#endif /*_LIB_BIJI_H*/
