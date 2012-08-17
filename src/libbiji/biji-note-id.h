/* BIJI NOTE ID is private */


#ifndef _BIJI_NOTE_ID_H_
#define _BIJI_NOTE_ID_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define BIJI_TYPE_NOTE_ID             (biji_note_id_get_type ())
#define BIJI_NOTE_ID(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_NOTE_ID, BijiNoteID))
#define BIJI_NOTE_ID_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_NOTE_ID, BijiNoteIDClass))
#define BIJI_IS_NOTE_ID(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_NOTE_ID))
#define BIJI_IS_NOTE_ID_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_NOTE_ID))
#define BIJI_NOTE_ID_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_NOTE_ID, BijiNoteIDClass))

typedef struct _BijiNoteIDClass BijiNoteIDClass;
typedef struct _BijiNoteID BijiNoteID;

typedef struct _BijiNoteIDPrivate BijiNoteIDPrivate;

struct _BijiNoteIDClass
{
  GObjectClass parent_class;
};

struct _BijiNoteID
{
  GObject parent_instance;
  BijiNoteIDPrivate* priv;
};

GType biji_note_id_get_type (void) G_GNUC_CONST;

/* DEBUG TMP */
void print_note_id(BijiNoteID* n);

/* Generic */
gboolean _biji_note_id_are_same(BijiNoteID *a, BijiNoteID *b);

gchar* biji_note_id_get_path(BijiNoteID* n);
int set_note_id_path(BijiNoteID* n,gchar* path);
void _biji_note_id_set_title(BijiNoteID* n,gchar* title);
gchar* _biji_note_id_get_title(BijiNoteID* n);

/* Dates */ 
gchar * biji_note_id_get_last_change_date(BijiNoteID* n);
glong biji_note_id_get_last_change_date_sec(BijiNoteID *n);
int set_note_id_last_change_date(BijiNoteID* n,gchar* date);
void _biji_note_id_set_change_date_now(BijiNoteID *n);
gchar * biji_note_id_get_last_metadata_change_date(BijiNoteID* n);
int set_note_id_last_metadata_change_date(BijiNoteID* n,gchar* date);
void _biji_note_id_set_metadata_change_now(BijiNoteID *n);
gchar * biji_note_id_get_create_date(BijiNoteID* n);
int set_note_id_create_date(BijiNoteID* n,gchar* date);


G_END_DECLS

#endif /* _BIJI_NOTE_ID_H_ */
