
#include <glib/gprintf.h>
#include <stdlib.h>

#include <libbiji/libbiji.h>

#include "bjb-bijiben.h"
#include "bjb-tracker.h"
#include "bjb-window-base.h"
#include "bjb-main-view.h"
#include "bjb-note-view.h"

#define BJB_WIDTH 600
#define BJB_HEIGHT 500


/* Window available views..
 * Not sure we'll use it... */
enum { 
  MAIN_VIEW,
  TAGS_VIEW,
  NOTE_EDITOR 
};

// Properties binded to gsettings.
enum
{
  PROP_0,
  PROP_GTK_APP,
  N_PROPERTIES
};

/* As the main window remains, it owns the data */
struct _BijiMainWindowPriv
{
  // To register new windows and access the data.
  GtkApplication *app ;
    
  // The real data
  GList *tags ;

  // actually only the frame is used
  GtkContainer *frame ;
  int current_view ;
  gchar *entry ;

  // To avoid loiding several times
  GdkPixbuf * note_pixbuf;
  PangoFontDescription *font ;
};

/* Gobject */
G_DEFINE_TYPE (BijiMainWindow, biji_main_window, GTK_TYPE_APPLICATION_WINDOW);


static GObject *
biji_main_window_constructor (GType                  gtype,
                       guint                  n_properties,
                       GObjectConstructParam *properties)
{
  GObject *obj;
  {
    obj = G_OBJECT_CLASS (biji_main_window_parent_class)->constructor (gtype, n_properties, properties);
  }
  return obj;
}


static void
biji_main_window_finalize (GObject *object)
{
	/*FIXME : should g list free full the notes*/
}

static void
biji_main_window_class_init (BijiMainWindowClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->constructor = biji_main_window_constructor;
  gobject_class->finalize = biji_main_window_finalize ;


  /*
  g_object_class_install_property (gobject_class,PROP_GTK_APP,
                                   g_param_spec_pointer ("bjb-app",
							                		     "Bjb App",
                                                         "Bjb App",
							                              G_PARAM_READWRITE));
														  */
	
  g_type_class_add_private (klass, sizeof (BijiMainWindowPriv));
}

static void
biji_main_window_destroy (gpointer a, BijiMainWindow * self)
{
}

/* Gobj */
static void 
biji_main_window_init (BijiMainWindow *self) 
{
  self->priv = 
  G_TYPE_INSTANCE_GET_PRIVATE(self,BIJI_TYPE_MAIN_WINDOW,BijiMainWindowPriv);

  self->priv->tags = NULL ;

  // We probably want to offer a no entry window at first (startup)
  self->priv->entry = NULL ;

  /* title is set by frame. icon is app wide. */
  gtk_widget_set_size_request (GTK_WIDGET (self), 300, 150 );
  gtk_window_set_default_size (GTK_WINDOW(self),  BJB_WIDTH, BJB_HEIGHT );
  gtk_window_set_position(GTK_WINDOW(self),GTK_WIN_POS_CENTER);
    
  // connectors
  // TODO
  g_signal_connect(GTK_WIDGET(self),"destroy",
                   G_CALLBACK(biji_main_window_destroy),self);
	//g_signal_connect(G_OBJECT(self),"note-added",G_CALLBACK(update_main_window_notes),self);
	//g_signal_connect(G_OBJECT(self),"note-removed",G_CALLBACK(update_main_window_notes),self);
}

GdkPixbuf *
main_window_get_note_pixbuf(GtkWidget *window)
{
  BijiMainWindow *win = BIJI_MAIN_WINDOW(window);
    
  return win->priv->note_pixbuf ;
}

static GdkPixbuf *
get_note_pixbuf()
{
  GdkPixbuf *note_pixbuf ;
  GError *error = NULL ;

  const gchar * icons_path = bijiben_get_bijiben_dir (); 
  gchar * full_path = g_strdup_printf ("%s/bijiben/icons/hicolor/scalable/actions/note.svg",
                                       icons_path);

  note_pixbuf = gdk_pixbuf_new_from_file_at_size(full_path,
                                                 48,
                                                 48,
                                                 &error);
  g_free(full_path);
    
  if (error)
  {
    g_message("Error is %s",error->message);
    g_error_free (error);
    return NULL ;
  }

  if (!note_pixbuf)
  {
    g_message("Notes pixbuf error.");
    return NULL ;
  }
  
  return note_pixbuf ;  
}

