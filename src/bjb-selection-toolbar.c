/*
 * Copyright © 2012 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */


#include "config.h"

#include <clutter-gtk/clutter-gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libgd/gd.h>

#include "bjb-main-view.h"
#include "bjb-selection-toolbar.h"

enum
{
  PROP_0,
  PROP_ACTOR,
  PROP_BJB_SELECTION,
  PROP_BJB_MAIN_VIEW,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

struct _BjbSelectionToolbarPrivate
{
  GtkWidget          *toolbar_trash;
  GtkWidget          *toolbar_color;
  GtkWidget          *toolbar_tag;

  /* sure */
  BjbMainView        *view ;
  ClutterActor       *actor ;
  GtkWidget          *widget ;
  GdMainView         *selection ;
  ClutterActor       *parent_actor;
  ClutterConstraint  *width_constraint ;

  /* misc gtk */
  GtkToolItem        *left_group;
  GtkToolItem        *right_group;
  GtkToolItem        *separator;
  GtkWidget          *left_box;
  GtkWidget          *right_box;
};

G_DEFINE_TYPE (BjbSelectionToolbar, bjb_selection_toolbar, G_TYPE_OBJECT);

static void
bjb_selection_toolbar_fade_in (BjbSelectionToolbar *self)
{
  BjbSelectionToolbarPrivate *priv = self->priv;
  guint8 opacity;

  opacity = clutter_actor_get_opacity (priv->actor);

  if (opacity != 0)
    return;

  clutter_actor_set_opacity (priv->actor, 255);
}


static void
bjb_selection_toolbar_fade_out (BjbSelectionToolbar *self)
{
  clutter_actor_set_opacity (self->priv->actor, 0);
}

static void
bjb_selection_toolbar_set_item_visibility (BjbSelectionToolbar *self)
{
  BjbSelectionToolbarPrivate *priv = self->priv;

  gtk_widget_set_visible (priv->toolbar_trash, TRUE);
}

static void
bjb_selection_toolbar_selection_changed (GdMainView *view, gpointer user_data)
{
  
  BjbSelectionToolbar *self = BJB_SELECTION_TOOLBAR (user_data);
  GList *selection;

  selection = gd_main_view_get_selection(view);

  if (g_list_length (selection) > 0)
  {
    bjb_selection_toolbar_set_item_visibility (self);
    bjb_selection_toolbar_fade_in (self);
  }
  
  else
    bjb_selection_toolbar_fade_out (self);
}

static void
bjb_selection_toolbar_dispose (GObject *object)
{
  BjbSelectionToolbar *self = BJB_SELECTION_TOOLBAR (object);
  BjbSelectionToolbarPrivate *priv = self->priv;

  gtk_widget_destroy (priv->widget);

  G_OBJECT_CLASS (bjb_selection_toolbar_parent_class)->dispose (object);
}


static void
bjb_selection_toolbar_init (BjbSelectionToolbar *self)
{
  BjbSelectionToolbarPrivate *priv;
  GtkWidget                  *bin;
  GtkWidget                  *image;
  GtkStyleContext            *context;
  GdkRGBA                     color = {0.0, 0.0, 0.0, 0.0};

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BJB_TYPE_SELECTION_TOOLBAR, BjbSelectionToolbarPrivate);
  priv = self->priv;

