#ifndef _BIJI_NOTE_BOOK_H_
#define _BIJI_NOTE_BOOK_H_

#include <glib-object.h>

#include "biji-note-obj.h"

G_BEGIN_DECLS

#define BIJI_TYPE_NOTE_BOOK             (biji_note_book_get_type ())
#define BIJI_NOTE_BOOK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_NOTE_BOOK, BijiNoteBook))
#define BIJI_NOTE_BOOK_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_NOTE_BOOK, BijiNoteBookClass))
#define BIJI_IS_NOTE_BOOK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_NOTE_BOOK))
#define BIJI_IS_NOTE_BOOK_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_NOTE_BOOK))
#define BIJI_NOTE_BOOK_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_NOTE_BOOK, BijiNoteBookClass))

typedef struct _BijiNoteBookClass BijiNoteBookClass;
typedef struct _BijiNoteBook BijiNoteBook;

typedef struct _BijiNoteBookPrivate BijiNoteBookPrivate;

struct _BijiNoteBookClass
{
	GObjectClass parent_class;
};

struct _BijiNoteBook
{
	GObject parent_instance;
	BijiNoteBookPrivate *priv ;
};

GType biji_note_book_get_type (void) G_GNUC_CONST; 

BijiNoteBook *_biji_note_book_get_new_from_dir(gchar* folder);
gboolean _biji_note_book_is_title_unique(BijiNoteBook *book,gchar *title);
void _biji_note_book_add_one_note(BijiNoteBook *book,BijiNoteObj *note);
gboolean _note_book_remove_one_note(BijiNoteBook *book,BijiNoteObj *note);
void _biji_note_book_add_note_to_tag_book(BijiNoteBook *book,BijiNoteObj *note,gchar *tag);

// This GLIST should be free but not the notes
GList *_biji_note_book_get_notes(BijiNoteBook *book);

// This GLIST should be free but not the notes
GList * _biji_note_book_get_notes_with_tag(BijiNoteBook *book,gchar *tag);

GList * _biji_note_book_get_no_tag_notes(BijiNoteBook *book);

// This GLIST should be free but not the notes
GList * _biji_note_book_get_notes_with_tag_prefix(BijiNoteBook *book,gchar *tag);

gint _biji_book_get_number(BijiNoteBook *book);

// This func should be erased later on.
BijiNoteObj * _biji_book_get_nth(BijiNoteBook *book,int i);

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

G_END_DECLS

#endif /* _BIJI_NOTE_BOOK_H_ */
