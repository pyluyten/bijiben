
#include <glib/gprintf.h>
#include <stdlib.h>

#include <libbiji/libbiji.h>

#include "bjb-bijiben.h"
#include "bjb-tracker.h"
#include "bjb-window-base.h"
#include "bjb-main-view.h"
#include "bjb-note-view.h"


#define BJB_WIDTH 880
#define BJB_HEIGHT 780
#define BJB_DEFAULT_FONT "Serif 10"


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
struct _BjbWindowBasePriv
{
  // To register new windows and access the data.
  GtkApplication *app ;
  BjbController  *controller;
    
  // The real data
  GList *tags ;

  // actually only the frame is used
  GtkContainer *frame ;
  int current_view ;
  gchar *entry ;

  // To avoid loiding several times
  PangoFontDescription *font ;
};

/* Gobject */
G_DEFINE_TYPE (BjbWindowBase, bjb_window_base, GTK_TYPE_APPLICATION_WINDOW);


static GObject *
bjb_window_base_constructor (GType                  gtype,
                       guint                  n_properties,
                       GObjectConstructParam *properties)
{
  GObject *obj;
  {
    obj = G_OBJECT_CLASS (bjb_window_base_parent_class)->constructor (gtype, n_properties, properties);
  }
  return obj;
}


static void
bjb_window_base_finalize (GObject *object)
{
	/* TODO */
}

