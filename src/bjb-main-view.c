#include <glib/gprintf.h>
#include <libbiji/libbiji.h>
#include <libgd/gd-main-view.h>

#include "utils/bjb-icons-colors.h"

#include "bjb-app-menu.h"
#include "bjb-bijiben.h"
#include "bjb-controller.h"
#include "bjb-main-toolbar.h"
#include "bjb-main-view.h"
#include "bjb-note-view.h"
#include "bjb-rename-note.h"
#include "bjb-search-toolbar.h"
#include "bjb-selection-toolbar.h"
#include "bjb-tags-view.h"
#include "bjb-tracker.h"
#include "bjb-window-base.h"

#define DEFAULT_VIEW GD_MAIN_VIEW_ICON

enum
{
  PROP_0,
  PROP_WINDOW,
  PROP_BJB_CONTROLLER,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

/************************** Gobject ***************************/

struct _BjbMainViewPriv {  
  GtkWidget        *window;
  GtkWidget        *label;
  ClutterActor     *bin;
  ClutterActor     *embed ;
  ClutterActor     *content;

  /* Toolbar */
  BjbMainToolbar   *main_toolbar;
  ClutterActor     *toolbar_actor;

  /* Selection Mode */
  ClutterActor     *actions ;

  /* Search Entry  */
  ClutterActor     *search_actor;
  BjbSearchToolbar *search_bar;

  /* View Notes , model */
  /* TODO : controller is app-wide controller.
   * add one window specific controller */
  GdMainView       *view ; 
  BjbController    *controller ;

