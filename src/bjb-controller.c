/*
 * bijiben
 * Copyright (C) Pierre-Yves Luyten 2012 <py@luyten.fr>
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

#include <libgd/gd.h>

#include "bjb-controller.h"
#include "bjb-main-view.h"
#include "bjb-window-base.h"
#include "utils/bjb-icons-colors.h"


/* Gobject */

struct _BjbControllerPrivate
{
  BijiNoteBook  *book ;
  gchar         *needle ;
  GtkTreeModel  *model ;
  GtkTreeModel  *completion;

  /* Optional
   * Currently Controller is window-wide, in order to live while
   * going from main view to note view.
   * But not app-wide : each win has its controller & needle.
   *
   * gd-main-view is setup by main-view to allow choosing pixbuf
   * according to grid / list view mode */
  GdMainView    *cur;

  /*  Private  */
  GList         *notes_to_show ;
};

enum {
  PROP_0,
  PROP_BOOK ,
  PROP_NEEDLE ,
  PROP_MODEL ,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

/* The Controller wants to inform the toolbar when search starts.
 * But other might be interested to know. */
enum {
  SEARCH_CHANGED,
  BJB_CONTROLLER_SIGNALS
};

static guint bjb_controller_signals [BJB_CONTROLLER_SIGNALS] = { 0 };

#define BJB_CONTROLLER_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BJB_TYPE_CONTROLLER, BjbControllerPrivate))

G_DEFINE_TYPE (BjbController, bjb_controller, G_TYPE_OBJECT);

/* GObject */

static void
bjb_controller_init (BjbController *self)
{
  BjbControllerPrivate *priv  ;
  GtkListStore     *store ;
  GtkListStore     *completion ;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                            BJB_TYPE_CONTROLLER, 
                                            BjbControllerPrivate);
  priv = self->priv ;

  /* Create the columns */
  store = gtk_list_store_new (NUMBER_COLUMNS,
                              G_TYPE_STRING,   // urn
                              G_TYPE_STRING,   // uri
                              G_TYPE_STRING,   // name
                              G_TYPE_STRING,   // author
                              GDK_TYPE_PIXBUF,   // icon then note
                              G_TYPE_INT64,    // mtime
                              G_TYPE_BOOLEAN);   // state

  priv->model = GTK_TREE_MODEL(store) ;
  
  completion  = gtk_list_store_new (1, G_TYPE_STRING);
  
  priv->completion = GTK_TREE_MODEL(completion);

}

static void
free_notes_store(BjbController *self)
{
  GtkListStore *store ;

  store = GTK_LIST_STORE (self->priv->model) ;

  gtk_list_store_clear(store);
}

static void
bjb_controller_finalize (GObject *object)
{
  BjbController *self = BJB_CONTROLLER(object);
  BjbControllerPrivate *priv = self->priv ;

  free_notes_store (self);

  g_object_unref (priv->completion);
  g_free (priv->needle);

  G_OBJECT_CLASS (bjb_controller_parent_class)->finalize (object);
}