static void
bjb_window_base_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  BjbWindowBase *self = BJB_WINDOW_BASE (object);

  switch (property_id)
    {
    case PROP_GTK_APP:
      g_value_set_object (value, self->priv->app);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
bjb_window_base_set_property (GObject    *object,
                              guint       property_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
  BjbWindowBase *self = BJB_WINDOW_BASE (object);

  switch (property_id)
    {
    case PROP_GTK_APP:
	  bjb_window_base_set_application(self,g_value_get_object(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
bjb_window_base_class_init (BjbWindowBaseClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    
  gobject_class->constructor = bjb_window_base_constructor;
  gobject_class->get_property = bjb_window_base_get_property;
  gobject_class->set_property = bjb_window_base_set_property;
  gobject_class->finalize = bjb_window_base_finalize ;

  g_object_class_install_property (gobject_class,PROP_GTK_APP,
                                   g_param_spec_object ("gtk-application",
							                		    "Gtk-Application",
                                                        "Gtk Application",
                                                        GTK_TYPE_APPLICATION,
							                            G_PARAM_READWRITE));
														  
	
  g_type_class_add_private (klass, sizeof (BjbWindowBasePriv));
}

static void
bjb_window_base_destroy (gpointer a, BjbWindowBase * self)
{
}

/* Gobj */
static void 
bjb_window_base_init (BjbWindowBase *self) 
{
    const gchar *icons_path;
    gchar *full_path;
    GList *icons = NULL;
    GdkPixbuf *bjb ;
    GError *error = NULL ;
    
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
                                             BJB_TYPE_WINDOW_BASE,
                                             BjbWindowBasePriv);
    
    /* Title is set by frame. icon is app wide. */
    gtk_widget_set_size_request (GTK_WIDGET (self), 300, 150 );
    gtk_window_set_default_size (GTK_WINDOW(self),  BJB_WIDTH, BJB_HEIGHT );
    gtk_window_set_position(GTK_WINDOW(self),GTK_WIN_POS_CENTER);

    /* Icon for window. TODO - Should be BjbApp */
    icons_path = bijiben_get_bijiben_dir ();
    full_path = g_strdup_printf ("%s/icons/hicolor/48x48/apps/bijiben.png", icons_path);
    bjb = gdk_pixbuf_new_from_file (full_path, &error);
    g_free (full_path);
    
    if ( error )
    {
        g_message("%s", error->message);
        g_error_free(error);
    }
    
    icons = g_list_prepend(icons,bjb);
    gtk_window_set_default_icon_list(icons);
    g_list_foreach (icons, (GFunc) g_object_unref, NULL);
    g_list_free (icons);

    /*  We probably want to offer a no entry window at first (startup) */
    self->priv->entry = NULL ;

    self->priv->tags = get_all_tracker_tags();
    self->priv->font = pango_font_description_from_string (BJB_DEFAULT_FONT);
    
    /* Signals */
    g_signal_connect(GTK_WIDGET(self),"destroy",
                     G_CALLBACK(bjb_window_base_destroy),self);
}

GtkWindow *
bjb_window_base_new(GtkApplication *app)
{    
  BjbWindowBase *ret ;
  BjbController *controller ;
	
  ret = g_object_new(BJB_TYPE_WINDOW_BASE,"gtk-application",app,NULL);
  GtkWindow *win = GTK_WINDOW(ret);
  ret->priv->app = app ;
  gtk_window_set_application (win, GTK_APPLICATION (app));
  gtk_window_set_title (win, "Notes");
  gtk_window_set_hide_titlebar_when_maximized(win,TRUE);

  controller = bjb_controller_new(bijiben_get_book(app) , ret->priv->entry );
  ret->priv->controller = controller ;
  
  bjb_window_base_set_frame(win,
                            bjb_main_view_new(ret,
                                              bijiben_get_book(app),
                                              controller));

  prepare_view_for_usage((BjbMainView*)ret->priv->frame);

  return win ;
}

BjbController *
bjb_window_base_get_controller ( BjbWindowBase *window )
{
  return window->priv->controller ;
}

PangoFontDescription *
window_base_get_font(GtkWidget *window)
{
  BjbWindowBase *b = BJB_WINDOW_BASE(window);
  return b->priv->font ;
}

void
bjb_window_base_set_frame(gpointer win,GtkContainer *frame)
{
  BjbWindowBase *b ;
  
  b = BJB_WINDOW_BASE(win);
  
  if ( b->priv->frame && GTK_IS_CONTAINER(b->priv->frame))
  {
    gtk_widget_destroy(GTK_WIDGET(b->priv->frame));
    b->priv->frame= frame ;
  }
  
  else
  {
    b->priv->frame = frame ;
  }
  
  gtk_container_add(GTK_CONTAINER(win),GTK_WIDGET(b->priv->frame));
}

GtkContainer *
bjb_window_base_get_frame(gpointer win)
{
  if (BJB_IS_WINDOW_BASE(win))
    return BJB_WINDOW_BASE(win)->priv->frame ;

  else
    return NULL ;
}

gpointer bjb_window_base_get_app(GtkWidget *win)
{
  return BJB_WINDOW_BASE(win)->priv->app ;
}

BjbSettings * bjb_window_base_get_settings(GtkWidget *win)
{
  return bjb_app_get_settings(BJB_WINDOW_BASE(win)->priv->app);
}

BijiNoteBook *
bjb_window_base_get_book(GtkWidget * win)
{
   BjbWindowBase *b = BJB_WINDOW_BASE(win);
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
bjb_window_base_set_book(GtkWidget *win, BijiNoteBook *notes)
{
  BjbWindowBase *b = BJB_WINDOW_BASE(win);
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
bjb_window_base_get_tags(GtkWidget * win)
{
    BjbWindowBase *b = BJB_WINDOW_BASE(win);
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
bjb_window_base_set_tags(GtkWidget * win, GList * tags)
{
    BjbWindowBase *b = BJB_WINDOW_BASE(win);
    if ( b->priv->tags != NULL )
    {
       g_list_free_full (b->priv->tags,
                         (GDestroyNotify) g_free);
    }
    b->priv->tags = tags ;
}


void
bjb_window_base_set_entry(GtkWidget *win, gchar *search_entry)
{
  BjbWindowBase *bmw = BJB_WINDOW_BASE(win);
  bmw->priv->entry = search_entry ; 
}

void bjb_window_base_delete_entry(GtkWidget *win)
{
  BJB_WINDOW_BASE(win)->priv->entry = NULL ;
}

gchar *
bjb_window_base_get_entry(GtkWidget *win)
{
  BjbWindowBase *bmw = BJB_WINDOW_BASE(win);
  return bmw->priv->entry ;
}

void
bjb_window_base_set_application ( BjbWindowBase *self, GtkApplication *app)
{
    self->priv->app = app ;
}
