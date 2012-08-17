/* It's the frame, that is, almost the window. But we do use
 * gtk view from libbiji */

#ifndef _NOTE_VIEW
#define _NOTE_VIEW

#include <glib-object.h>

#include <libbiji/libbiji.h>

G_BEGIN_DECLS

#define BJB_TYPE_NOTE_VIEW bjb_note_view_get_type()

#define BJB_NOTE_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), BJB_TYPE_NOTE_VIEW, BjbNoteView))

#define BJB_NOTE_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), BJB_TYPE_NOTE_VIEW, BjbNoteViewClass))

#define BJB_NOTE_IS_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BJB_TYPE_NOTE_VIEW))

#define BJB_NOTE_IS_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), BJB_TYPE_NOTE_VIEW))

#define BJB_NOTE_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), BJB_TYPE_NOTE_VIEW, BjbNoteViewClass))

typedef struct _BjbNoteViewPrivate BjbNoteViewPrivate;

typedef struct {
  GtkFrame parent;
  BjbNoteViewPrivate * priv ;
} BjbNoteView;

typedef struct {
  GtkFrameClass parent_class;
} BjbNoteViewClass;

GType bjb_note_view_get_type (void);

BjbNoteView * bjb_note_view_new (GtkWidget *win,BijiNoteObj* note, gboolean is_main_window);


G_END_DECLS

#endif /* _NOTE_VIEW */