  /* signals */
  gulong            notes_changed ;
};

G_DEFINE_TYPE (BjbMainView, bjb_main_view, CLUTTER_TYPE_ACTOR);

static void
bjb_main_view_init (BjbMainView *object)
{
  object->priv = 
  G_TYPE_INSTANCE_GET_PRIVATE(object,BJB_TYPE_MAIN_VIEW,BjbMainViewPriv);
}

static void
bjb_main_view_finalize (GObject *object)
{
  BjbMainView     *self = BJB_MAIN_VIEW(object) ;
  BjbMainViewPriv *priv = self->priv;

  /* Signals */
  g_signal_handler_disconnect(bjb_window_base_get_book(priv->window),
                              priv->notes_changed);

  /* Widgets, actors */
  clutter_actor_destroy (priv->bin);
  g_object_unref (priv->search_bar);

  G_OBJECT_CLASS (bjb_main_view_parent_class)->finalize (object);
}

static void
bjb_main_view_set_controller ( BjbMainView *self, BjbController *controller)
{
  self->priv->controller = controller ;
}

static void
bjb_main_view_get_property ( GObject      *object,
                             guint        prop_id,
                             GValue       *value,
                             GParamSpec   *pspec)
{
  BjbMainView *self = BJB_MAIN_VIEW (object);

  switch (prop_id)
  {
    case PROP_WINDOW:
      g_value_set_object (value, self->priv->window);
      break;
    case PROP_BJB_CONTROLLER:
      g_value_set_object (value, self->priv->controller);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
bjb_main_view_set_property ( GObject        *object,
                             guint          prop_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
  BjbMainView *self = BJB_MAIN_VIEW (object);

  switch (prop_id)
  {
    case PROP_WINDOW:
      self->priv->window = g_value_get_object(value);
      break;
    case PROP_BJB_CONTROLLER:
      bjb_main_view_set_controller(self,g_value_get_object(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GObject *
biji_main_view_constructor (GType                  gtype,
                            guint                  n_properties,
                            GObjectConstructParam  *properties)
{
  GObject *obj;
  {
    obj = G_OBJECT_CLASS (bjb_main_view_parent_class)->constructor (gtype, n_properties, properties);
  }
  return obj;
}

/* Callbacks */

void
switch_to_note_view(BjbMainView *self,BijiNoteObj *note)
{
  GtkWidget *window = self->priv->window;

  g_object_unref (self);
  bjb_note_view_new (window,note);
}

static void
show_window_if_title_same(GtkWindow *window, BijiNoteObj *to_open)
{
  if ( g_strcmp0 (gtk_window_get_title (window),
                  biji_note_get_title(to_open)) == 0 )
  {
    gtk_window_present(window);
  }
}

static void
switch_to_note(BjbMainView *view, BijiNoteObj *to_open)
{
  // If the note is already opened in another window, just show it.
  if ( biji_note_obj_is_opened(to_open) )
  {
    GList *notes ;

    notes = gtk_application_get_windows 
                    (GTK_APPLICATION
                              (bjb_window_base_get_app(view->priv->window)));
    g_list_foreach (notes,(GFunc)show_window_if_title_same,to_open);
    return ;
  }

  // Otherwise, leave main view to show this note into current window.
  switch_to_note_view(view,to_open);
}

static GList *
get_selected_paths(BjbMainView *self)
{
  return gd_main_view_get_selection ( self->priv->view ) ; 
}

static gchar *
get_note_url_from_tree_path(GtkTreePath *path, BjbMainView *self)
{
  GtkTreeIter iter ;
  gchar *note_path ;
  GtkTreeModel *model ;

  model = bjb_controller_get_model(self->priv->controller);  
  gtk_tree_model_get_iter (model,&iter, path);
  gtk_tree_model_get (model, &iter,GD_MAIN_COLUMN_URI, &note_path,-1);

  return note_path ;
}

void
action_delete_selected_notes(GtkWidget *w,BjbMainView *view)
{
  GList *notes = NULL ;
  gint i ;

  g_return_if_fail(GTK_IS_WIDGET(w));
  g_return_if_fail(BJB_IS_MAIN_VIEW(view));

  /*  GtkTreePath */
  GList *paths = get_selected_paths(view);

  for ( i=0 ;  i < g_list_length (paths) ; i++ )
  {
    gchar *url = get_note_url_from_tree_path(g_list_nth_data(paths,i),
                                             view) ;

    notes = g_list_append(notes,
                          note_book_get_note_at_path
                          (bjb_window_base_get_book(view->priv->window),url));
    
  } 

  for (i=0 ; i<g_list_length(notes) ;i++ )
  {
    BijiNoteObj *note = g_list_nth_data(notes,i) ;
      
    biji_note_delete_from_tracker(note);
    biji_note_book_remove_note(bjb_window_base_get_book(view->priv->window),note);
  }
}

static gboolean
on_item_activated(GdMainView        * gd, 
                  const gchar       * id,
                  const GtkTreePath * path,
                  BjbMainView       * view)
{
  BijiNoteBook * book ;
  BijiNoteObj  * to_open ;
  GtkTreeIter    iter ;
  gchar        * note_path ;
  GtkTreeModel * model ;

  /* Get Note Path */
  model = gd_main_view_get_model(gd);
  gtk_tree_model_get_iter (model,&iter, (GtkTreePath*) path);
  gtk_tree_model_get (model, &iter,COL_URI, &note_path,-1);

  /* Switch to that note */
  book = bjb_window_base_get_book(view->priv->window); 
  to_open = note_book_get_note_at_path(book,note_path) ;
  switch_to_note(view,to_open); 

  return FALSE ;
}

static void
bjb_main_view_constructed(GObject *o)
{
  BjbMainView          *self;
  BjbMainViewPriv      *priv;
  ClutterActor         *stage, *top, *view, *selection_bar;
  ClutterLayoutManager *filler, *packer, *switcher, *overlay;
  ClutterConstraint    *constraint ;
  BjbSelectionToolbar  *panel ;

  G_OBJECT_CLASS (bjb_main_view_parent_class)->constructed(G_OBJECT(o));

  self = BJB_MAIN_VIEW(o);
  priv = self->priv ;
  stage = bjb_window_base_get_stage (BJB_WINDOW_BASE(priv->window));

  priv->view = gd_main_view_new (DEFAULT_VIEW);
  bjb_controller_set_main_view (priv->controller, priv->view);

  /* Probably move this to window_base or delete this */
  filler = clutter_bin_layout_new (CLUTTER_BIN_ALIGNMENT_CENTER,
                                   CLUTTER_BIN_ALIGNMENT_CENTER);
  priv->bin = clutter_actor_new();
  clutter_actor_set_name (priv->bin, "bin");
  clutter_actor_set_layout_manager(priv->bin,filler);
  clutter_actor_add_child(stage,priv->bin);

  constraint = clutter_bind_constraint_new (stage, CLUTTER_BIND_SIZE, 0.0);
  clutter_actor_add_constraint (priv->bin, constraint);

  packer = clutter_box_layout_new();
  clutter_box_layout_set_orientation(CLUTTER_BOX_LAYOUT(packer),
                                     CLUTTER_ORIENTATION_VERTICAL);

  priv->embed = clutter_actor_new();
  clutter_actor_set_layout_manager (priv->embed,packer) ;
  clutter_actor_add_child (priv->bin, priv->embed) ;
  clutter_actor_set_x_expand (priv->embed, TRUE);
  clutter_actor_set_y_expand (priv->embed, TRUE);

  /* Top contains main toolbar and search entry */
  switcher = clutter_box_layout_new();
  clutter_box_layout_set_orientation(CLUTTER_BOX_LAYOUT(switcher),
                                     CLUTTER_ORIENTATION_VERTICAL);

  top = clutter_actor_new();
  clutter_actor_set_layout_manager(top,switcher);

  clutter_actor_add_child(priv->embed,top);

  /* main Toolbar */
  priv->main_toolbar = bjb_main_toolbar_new(priv->view,self);
  priv->toolbar_actor = bjb_main_toolbar_get_actor(priv->main_toolbar);

  clutter_actor_add_child(top,priv->toolbar_actor);
  clutter_actor_set_x_expand (priv->toolbar_actor, TRUE);

  /* Search entry toolbar */
  priv->search_bar = bjb_search_toolbar_new(priv->window,
                                            top,
                                            priv->controller);
  priv->search_actor = bjb_search_toolbar_get_actor(priv->search_bar);

  clutter_actor_add_child(top,priv->search_actor);
  clutter_actor_set_x_expand (priv->search_actor, TRUE);

  /* Overlay contains : Notes view and selection panel  */
  overlay = clutter_bin_layout_new(CLUTTER_BIN_ALIGNMENT_CENTER,
                                   CLUTTER_BIN_ALIGNMENT_CENTER);
  priv->content = clutter_actor_new();
  clutter_actor_set_layout_manager(priv->content,overlay);

  clutter_actor_add_child(priv->embed,priv->content);
  clutter_actor_set_y_expand (priv->content, TRUE);

  /* Main view */
  view = gtk_clutter_actor_new_with_contents(GTK_WIDGET(priv->view));
  clutter_actor_add_child(priv->content,view);
  clutter_actor_set_x_expand (view, TRUE);
  clutter_actor_set_y_expand (view, TRUE);

  gd_main_view_set_selection_mode ( priv->view, FALSE);
  gd_main_view_set_model(priv->view,
                         bjb_controller_get_model(priv->controller));

  g_signal_connect(priv->view,"item-activated",
                   G_CALLBACK(on_item_activated),self);

  /* Selection Panel */
  panel = bjb_selection_toolbar_new (priv->content,priv->view,self);
  selection_bar = bjb_selection_toolbar_get_actor (panel);
  clutter_actor_add_child (priv->bin, selection_bar);

  gtk_widget_show_all (priv->window);
  gtk_window_set_title (GTK_WINDOW (priv->window),
                        BIJIBEN_MAIN_WIN_TITLE);
}

static void
bjb_main_view_class_init (BjbMainViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = bjb_main_view_finalize;
  object_class->get_property = bjb_main_view_get_property;
  object_class->set_property = bjb_main_view_set_property;
  object_class->constructor = biji_main_view_constructor;
  object_class->constructed = bjb_main_view_constructed;
    
  g_type_class_add_private (klass, sizeof (BjbMainViewPriv));
  
  properties[PROP_WINDOW] = g_param_spec_object ("window",
                                                 "Window",
                                                 "Parent Window",
                                                 GTK_TYPE_WIDGET,
                                                 G_PARAM_READWRITE |
                                                 G_PARAM_CONSTRUCT |
                                                 G_PARAM_STATIC_STRINGS);
                                                 
  g_object_class_install_property (object_class,PROP_WINDOW,properties[PROP_WINDOW]);

  properties[PROP_BJB_CONTROLLER] = g_param_spec_object ("controller",
                                                         "Controller",
                                                         "BjbController",
                                                         BJB_TYPE_CONTROLLER,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_BJB_CONTROLLER,properties[PROP_BJB_CONTROLLER]);
}

BjbMainView *
bjb_main_view_new(GtkWidget *win,
                  BjbController *controller)
{
  return g_object_new( BJB_TYPE_MAIN_VIEW,
                       "window", win,
                       "controller", controller,
                       NULL);
}

ClutterActor *
bjb_main_view_get_actor(BjbMainView *b)
{
  return b->priv->embed ;
}

GtkWidget *
bjb_main_view_get_window(BjbMainView *view)
{
  return view->priv->window ;
}

void
bjb_main_view_update_model (BjbMainView *self)
{
  g_warning ("bjb mainv iew update model");
  BjbMainViewPriv *priv = self->priv;
  
  bjb_controller_set_main_view (priv->controller,priv->view);
  gd_main_view_set_model(priv->view,bjb_controller_get_model(priv->controller));
}
