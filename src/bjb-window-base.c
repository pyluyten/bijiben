
#include <glib/gprintf.h>
#include <stdlib.h>

#include <libbiji/libbiji.h>

#include "bjb-bijiben.h"
#include "bjb-window-base.h"
#include "bjb-main-view.h"
#include "bjb-note-view.h"


#define BJB_WIDTH 880
#define BJB_HEIGHT 600
#define BIJIBEN_MAIN_WIN_TITLE "Bijiben"

#define BJB_DEFAULT_FONT "Serif 10"

/* Window available views..
 * Not sure we'll use it... */
enum { 
  MAIN_VIEW,
  TAGS_VIEW,
  NOTE_EDITOR 
};

/* As the main window remains, it owns the data */
struct _BjbWindowBasePriv
{
  /* To register new windows and access the data */
  GtkApplication *app ;
  BjbController  *controller;
    
  /* The real data */
  GList *tags ;

  /* UI */
  ClutterActor *stage, *frame ;
  int current_view ;
  gchar *entry ;

  /* To avoid loiding several times */
  PangoFontDescription *font ;

  /* Tmp. Instead we want to be able to create a window base
   * without the view */
  BjbMainView       *view;
};

/* Gobject */
G_DEFINE_TYPE (BjbWindowBase, bjb_window_base, GTK_TYPE_APPLICATION_WINDOW);


static GObject *
bjb_window_base_constructor (GType                  gtype,
                             guint                  n_properties,
                             GObjectConstructParam  *properties)
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
  BjbWindowBase *self = BJB_WINDOW_BASE (object);
  BjbWindowBasePriv *priv = self->priv;

  g_clear_object (&priv->controller);
  G_OBJECT_CLASS (bjb_window_base_parent_class)->finalize (object);
}

static void
bjb_window_base_class_init (BjbWindowBaseClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->constructor = bjb_window_base_constructor;
  gobject_class->finalize = bjb_window_base_finalize ;

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
  BjbWindowBasePriv *priv;
  const gchar *icons_path;
  gchar *full_path;
  GList *icons = NULL;
  GdkPixbuf *bjb ;
  GError *error = NULL ;
  GtkClutterEmbed *embed;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
                                           BJB_TYPE_WINDOW_BASE,
                                           BjbWindowBasePriv);
  priv = self->priv;
    
  gtk_window_set_default_size (GTK_WINDOW (self), BJB_WIDTH, BJB_HEIGHT);
  gtk_window_set_position (GTK_WINDOW (self),GTK_WIN_POS_CENTER);
  gtk_window_set_title (GTK_WINDOW (self), BIJIBEN_MAIN_WIN_TITLE);

  /* Icon for window. TODO - Should be BjbApp */
  icons_path = bijiben_get_bijiben_dir ();
  full_path = g_build_filename (icons_path,
                                "icons",
                                "hicolor",
                                "48x48",
                                "apps",
                                "bijiben.png",
                                NULL);

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
  priv->entry = NULL ;

  priv->tags = get_all_tracker_tags();
  priv->font = pango_font_description_from_string (BJB_DEFAULT_FONT);

  /* UI */
  embed = GTK_CLUTTER_EMBED (gtk_clutter_embed_new());
  gtk_clutter_embed_set_use_layout_size (embed, TRUE);
  
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (embed));
  priv->stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (embed));

  /* Signals */
  g_signal_connect(GTK_WIDGET(self),"destroy",
                   G_CALLBACK(bjb_window_base_destroy),self);
}

GtkWindow *
bjb_window_base_new(void)
{
  BjbWindowBase *retval;
  BjbWindowBasePriv *priv;

  retval = g_object_new(BJB_TYPE_WINDOW_BASE,
                        "application", g_application_get_default(),
                        "hide-titlebar-when-maximized", TRUE,
                        NULL);
  priv = retval->priv;

  priv->controller = bjb_controller_new 
    (bijiben_get_book (BIJIBEN_APPLICATION(g_application_get_default())),
     priv->entry );

  /* UI : notes view. But some settings could allow other default. */
  priv->view = bjb_main_view_new ( GTK_WIDGET(retval),priv->controller);
  priv->frame = bjb_main_view_get_actor(priv->view);

  return GTK_WINDOW (retval);
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
bjb_window_base_set_frame(BjbWindowBase *bwb,ClutterActor *frame)
{
  /* TODO removing frame should finalize
   * or we can implement some interface
   * (bool) (hide_frame) (bwb) */
  if ( bwb->priv->frame )
  {
    clutter_actor_destroy(bwb->priv->frame);
    bwb->priv->frame = NULL ;
  }

  if ( CLUTTER_IS_ACTOR( bwb->priv->frame) )
  {
    bwb->priv->frame = frame ;
    clutter_actor_add_child (bwb->priv->stage, frame);
  }
}

ClutterActor *
bjb_window_base_get_frame(BjbWindowBase *bwb)
{
  return bwb->priv->frame ;
}

ClutterActor *
bjb_window_base_get_stage(BjbWindowBase *bwb)
{
  return bwb->priv->stage ;
}

BijiNoteBook *
bjb_window_base_get_book(GtkWidget * win)
{
   BjbWindowBase *b = BJB_WINDOW_BASE(win);

   if ( b->priv )
   {
     return bijiben_get_book(BIJIBEN_APPLICATION(g_application_get_default()));
   }

   else
   {
       g_message("Can't get notes");
       return NULL ;
   }
}

GList *
bjb_window_base_get_tags(GtkWidget * win)
{
    BjbWindowBase *b = BJB_WINDOW_BASE(win);

  if ( b->priv )
    return b->priv->tags ;

  else
    return NULL ;
}

void 
bjb_window_base_set_tags(GtkWidget * win, GList * tags)
{
  BjbWindowBase *b = BJB_WINDOW_BASE(win);
  
  if ( b->priv->tags != NULL )
    g_list_free_full (b->priv->tags,(GDestroyNotify) g_free);
  
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

gpointer
bjb_window_base_get_main_view (BjbWindowBase *self)
{
  return (gpointer) self->priv->view;
}
