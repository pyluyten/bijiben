/* biji-note-obj.h
 * Copyright (C) Pierre-Yves LUYTEN 2012 <py@luyten.fr>
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

#ifndef _BIJI_NOTE_OBJ_H_
#define _BIJI_NOTE_OBJ_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "biji-note-id.h"

G_BEGIN_DECLS

/* Available formating for biji_note_obj_editor_apply_format
 * If note is opened, and if text is opened
 * This toggle the format
 * eg bold text will become normal and normal text becomes bold */
typedef enum
{
  BIJI_NO_FORMAT,
  BIJI_BOLD,
  BIJI_ITALIC,
  BIJI_STRIKE,
  BIJI_BULLET_LIST,
  BIJI_ORDER_LIST
} BijiEditorFormat;

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
gchar* biji_note_obj_get_path (BijiNoteObj *n);
BijiNoteID* note_get_id(BijiNoteObj *n);
int set_note_last_change_date(BijiNoteObj* n,gchar* date) ;

glong biji_note_obj_get_last_change_date_sec ( BijiNoteObj *n );
gchar * biji_note_obj_get_last_change_date_string (BijiNoteObj *self);

int set_note_last_metadata_change_date(BijiNoteObj* n,gchar* date) ;
int set_note_create_date(BijiNoteObj* n,gchar* date);
int get_note_status(BijiNoteObj* n);
void set_note_status(BijiNoteObj* n, int status) ;
gboolean biji_note_obj_get_rgba(BijiNoteObj *n, GdkRGBA *rgba) ;
void biji_note_obj_set_rgba(BijiNoteObj *n, GdkRGBA *rgba) ;

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
void biji_note_obj_save_note (BijiNoteObj *self);

GdkPixbuf * biji_note_obj_get_icon (BijiNoteObj *note);

gchar *biji_note_get_title(BijiNoteObj* note_obj_ptr);

gchar *biji_note_get_raw_text(BijiNoteObj *note);

void biji_note_obj_set_raw_text (BijiNoteObj *note, gchar *plain_text);

int set_note_title(BijiNoteObj* note_obj_ptr,gchar* title) ;

gboolean biji_note_obj_is_template(BijiNoteObj *note);

gboolean biji_note_obj_has_tag(BijiNoteObj *note,gchar *tag);

GList *biji_note_obj_get_tags(BijiNoteObj *note);

gboolean biji_note_obj_add_tag(BijiNoteObj *note, gchar *tag);

gboolean biji_note_obj_remove_tag(BijiNoteObj *note,gchar *tag);

gchar *biji_note_obj_get_last_change_date(BijiNoteObj *note);

gchar *biji_note_obj_get_create_date(BijiNoteObj *note);

/* Webkit : note edition */

GtkWidget * biji_note_obj_open                      (BijiNoteObj *note);

gboolean biji_note_obj_is_opened                    (BijiNoteObj *note);

GtkWidget * biji_note_obj_get_editor                (BijiNoteObj *note);

void biji_note_obj_set_html_content                 (BijiNoteObj *note, gchar *html);

gchar * biji_note_obj_get_html                      (BijiNoteObj *note);

void biji_note_obj_editor_apply_format              (BijiNoteObj *note,
                                                           gint format);

gboolean biji_note_obj_editor_has_selection         (BijiNoteObj *note);

gchar * biji_note_obj_editor_get_selection (BijiNoteObj *note);

void biji_note_obj_editor_cut                       (BijiNoteObj *note);

void biji_note_obj_editor_copy                      (BijiNoteObj *note);

void biji_note_obj_editor_paste                     (BijiNoteObj *note);

G_END_DECLS

#endif /* _BIJI_NOTE_OBJ_H_ */