  priv->widget = gtk_toolbar_new ();
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (priv->widget), FALSE);
  gtk_toolbar_set_icon_size (GTK_TOOLBAR (priv->widget), GTK_ICON_SIZE_LARGE_TOOLBAR);
  context = gtk_widget_get_style_context (priv->widget);
  gtk_style_context_add_class (context, "osd");

  priv->actor = gtk_clutter_actor_new_with_contents (priv->widget);
  clutter_actor_set_opacity (priv->actor, 0);
  g_object_set (priv->actor, "show-on-set-parent", FALSE, NULL);

  clutter_actor_set_easing_mode (priv->actor, CLUTTER_EASE_IN_QUAD);
  clutter_actor_set_easing_duration (priv->actor, 300.0);

  clutter_actor_set_easing_mode (priv->actor, CLUTTER_EASE_OUT_QUAD);
  clutter_actor_set_easing_duration (priv->actor, 300.0);

  bin = gtk_clutter_actor_get_widget (GTK_CLUTTER_ACTOR (priv->actor));
  gtk_widget_override_background_color (bin,
                                        GTK_STATE_FLAG_NORMAL,
                                        &color);

  priv->left_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  priv->left_group = gtk_tool_item_new ();
  gtk_container_add (GTK_CONTAINER (priv->left_group), priv->left_box);
  gtk_toolbar_insert (GTK_TOOLBAR (priv->widget), priv->left_group, -1);
  gtk_widget_show_all (GTK_WIDGET (priv->left_group));

  /* Notes tags */
  priv->toolbar_tag = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("emblem-documents-symbolic", GTK_ICON_SIZE_INVALID);
  gtk_image_set_pixel_size (GTK_IMAGE (image), 32);
  gtk_container_add (GTK_CONTAINER (priv->toolbar_tag), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (priv->toolbar_tag), _("Tag"));
  gtk_container_add (GTK_CONTAINER (priv->left_box), priv->toolbar_tag);

  /* Notes color */
  priv->toolbar_color = gtk_color_button_new ();
  gtk_container_add (GTK_CONTAINER (priv->left_box), priv->toolbar_color);

  priv->separator = gtk_separator_tool_item_new ();
  gtk_separator_tool_item_set_draw (GTK_SEPARATOR_TOOL_ITEM (priv->separator), FALSE);
  gtk_widget_set_visible (GTK_WIDGET (priv->separator), TRUE);
  gtk_tool_item_set_expand (priv->separator, TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR (priv->widget), priv->separator, -1);

  priv->right_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  priv->right_group = gtk_tool_item_new ();
  gtk_container_add (GTK_CONTAINER (priv->right_group), priv->right_box);
  gtk_toolbar_insert (GTK_TOOLBAR (priv->widget), priv->right_group, -1);
  gtk_widget_show_all (GTK_WIDGET (priv->right_group));

  /* Trash notes */
  priv->toolbar_trash = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("user-trash-symbolic", GTK_ICON_SIZE_INVALID);
  gtk_image_set_pixel_size (GTK_IMAGE (image), 32);
  gtk_container_add (GTK_CONTAINER (priv->toolbar_trash), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (priv->toolbar_trash), _("Delete"));
  gtk_container_add (GTK_CONTAINER (priv->right_box), priv->toolbar_trash);

  gtk_widget_show_all (priv->widget);
  clutter_actor_show(priv->actor);
}