static void
bjb_controller_get_property (GObject  *object,
               guint     property_id,
               GValue   *value,
               GParamSpec *pspec)
{
  BjbController *self = BJB_CONTROLLER (object);

  switch (property_id)
  {
  case PROP_BOOK:
    g_value_set_object (value, self->priv->book);
    break;
  case PROP_NEEDLE:
    g_value_set_string(value, self->priv->needle);
    break;
  case PROP_MODEL:
    g_value_set_object(value, self->priv->model);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
bjb_controller_set_property (GObject  *object,
               guint     property_id,
               const GValue *value,
               GParamSpec *pspec)
{
  BjbController *self = BJB_CONTROLLER (object);

  switch (property_id)
  {
  case PROP_BOOK:
    bjb_controller_set_book(self,g_value_get_object(value));
    break;
  case PROP_NEEDLE:
    bjb_controller_set_needle(self,g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

/* Implement model */

static void
bjb_controller_add_note ( BijiNoteObj *note, BjbController *self )
{
  GtkTreeIter    iter;
  GtkListStore  *store;
  GdkPixbuf     *pix = NULL;

  store = GTK_LIST_STORE(self->priv->model);

  /* Only append notes which are not templates. Currently useless */
  if ( biji_note_obj_is_template (note) == FALSE)
  {

    /* First , if there is a gd main view , and if gd main view
     * is a list, then load the 16x16 note pixbuf
     * This is probably not correct but allows a nice list view */
    if (self->priv->cur)
    {
      if (gd_main_view_get_view_type (self->priv->cur) == GD_MAIN_VIEW_LIST)
        pix = get_note_pixbuf();
    }

    /* If no gd-main-view or if not list view,
     * load the icon for grid */
    if (!pix)
      pix = biji_note_obj_get_icon (note);

    /* Appart from pixbuf, both icon & list view types
     * currently use the same model */
    gtk_list_store_append (store,&iter);
    gtk_list_store_set (store, 
                        &iter,
                        COL_URN,    biji_note_obj_get_path(note),
                        COL_URI,    biji_note_obj_get_path(note),
                        COL_NAME,   biji_note_get_title(note),
                        COL_AUTHOR,   NULL,
                        COL_IMAGE,  pix,
                        COL_MTIME,  biji_note_obj_get_last_change_date_sec(note),
                        COL_SELECTED, FALSE,
                        -1);
  }
}

static void
update_view (BjbController *self)
{
  GList *notes ;

  /* Do not update if nothing to show */
  if (!self->priv->cur)
    return;

  notes = self->priv->notes_to_show ;

  free_notes_store(self);
  g_list_foreach (notes,(GFunc)bjb_controller_add_note,self);
}

static glong
most_recent_note_first ( BijiNoteObj *a, BijiNoteObj *b)
{
  glong result = biji_note_obj_get_last_change_date_sec (b);
  return result - biji_note_obj_get_last_change_date_sec (a);
}

static void
sort_notes( BjbController *self)
{
  GList *notes ;

  notes = self->priv->notes_to_show ;
  notes = g_list_sort(notes,(GCompareFunc)most_recent_note_first);
  self->priv->notes_to_show = notes ;
}

static void
add_note_if_searched ( BijiNoteObj *obj , BjbController *self )
{
  gboolean add_note ;
  
  BjbControllerPrivate *priv = self->priv ;
  add_note = FALSE ;

  /* Title match ? */
  if (g_strrstr(g_utf8_casefold(biji_note_get_title(obj),-1),
                            g_utf8_casefold(priv->needle,-1)))
    add_note = TRUE ;

  /* Tag match ? */
  if ( _biji_note_obj_has_tag_prefix(obj,priv->needle))
    add_note = TRUE ;

  /* Content match? */
  if (g_strrstr(g_utf8_casefold(biji_note_get_raw_text(obj),-1),
                              g_utf8_casefold(priv->needle,-1)))
    add_note = TRUE ;


  if ( add_note )
    priv->notes_to_show = g_list_append ( priv->notes_to_show, obj ) ;
}

static void
bjb_controller_apply_needle ( BjbController *self )
{
  gchar *needle ;

  g_list_free(self->priv->notes_to_show);
  self->priv->notes_to_show = NULL ;
  
  needle = self->priv->needle ;

  /* Show all notes */
  if ( needle == NULL || g_strcmp0 (needle,"") == 0)
  {
    self->priv->notes_to_show = biji_note_book_get_notes(self->priv->book);
  } 

  /* Test which note to show */
  else
  {
    GList *all_notes ;

    all_notes = biji_note_book_get_notes(self->priv->book) ;

    g_list_foreach (all_notes,
                   (GFunc) add_note_if_searched,
                    self) ;

    g_list_free(all_notes);

  }

  sort_notes (self) ;
  update_view(self);
}

static void
on_needle_changed ( BjbController *self )
{
  bjb_controller_apply_needle (self);
  g_signal_emit (self, bjb_controller_signals[SEARCH_CHANGED], 0);
}

static void
add_note_to_completion(BijiNoteObj *note , BjbController *self)
{
  GtkListStore *store;
  GtkTreeIter iter;

  store = GTK_LIST_STORE(self->priv->completion);

  // Search Tag.
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, 
                      &iter, 
                      0, 
                      biji_note_get_title(note),
                      -1);
}

static void
refresh_completion(BjbController *self)
{
  gtk_list_store_clear(GTK_LIST_STORE(self->priv->completion));
  
  g_list_foreach(biji_note_book_get_notes(self->priv->book),
                 (GFunc)add_note_to_completion,
                 self);
}

static void
on_book_changed ( BijiNoteBook *book, BjbController *self )
{
  refresh_completion(self);
  bjb_controller_apply_needle (self);
}

static void
bjb_controller_constructed (GObject *obj)
{
  BjbController *self = BJB_CONTROLLER (obj);

  G_OBJECT_CLASS(bjb_controller_parent_class)->constructed(obj);

  /* Rather connect to notes individually
   * and only book for new notes */
  g_signal_connect (self->priv->book, "changed",
                    G_CALLBACK(on_book_changed), self);
}

static void
bjb_controller_class_init (BjbControllerClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (BjbControllerPrivate));

  object_class->get_property = bjb_controller_get_property;
  object_class->set_property = bjb_controller_set_property;
  object_class->finalize = bjb_controller_finalize;
  object_class->constructed = bjb_controller_constructed;

  bjb_controller_signals[SEARCH_CHANGED] = g_signal_new ( "search-changed" ,
                                                  G_OBJECT_CLASS_TYPE (klass),
                                                  G_SIGNAL_RUN_LAST,
                                                  0, 
                                                  NULL, 
                                                  NULL,
                                                  g_cclosure_marshal_VOID__VOID,
                                                  G_TYPE_NONE,
                                                  0);

  properties[PROP_BOOK] = g_param_spec_object ("book",
                                               "Book",
                                               "The BijiNoteBook",
                                               BIJI_TYPE_NOTE_BOOK,
                                               G_PARAM_READWRITE |
                                               G_PARAM_CONSTRUCT |
                                               G_PARAM_STATIC_STRINGS);

  properties[PROP_NEEDLE] = g_param_spec_string ("needle",
                                                 "Needle",
                                                 "String to search notes",
                                                 NULL,
                                                 G_PARAM_READWRITE |
                                                 G_PARAM_CONSTRUCT |
                                                 G_PARAM_STATIC_STRINGS);


  properties[PROP_MODEL] = g_param_spec_object ("model",
                                                "Model",
                                                "The GtkTreeModel",
                                                GTK_TYPE_TREE_MODEL,
                                                G_PARAM_READABLE  |
                                                G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, NUM_PROPERTIES, properties);

}

BjbController *
bjb_controller_new (BijiNoteBook *book, 
                           gchar *needle)
{
  return g_object_new ( BJB_TYPE_CONTROLLER,
              "book", book,
              "needle", needle,
              NULL); 
}

void
bjb_controller_set_book (BjbController *self, BijiNoteBook  *book )
{
  self->priv->book = book ;
  
  /* Only update completion.
   * Notes model is updated when needle changes */
  refresh_completion(self);
}

void
bjb_controller_set_needle (BjbController *self, const gchar *needle )
{
  self->priv->needle = g_strdup(needle);
  on_needle_changed(self);
}

gchar *
bjb_controller_get_needle (BjbController *self)
{
  if (!self->priv->needle)
    return NULL;

  return self->priv->needle;
}

GtkTreeModel *
bjb_controller_get_model  (BjbController *self)
{
  return self->priv->model ;
}

GtkTreeModel *
bjb_controller_get_completion(BjbController *self)
{
  return self->priv->completion ;
}

void
bjb_controller_set_main_view (BjbController *self, GdMainView *current)
{
  /* Refresh the model */
  self->priv->cur = current;
  update_view(self);
}


