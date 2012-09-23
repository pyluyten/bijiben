#ifndef _BIJI_NOTE_BUFFER_H_
#define _BIJI_NOTE_BUFFER_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "biji-note-obj.h"

G_BEGIN_DECLS

#define BIJI_TYPE_NOTE_BUFFER             (biji_note_buffer_get_type ())
#define BIJI_NOTE_BUFFER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_NOTE_BUFFER, BijiNoteBuffer))
#define BIJI_NOTE_BUFFER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_NOTE_BUFFER, BijiNoteBufferClass))
#define BIJI_IS_NOTE_BUFFER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_NOTE_BUFFER))
#define BIJI_IS_NOTE_BUFFER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_NOTE_BUFFER))
#define BIJI_NOTE_BUFFER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_NOTE_BUFFER, BijiNoteBufferClass))

typedef struct _BijiNoteBufferPrivate BijiNoteBufferPrivate;
typedef struct _BijiNoteBufferClass BijiNoteBufferClass;
typedef struct _BijiNoteBuffer BijiNoteBuffer;

struct _BijiNoteBufferClass
{
	GtkTextBufferClass parent_class;
};

struct _BijiNoteBuffer
{
	GtkTextBuffer parent_instance;
	BijiNoteBufferPrivate *priv;
};

GType biji_note_buffer_get_type (void) G_GNUC_CONST;

// Funct to be used by libbiji ONLY internaly..........................
void apply_active_tags(GtkTextBuffer *buffer,GtkTextIter *iter, const gchar *input, gpointer user_data);
gpointer note_buffer_get_obj(BijiNoteBuffer *buf) ;
void note_buffer_update_active_tags (BijiNoteBuffer *self);
gchar *get_bullet_by_depth( gint depth);

// Funct to offer throught libbiji.h...................................
BijiNoteBuffer * create_note_buffer(gpointer note_obj);

// Tags
gboolean is_active_tag(BijiNoteBuffer *buf, gchar *tag_name);
void buffer_selection_toggle_tag_by_name(GtkTextBuffer *buffer,gchar *name);
void note_buffer_remove_tags(GtkTextBuffer *buffer);
gboolean augment_font_size(GtkTextBuffer *gtk_text_buffer);
gboolean decrease_font_size(GtkTextBuffer *gtk_text_buffer);
void format_note_title(GtkTextBuffer *buffer);

// bullet lists AT CURSOR LINE
void note_buffer_enable_bullets(BijiNoteBuffer *buffer);
void note_buffer_disable_bullets(BijiNoteBuffer *buffer);
gboolean augment_depth(BijiNoteBuffer *note_buf);
gboolean decrease_depth(BijiNoteBuffer *note_buf);

// Callbacks etc.....................................................

gboolean note_buffer_add_new_line(GtkTextBuffer *buf);
gboolean note_buffer_backspace_handler(BijiNoteBuffer *buffer);
gboolean note_buffer_add_tab(BijiNoteBuffer *buffer);
gboolean note_buffer_remove_tab(BijiNoteBuffer *buffer);
void note_buffer_check_selection(BijiNoteBuffer *buffer,gpointer biji_note_editor);
/*void on_insert_text_event() */

// temp debug
void gtk_note_buffer_debug_xml(GtkTextBuffer *buffer) ;

/* Public methods
 * TODO : implement an interface*/

GtkTextView *biji_text_view_new_from_note(BijiNoteObj *note);

void biji_note_close(BijiNoteObj *note) ; 

gchar *gtk_text_view_get_selection(GtkTextView *editor);

gboolean gtk_text_view_selection_has_tag(GtkTextView *editor, gchar *name);

void biji_toggle_tag_by_name(GtkTextView *editor,gchar *name);

void biji_toggle_bold_tag(GtkTextView *editor);

void biji_toggle_italic_tag(GtkTextView *editor);

void biji_toggle_strike_tag(GtkTextView *editor);

void biji_toggle_underline_tag(GtkTextView *editor);

void biji_toggle_highlight_tag(GtkTextView *editor);

void biji_toggle_monospace_tag(GtkTextView *editor);

void biji_remove_all_format(GtkTextView *editor);

gboolean biji_augment_font_size(GtkTextView *editor);

gboolean biji_decrease_font_size(GtkTextView *editor);

void biji_augment_depth(GtkTextView *editor);

void biji_decrease_depth(GtkTextView *editor);

G_END_DECLS

#endif /* _BIJI_NOTE_BUFFER_H_ */