static void
bjb_selection_toolbar_get_property (GObject  *object,
                                    guint     property_id,
                                    GValue   *value,
                                    GParamSpec *pspec)
{
  BjbSelectionToolbar *self = BJB_SELECTION_TOOLBAR (object);

  switch (property_id)
  {
    case PROP_ACTOR:
      g_value_set_object (value, self->priv->actor);
      break;
    case PROP_BJB_SELECTION:
      g_value_set_object(value, self->priv->selection);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
bjb_selection_toolbar_set_property (GObject  *object,
                                    guint     property_id,
                                    const GValue *value,
                                    GParamSpec *pspec)
{
  BjbSelectionToolbar *self = BJB_SELECTION_TOOLBAR (object);

  switch (property_id)
  {
    case PROP_ACTOR:
      self->priv->parent_actor = g_value_get_object (value);
      break;
    case PROP_BJB_SELECTION:
      self->priv->selection = g_value_get_object (value);
      break;
    case PROP_BJB_MAIN_VIEW:
      self->priv->view = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
bjb_selection_toolbar_notify_width (GObject *object,
                                    GParamSpec *pspec,
                                    gpointer user_data)
{
  BjbSelectionToolbar *self = BJB_SELECTION_TOOLBAR (user_data);
  BjbSelectionToolbarPrivate *priv = self->priv;
  
  gfloat offset = 300.0;
  gfloat width;

  width = clutter_actor_get_width (priv->parent_actor);
  if (width > 1000)
    offset += (width - 1000);
  else if (width < 600)
    offset -= (600 - width);

  clutter_bind_constraint_set_offset (CLUTTER_BIND_CONSTRAINT (priv->width_constraint), -1 * offset);
}

static void
bjb_selection_toolbar_constructed(GObject *obj)
{
  ClutterConstraint *constraint ;
  BjbSelectionToolbar *self = BJB_SELECTION_TOOLBAR(obj);
  BjbSelectionToolbarPrivate *priv = self->priv ;
  
  G_OBJECT_CLASS (bjb_selection_toolbar_parent_class)->constructed (obj);

  /* item(s) selected --> fade in */
  g_signal_connect (self->priv->selection,
                    "view-selection-changed", 
                    G_CALLBACK(bjb_selection_toolbar_selection_changed),
                    self);

  g_signal_connect(priv->toolbar_color,"color-set",
                   G_CALLBACK(action_color_selected_notes),priv->view);

  g_signal_connect(priv->toolbar_tag,"clicked",
                   G_CALLBACK(action_tag_selected_notes),priv->view);

  g_signal_connect(priv->toolbar_trash,"clicked",
                   G_CALLBACK(action_delete_selected_notes),priv->view);

  priv->width_constraint = clutter_bind_constraint_new (priv->parent_actor, CLUTTER_BIND_WIDTH, -300.0); 
  clutter_actor_add_constraint (priv->actor, priv->width_constraint);
  
  g_signal_connect (priv->actor,
                    "notify::width",
                    G_CALLBACK (bjb_selection_toolbar_notify_width),
                    self);

  constraint = clutter_align_constraint_new (priv->parent_actor, CLUTTER_ALIGN_X_AXIS, 0.50);
  clutter_actor_add_constraint (priv->actor, constraint);

  constraint = clutter_align_constraint_new (priv->parent_actor, CLUTTER_ALIGN_Y_AXIS, 0.95);
  clutter_actor_add_constraint (priv->actor, constraint);
}

static void
bjb_selection_toolbar_class_init (BjbSelectionToolbarClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->dispose = bjb_selection_toolbar_dispose;
  object_class->get_property = bjb_selection_toolbar_get_property ;
  object_class->set_property = bjb_selection_toolbar_set_property ;
  object_class->constructed = bjb_selection_toolbar_constructed ;

  properties[PROP_ACTOR] = g_param_spec_object ("actor",
                                                "Actor",
                                                "ParentActor",
                                                CLUTTER_TYPE_ACTOR,
                                                G_PARAM_READWRITE |
                                                G_PARAM_CONSTRUCT |
                                                G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_ACTOR,properties[PROP_ACTOR]);

  properties[PROP_BJB_SELECTION] = g_param_spec_object ("selection",
                                                        "Selection",
                                                        "SelectionController",
                                                        GD_TYPE_MAIN_VIEW,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_BJB_SELECTION,properties[PROP_BJB_SELECTION]);

  properties[PROP_BJB_MAIN_VIEW] = g_param_spec_object ("bjbmainview",
                                                        "Bjbmainview",
                                                        "BjbMainView",
                                                        BJB_TYPE_MAIN_VIEW,
                                                        G_PARAM_READWRITE  |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_BJB_MAIN_VIEW,properties[PROP_BJB_MAIN_VIEW]);


  g_type_class_add_private (class, sizeof (BjbSelectionToolbarPrivate));
}


BjbSelectionToolbar *
bjb_selection_toolbar_new (ClutterActor *parent_actor,
                           GdMainView   *selection,
                           BjbMainView  *bjb_main_view)
{
  return g_object_new (BJB_TYPE_SELECTION_TOOLBAR,
                       "actor", parent_actor, 
                       "selection", selection,
                       "bjbmainview",bjb_main_view,
                       NULL);
}


ClutterActor *
bjb_selection_toolbar_get_actor (BjbSelectionToolbar *self)
{
  return self->priv->actor;
}