GtkWindow *
create_main_window_with_notes(GtkApplication *app)
{    
  BijiMainWindow *ret ;
  const gchar *icons_path;
  gchar *full_path;
  GList *icons = NULL;
  GdkPixbuf *test ;
  GError *error = NULL ;

  // FIXME
  icons_path = bijiben_get_bijiben_dir ();
  full_path = g_strdup_printf ("%s/icons/hicolor/48x48/apps/bijiben.png", icons_path);
  test = gdk_pixbuf_new_from_file (full_path, &error);
  g_free (full_path);
  if ( error )
  {
    g_message("Error loading bijiben icon: %s",error->message);
    g_error_free(error);
  }
  icons = g_list_prepend(icons,test);
  gtk_window_set_default_icon_list(icons);
  g_list_foreach (icons, (GFunc) g_object_unref, NULL);
  g_list_free (icons);
	
  g_message("FIXME : create_main_window_with_notes (gtk-application warning)");
  ret = g_object_new(BIJI_TYPE_MAIN_WINDOW,"gtk-application",app,NULL);
  GtkWindow *win = GTK_WINDOW(ret);
  ret->priv->app = app ;
  ret->priv->note_pixbuf = get_note_pixbuf();
  gtk_window_set_application (win, GTK_APPLICATION (app));
  gtk_window_set_title (win, "Notes");
  gtk_window_set_hide_titlebar_when_maximized(win,TRUE);
                   

  ret->priv->tags = get_all_tracker_tags();
  ret->priv->font = pango_font_description_from_string ("Serif 10");

  ret->priv->frame=GTK_CONTAINER(bjb_main_view_new(GTK_WIDGET(ret),
                                                   bijiben_get_book(app)));

  gtk_container_add(GTK_CONTAINER(ret),GTK_WIDGET(ret->priv->frame));
  gtk_widget_show_all(GTK_WIDGET(ret));

  prepare_view_for_usage((BjbMainView*)ret->priv->frame);

  return win ;
}

PangoFontDescription *
window_base_get_font(GtkWidget *window)
{
  BijiMainWindow *b = BIJI_MAIN_WINDOW(window);
  return b->priv->font ;
}

void
main_window_set_frame(gpointer win,GtkContainer *frame)
{
  BijiMainWindow *b = BIJI_MAIN_WINDOW(win);
  if ( b->priv->frame )
  {
    if ( GTK_IS_CONTAINER(b->priv->frame))
    {
      gtk_widget_destroy(GTK_WIDGET(b->priv->frame));
      b->priv->frame= frame ;
      gtk_container_add(GTK_CONTAINER(win),GTK_WIDGET(b->priv->frame));
      gtk_widget_show_all(GTK_WIDGET(frame));
      return ;
    }
  }
  b->priv->frame = frame ;
}

GtkContainer *
main_window_get_frame(gpointer win)
{
  if (BIJI_IS_MAIN_WINDOW(win))
    return BIJI_MAIN_WINDOW(win)->priv->frame ;

  else
    return NULL ;
}

gpointer main_window_get_app(GtkWidget *win)
{
  return BIJI_MAIN_WINDOW(win)->priv->app ;
}

BjbSettings *main_window_get_settings(GtkWidget *win)
{
  return bjb_app_get_settings(BIJI_MAIN_WINDOW(win)->priv->app);
}

BijiNoteBook *
bijiben_window_get_book(GtkWidget * win)
{
   BijiMainWindow *b = BIJI_MAIN_WINDOW(win);
   if ( b->priv )
   {
      return bijiben_get_book(b->priv->app) ;
   }
   else
   {
       g_message("Can't get notes");
       return NULL ;
   }
}

void
biji_win_set_book(GtkWidget *win, BijiNoteBook *notes)
{
  BijiMainWindow *b = BIJI_MAIN_WINDOW(win);
  if (b->priv)
  {
    bijiben_set_book(b->priv->app,notes) ;
  }
  else
  {
    g_message("Cannot set notes collection.");
  }
}

GList *
biji_win_get_tags(GtkWidget * win)
{
    BijiMainWindow *b = BIJI_MAIN_WINDOW(win);
    if ( b->priv )
    {
       return b->priv->tags ;
    }
    else
    {
        return NULL ;
    }
}

void 
biji_win_set_tags(GtkWidget * win, GList * tags)
{
    BijiMainWindow *b = BIJI_MAIN_WINDOW(win);
    if ( b->priv->tags != NULL )
    {
       g_list_free_full (b->priv->tags,
                         (GDestroyNotify) g_free);
    }
    b->priv->tags = tags ;
}


void
biji_win_set_entry(GtkWidget *win, gchar *search_entry)
{
  BijiMainWindow *bmw = BIJI_MAIN_WINDOW(win);
  bmw->priv->entry = search_entry ; 
}

void biji_win_delete_entry(GtkWidget *win)
{
  BIJI_MAIN_WINDOW(win)->priv->entry = NULL ;
}

gchar *
biji_win_get_entry(GtkWidget *win)
{
  BijiMainWindow *bmw = BIJI_MAIN_WINDOW(win);
  return bmw->priv->entry ;
}


