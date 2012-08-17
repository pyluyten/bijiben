
#ifndef _BIJI_NOTE_EDITOR_H_
#define _BIJI_NOTE_EDITOR_H_

#include <glib-object.h>
#include <gtk/gtk.h>

// GOBJECT 

G_BEGIN_DECLS

#define BIJI_TYPE_NOTE_EDITOR             (biji_note_editor_get_type ())
#define BIJI_NOTE_EDITOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_NOTE_EDITOR, BijiNoteEditor))
#define BIJI_NOTE_EDITOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_NOTE_EDITOR, BijiNoteEditorClass))
#define BIJI_IS_NOTE_EDITOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_NOTE_EDITOR))
#define BIJI_IS_NOTE_EDITOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_NOTE_EDITOR))
#define BIJI_NOTE_EDITOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_NOTE_EDITOR, BijiNoteEditorClass))

typedef struct _BijiNoteEditorPrivate BijiNoteEditorPrivate;
typedef struct _BijiNoteEditorClass BijiNoteEditorClass;
typedef struct _BijiNoteEditor BijiNoteEditor;

struct _BijiNoteEditorClass
{
	GtkTextViewClass parent_class;
};

struct _BijiNoteEditor
{
	GtkTextView parent_instance;
	BijiNoteEditorPrivate *priv;
};

GType biji_note_editor_get_type (void) G_GNUC_CONST;

// Accessors

void _biji_note_editor_set_text_is_selected(gpointer biji_note_editor,gboolean val) ;
gboolean _biji_note_editor_get_text_is_selected(gpointer biji_note_editor) ;

// Callbacks

gboolean key_pressed (GtkWidget *gtk_text_view, GdkEventKey *ev);

GtkTextView *biji_gtk_view_new(gpointer *note_obj);

G_END_DECLS

#endif /* _BIJI_NOTE_EDITOR_H_ */
