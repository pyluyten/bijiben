/* Bijiben
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

#include "bjb-main-toolbar.h"
#include "bjb-rename-note.h"
#include "bjb-window-base.h"

/* select_mode + two different standard mode */
typedef enum
{
  BJB_TOOLBAR_0,
  BJB_TOOLBAR_STD_LIST,
  BJB_TOOLBAR_STD_ICON,
  BJB_TOOLBAR_SELECT,
  BJB_TOOLBAR_NUM
} BjbToolbarType ;

struct _BjbMainToolbarPrivate
{
  /* Widget and actor */
  ClutterActor   *actor;
  GdMainToolbar  *toolbar;

  /* Controllers */
  GdMainView     *view;
  BjbToolbarType  type;
  BjbMainView    *parent;

  /* Standard mode buttons */
  GtkWidget      *new;  
  GtkWidget      *list;
  
  GtkWidget      *grid;
  
  GtkWidget      *select;

  /* Select mode buttons */
  gulong         finish_sig;
  gulong         update_selection;
};

/* GObject properties */

enum {
  PROP_0,
  PROP_VIEW,
  PROP_PARENT,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

#define BJB_MAIN_TOOLBAR_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BJB_TYPE_MAIN_TOOLBAR, BjbMainToolbarPrivate))

G_DEFINE_TYPE (BjbMainToolbar, bjb_main_toolbar, G_TYPE_OBJECT);

/* Callbacks */

static void
on_new_note_clicked (GtkWidget *but, BjbMainView *view)
{
  gchar *title;
  BijiNoteObj *result ;
  BijiNoteBook *book ;

  title = note_title_dialog(GTK_WINDOW(bjb_main_view_get_window(view)),
                            "New Note : Title",
                            "New Note");

  if (!title)
    return;

  /* append note to collection */
  book = bjb_window_base_get_book(bjb_main_view_get_window(view));
  result = biji_note_book_get_new_note_from_string (book, title);

  /* Go to that note */
  switch_to_note_view(view,result);
}

static void populate_main_toolbar(BjbMainToolbar *self);

static void
on_selection_mode_clicked (GtkWidget *button, BjbMainToolbar *self)
{
  GtkStyleContext *context;
  GdMainView *view = self->priv->view;
  GtkWidget *widget = GTK_WIDGET(self->priv->toolbar);

  if (gd_main_view_get_selection_mode (view))
  {
    gd_main_view_set_selection_mode (view, FALSE);
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_remove_class (context, "selection-mode");
    gtk_widget_reset_style (widget);
  }

  else
  {
    gd_main_view_set_selection_mode (view, TRUE);
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_add_class (context, "selection-mode");
    gtk_widget_reset_style (widget);
  }

  populate_main_toolbar(self);
  return ;
}

static gboolean
on_view_mode_clicked (GtkWidget *button, BjbMainToolbar *self)
{
  GdMainView *view = self->priv->view ;
  GdMainViewType current = gd_main_view_get_view_type(view);
    
  switch ( current )
  {
    case GD_MAIN_VIEW_ICON :
      gd_main_view_set_view_type ( view ,GD_MAIN_VIEW_LIST );
      break ;
    case GD_MAIN_VIEW_LIST :
      gd_main_view_set_view_type ( view ,GD_MAIN_VIEW_ICON );
      break ;
    default:
      gd_main_view_set_view_type ( view ,GD_MAIN_VIEW_ICON );
  }

  bjb_main_view_update_model (self->priv->parent);
  populate_main_toolbar (self);

  return TRUE;
}

static gboolean
update_selection_label (GdMainView *view, BjbMainToolbar *self)
{
  GList *selected;
  gint length;
  gchar *label;

  selected = gd_main_view_get_selection(view);
  length = g_list_length (selected);

  if (length == 0)
    label = g_strdup("Click on items to select them");
  else
    label = g_strdup_printf ("%d selected", length);

  gd_main_toolbar_set_labels (self->priv->toolbar, NULL, label);
  g_free (label);

  return TRUE;
}

static void
populate_bar_for_selection(BjbMainToolbar *self)
{
  BjbMainToolbarPrivate *priv = self->priv;

  priv->select = gd_main_toolbar_add_button (priv->toolbar,
                                             NULL,"Done", FALSE);

  g_signal_connect (priv->select, "clicked",
                    G_CALLBACK (on_selection_mode_clicked), self);

  priv->update_selection = g_signal_connect(priv->view,
                                            "view-selection-changed",
                                            G_CALLBACK(update_selection_label),
                                            self);

  update_selection_label(priv->view,self);
}

static void
populate_bar_for_standard(BjbMainToolbar *self)
{
  BjbMainToolbarPrivate *priv = self->priv;

  /* Label */
  gd_main_toolbar_set_labels(priv->toolbar,"New and recent",NULL);

  /* New Note */
  priv->new = gd_main_toolbar_add_button(priv->toolbar,
                                         NULL,
                                         "New",
                                         TRUE);

  g_signal_connect(priv->new,"clicked",
                   G_CALLBACK(on_new_note_clicked),priv->parent);

  /* Go to selection mode */
  priv->select = gd_main_toolbar_add_button(priv->toolbar,
                                            "emblem-default-symbolic",
                                            NULL,
                                            FALSE);

  g_signal_connect (priv->select,"clicked",
                    G_CALLBACK(on_selection_mode_clicked),self);
}

static void
populate_bar_for_icon_view(BjbMainToolbar *self)
{
  BjbMainToolbarPrivate *priv = self->priv;

  /* Switch to list */
  priv->list= gd_main_toolbar_add_button(priv->toolbar,
                                         "view-list-symbolic",
                                         NULL,
                                         FALSE);

  g_signal_connect (priv->list, "clicked",
                    G_CALLBACK(on_view_mode_clicked),self);

  populate_bar_for_standard(self);
}

static void
populate_bar_for_list_view(BjbMainToolbar *self)
{
  BjbMainToolbarPrivate *priv = self->priv;

  /* Switch to icon view */
  priv->grid = gd_main_toolbar_add_button(priv->toolbar,
                                          "view-grid-symbolic",
                                          NULL,
                                          FALSE);

  g_signal_connect (priv->grid, "clicked",
                    G_CALLBACK(on_view_mode_clicked),self);

  populate_bar_for_standard(self);
}

static void
populate_bar_switch(BjbMainToolbar *self)
{
  switch (self->priv->type)
  {
    case BJB_TOOLBAR_SELECT:
      populate_bar_for_selection(self);
      break;

    case BJB_TOOLBAR_STD_ICON:
      populate_bar_for_icon_view(self);
      break;

    case BJB_TOOLBAR_STD_LIST:
      populate_bar_for_list_view(self);
      break;

    default:
      g_warning("Main Toolbar implementation is erroneous.\
                 Please fill in a bug report");
  }

  gtk_widget_show_all(GTK_WIDGET(self->priv->toolbar));
}

static void
populate_main_toolbar(BjbMainToolbar *self)
{
  BjbMainToolbarPrivate *priv = self->priv;
  BjbToolbarType to_be = BJB_TOOLBAR_0 ;

  if (gd_main_view_get_selection_mode(priv->view) == TRUE)
    to_be = BJB_TOOLBAR_SELECT;

  else if (gd_main_view_get_view_type(priv->view) == GD_MAIN_VIEW_ICON)
    to_be = BJB_TOOLBAR_STD_ICON;

  else if (gd_main_view_get_view_type(priv->view) == GD_MAIN_VIEW_LIST)
    to_be = BJB_TOOLBAR_STD_LIST;
  
  /* Simply clear then populate */
  if (to_be != priv->type)
  {
    priv->type = to_be;
    gd_main_toolbar_clear (priv->toolbar);
    populate_bar_switch (self);
  }
}

static void
bjb_main_toolbar_constructed (GObject *obj)
{
  G_OBJECT_CLASS(bjb_main_toolbar_parent_class)->constructed(obj);
}

static void
bjb_main_toolbar_init (BjbMainToolbar *self)
{
  BjbMainToolbarPrivate *priv;
  
  priv = BJB_MAIN_TOOLBAR_GET_PRIVATE(self);
  self->priv = priv ;

  priv->type = BJB_TOOLBAR_0 ;
  priv->toolbar = GD_MAIN_TOOLBAR(gd_main_toolbar_new());
  priv->actor = gtk_clutter_actor_new_with_contents(GTK_WIDGET(priv->toolbar));

  clutter_actor_show(priv->actor);
}

static void
bjb_main_toolbar_finalize (GObject *object)
{
  BjbMainToolbar *self = BJB_MAIN_TOOLBAR(object);

  gtk_widget_destroy (GTK_WIDGET (self->priv->toolbar));

  /* chain up */
  G_OBJECT_CLASS (bjb_main_toolbar_parent_class)->finalize (object);
}

static void
bjb_main_toolbar_get_property (GObject     *object,
                               guint       property_id,
                               GValue      *value,
                               GParamSpec  *pspec)
{
  BjbMainToolbar *self = BJB_MAIN_TOOLBAR (object);

  switch (property_id)
    {
    case PROP_VIEW:
      g_value_set_object (value, self->priv->view);
      break;
    case PROP_PARENT:
      g_value_set_object (value, self->priv->parent);
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
bjb_main_toolbar_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  BjbMainToolbar *self = BJB_MAIN_TOOLBAR (object);

  switch (property_id)
    {
    case PROP_VIEW:
      bjb_main_toolbar_set_view(self,g_value_get_object(value));
      break;
    case PROP_PARENT:
      self->priv->parent = g_value_get_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
bjb_main_toolbar_class_init (BjbMainToolbarClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (BjbMainToolbarPrivate));

  object_class->get_property = bjb_main_toolbar_get_property;
  object_class->set_property = bjb_main_toolbar_set_property;
  object_class->constructed = bjb_main_toolbar_constructed;
  object_class->finalize = bjb_main_toolbar_finalize;

  properties[PROP_VIEW] = g_param_spec_object ("view",
                                               "View",
                                               "View",
                                               GD_TYPE_MAIN_VIEW,
                                               G_PARAM_READWRITE |
                                               G_PARAM_CONSTRUCT |
                                               G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, 
                                   PROP_VIEW, 
                                   properties[PROP_VIEW]);

  properties[PROP_PARENT] = g_param_spec_object ("parent",
                                                 "Parent",
                                                 "Parent",
                                                 BJB_TYPE_MAIN_VIEW,
                                                 G_PARAM_READWRITE |
                                                 G_PARAM_CONSTRUCT |
                                                 G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, 
                                   PROP_PARENT, 
                                   properties[PROP_PARENT]);
}

BjbMainToolbar *
bjb_main_toolbar_new (GdMainView *view, BjbMainView *parent)
{
  return g_object_new (BJB_TYPE_MAIN_TOOLBAR,
                       "parent",parent,
                       "view",view,
                       NULL);
}

void
bjb_main_toolbar_set_view (BjbMainToolbar *self, GdMainView *view)
{
  /* populate the toolbar */
  self->priv->view = view ;
  populate_main_toolbar(self);
}

ClutterActor *
bjb_main_toolbar_get_actor (BjbMainToolbar *self)
{
  return self->priv->actor;
}
