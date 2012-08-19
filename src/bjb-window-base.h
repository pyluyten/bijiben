#ifndef _BIJI_MAIN_WINDOW_H
#define _BIJI_MAIN_WINDOW_H

#include <gtk/gtk.h>
#include <libbiji/libbiji.h>

#include "bjb-settings.h"



/* The main window has three possible states
 * Opened notes are like tabs.*/
enum {
  VIEW_ALL_NOTES_WINDOW,
  VIEW_TAGS_WINDOW,
  EDIT_NOTE_WINDOW
};

#define BIJI_TYPE_MAIN_WINDOW                  (biji_main_window_get_type ())
#define BIJI_MAIN_WINDOW(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_MAIN_WINDOW, BijiMainWindow))
#define BIJI_IS_MAIN_WINDOW(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_MAIN_WINDOW))
#define BIJI_MAIN_WINDOW_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_MAIN_WINDOW, BijiMainWindowClass))
#define BIJI_IS_MAIN_WINDOW_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_MAIN_WINDOW))
#define BIJI_MAIN_WINDOW_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_MAIN_WINDOW, BijiMainWindowClass))

typedef struct _BijiMainWindow        BijiMainWindow;
typedef struct _BijiMainWindowClass   BijiMainWindowClass;

typedef struct _BijiMainWindowPriv BijiMainWindowPriv;

struct _BijiMainWindowClass
{
  GtkApplicationWindowClass parent_class;
};


struct _BijiMainWindow
{
  GtkApplicationWindow parent_instance ;
  BijiMainWindowPriv *priv;
};



GType biji_main_window_get_type (void);

GtkWindow * create_main_window_with_notes(GtkApplication *app) ;

// Accessor 
PangoFontDescription *window_base_get_font(GtkWidget *window);
GdkPixbuf * main_window_get_note_pixbuf(GtkWidget *window);

GdkPixbuf * get_note_pixbuf() ;

void main_window_set_frame(gpointer win,GtkContainer *frame);

GtkContainer * main_window_get_frame(gpointer win);

gpointer main_window_get_app(GtkWidget *win);

void bjb_main_window_set_application ( BijiMainWindow *self, GtkApplication *app);

BjbSettings *main_window_get_settings(GtkWidget *win);

BijiNoteBook * bijiben_window_get_book(GtkWidget * win);

void biji_win_set_book(GtkWidget * win, BijiNoteBook *notes);

GList * biji_win_get_tags(GtkWidget * win);

void biji_win_set_tags(GtkWidget * win, GList * tags);

void biji_win_set_entry(GtkWidget *win, gchar *search_entry) ;

void biji_win_delete_entry(GtkWidget *win);

gchar * biji_win_get_entry(GtkWidget *win) ;

// DEBUG 
void biji_application_print_note_id(GtkWindow* win);

gboolean switch_window_fullscreen();

#endif /* _BIJI_MAIN_WINDOW_H */
