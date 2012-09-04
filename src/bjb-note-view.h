#ifndef _NOTE_VIEW_H
#define _NOTE_VIEW_H

#include <glib-object.h>
#include <clutter-gtk/clutter-gtk.h>

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
  ClutterActor parent;
  BjbNoteViewPrivate * priv ;
} BjbNoteView;

typedef struct {
  ClutterActorClass parent_class;
} BjbNoteViewClass;

GType bjb_note_view_get_type (void);

BjbNoteView * bjb_note_view_new (GtkWidget *win,BijiNoteObj* note, gboolean is_main_window);

ClutterActor * bjb_note_view_get_actor (BjbNoteView *view);

G_END_DECLS

#endif /* _NOTE_VIEW_H */
