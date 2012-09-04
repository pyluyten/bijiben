#ifndef _BJB_WINDOW_BASE_H
#define _BJB_WINDOW_BASE_H

#include <gtk/gtk.h>
#include <clutter-gtk/clutter-gtk.h>
#include <libbiji/libbiji.h>

#include "bjb-settings.h"
#include "bjb-controller.h"

/* The main window has three possible states
 * Opened notes are like tabs.*/
enum {
  VIEW_ALL_NOTES_WINDOW,
  VIEW_TAGS_WINDOW,
  EDIT_NOTE_WINDOW
};

#define BJB_TYPE_WINDOW_BASE                  (bjb_window_base_get_type ())
#define BJB_WINDOW_BASE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), BJB_TYPE_WINDOW_BASE, BjbWindowBase))
#define BJB_IS_WINDOW_BASE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BJB_TYPE_WINDOW_BASE))
#define BJB_WINDOW_BASE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), BJB_TYPE_WINDOW_BASE, BjbWindowBaseClass))
#define BJB_IS_WINDOW_BASE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), BJB_TYPE_WINDOW_BASE))
#define BJB_WINDOW_BASE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), BJB_TYPE_WINDOW_BASE, BjbWindowBaseClass))

typedef struct _BjbWindowBase        BjbWindowBase;
typedef struct _BjbWindowBaseClass   BjbWindowBaseClass;

typedef struct _BjbWindowBasePriv BjbWindowBasePriv;

struct _BjbWindowBaseClass
{
  GtkApplicationWindowClass parent_class;
};


struct _BjbWindowBase
{
  GtkApplicationWindow parent_instance ;
  BjbWindowBasePriv *priv;
};



GType bjb_window_base_get_type (void);

GtkWindow * bjb_window_base_new(GtkApplication *app) ;

// Accessor 

BjbController * bjb_window_base_get_controller ( BjbWindowBase *window ) ;

PangoFontDescription *bjb_window_base_get_font(GtkWidget *window);

void bjb_window_base_set_frame(BjbWindowBase *bwb,ClutterActor *frame);

ClutterActor * bjb_window_base_get_frame(BjbWindowBase *bwb);

ClutterActor * bjb_window_base_get_stage(BjbWindowBase *bwb);

gpointer bjb_window_base_get_app(GtkWidget *win);

void bjb_window_base_set_application ( BjbWindowBase *self, GtkApplication *app);

BjbSettings *bjb_window_base_get_settings(GtkWidget *win);

BijiNoteBook * bjb_window_base_get_book(GtkWidget * win);

void bjb_window_base_set_book(GtkWidget * win, BijiNoteBook *notes);

GList * bjb_window_base_get_tags(GtkWidget * win);

void bjb_window_base_set_tags(GtkWidget * win, GList * tags);

void bjb_window_base_set_entry(GtkWidget *win, gchar *search_entry) ;

void bjb_window_base_delete_entry(GtkWidget *win);

gchar * bjb_window_base_get_entry(GtkWidget *win) ;

// DEBUG 
void biji_application_print_note_id(GtkWindow* win);

gboolean switch_window_fullscreen();

#endif /* _BJB_WINDOW_BASE_H */
