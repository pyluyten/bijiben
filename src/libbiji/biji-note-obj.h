/* NoteOb is the foundation of libbiji.
 * It's a note : it's data (BijiNoteID , content )
 * and access to this data (NoteBuffer,NoteView,NoteUndoer)
 * and signals (renamed / changed / deleted / created )
 *
 * BUT MOST PART HERE IS NOT PUBLIC. I HAVE TO FIX THIS
 * BEST WOULD BE TO GATHER PUBLIC STUFF INTO LIBBIJI.H
 * 
 */

#ifndef _BIJI_NOTE_OBJ_H_
#define _BIJI_NOTE_OBJ_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "biji-note-id.h"

G_BEGIN_DECLS

#define BIJI_TYPE_NOTE_OBJ             (biji_note_obj_get_type ())
#define BIJI_NOTE_OBJ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_NOTE_OBJ, BijiNoteObj))
#define BIJI_NOTE_OBJ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_NOTE_OBJ, BijiNoteObjClass))
#define BIJI_IS_NOTE_OBJ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_NOTE_OBJ))
#define BIJI_IS_NOTE_OBJ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_NOTE_OBJ))
#define BIJI_NOTE_OBJ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_NOTE_OBJ, BijiNoteObjClass))

typedef struct _BijiNoteObjClass BijiNoteObjClass;
typedef struct _BijiNoteObj BijiNoteObj;

typedef struct _BijiNoteObjPrivate BijiNoteObjPrivate;

struct _BijiNoteObjClass
{
  GObjectClass parent_class;
};

struct _BijiNoteObj
{
  GObject parent_instance;
  BijiNoteObjPrivate* priv ;
};

GType biji_note_obj_get_type (void) G_GNUC_CONST;

// This structure, for convenience, associates a tag with its notes
typedef struct
{
  gchar *name ;
  BijiNoteObj *template_note ;
  GList *notes ;
  gint length ;
} TagBook ;

/////////////////////////////////////////////////// Relationships other notes
gpointer biji_note_obj_get_note_book(BijiNoteObj *note);
void _biji_note_obj_set_book(BijiNoteObj *note, gpointer biji_note_book);
void biji_note_obj_delete(BijiNoteObj *dead);
gboolean note_obj_are_same(BijiNoteObj *a, BijiNoteObj* b);

///////////////////////////////////////////////////////////////////// Metadata 
void _biji_note_obj_set_title(BijiNoteObj *obj,gchar *title);
gchar * _biji_note_obj_get_title(BijiNoteObj *obj);
gchar* get_note_path (BijiNoteObj *n);
BijiNoteID* note_get_id(BijiNoteObj *n);
int set_note_last_change_date(BijiNoteObj* n,gchar* date) ;

glong biji_note_obj_get_last_change_date_sec ( BijiNoteObj *n );
gchar * biji_note_obj_get_last_change_date_string (BijiNoteObj *self);

int set_note_last_metadata_change_date(BijiNoteObj* n,gchar* date) ;
int set_note_create_date(BijiNoteObj* n,gchar* date);
int get_note_status(BijiNoteObj* n);
void set_note_status(BijiNoteObj* n, int status) ;
GdkRGBA * biji_note_obj_get_rgba(BijiNoteObj *n) ;
void biji_note_obj_set_rgba(BijiNoteObj *n, GdkRGBA *rgba) ;

///////////////////////////////////////////////    XML content, buffer, editor
void _biji_note_obj_increment_opened(BijiNoteObj *note) ;
void biji_note_set_content(BijiNoteObj* n, gchar* content);
gchar* note_obj_get_content(BijiNoteObj*n);
gchar * _biji_note_obj_get_raw_text(BijiNoteObj *n);
GtkTextTagTable *note_obj_get_gtk_tags(BijiNoteObj *note);
void biji_note_set_gtk_tags(BijiNoteObj *note, GtkTextTagTable *table);
GtkTextBuffer *biji_note_get_or_create_buffer(gpointer biji_note_obj);
gint _biji_note_obj_get_left_margin(BijiNoteObj *obj);
gboolean _biji_note_obj_is_opened(BijiNoteObj *note);

//////////////////////////////////////////////////////  Tags (as in tag clouds)
GList * _biji_note_obj_get_tags(BijiNoteObj *n) ;
gboolean _biji_note_obj_has_tag(BijiNoteObj *note,gchar *tag);
void _biji_note_obj_add_tag(BijiNoteObj *note, gchar *tag) ;
gboolean _biji_note_obj_has_tag_prefix(BijiNoteObj *note,gchar *tag);
void _biji_note_obj_set_tags(BijiNoteObj *n, GList *tags) ;

///////////////////////////////////////////////////////////////////  templates
gboolean note_obj_is_template(BijiNoteObj *n) ;
void note_obj_set_is_template(BijiNoteObj *n,gboolean is_template);
gchar *_biji_note_template_get_tag(BijiNoteObj *n);

/////////////////////////////////////////////////////////////////// Save
// returns TRUE if note saved , FALSE if not saved
gboolean note_obj_save_note_using_buffer(gpointer biji_note_obj);
void _biji_note_obj_propose_saving(gpointer note_obj);
void _biji_note_obj_close_note(gpointer note_obj);
void _biji_note_obj_mark_as_need_save(gpointer note_obj);

GdkPixbuf * biji_note_obj_get_icon (BijiNoteObj *note);

G_END_DECLS

#endif /* _BIJI_NOTE_OBJ_H_ */
