/*
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

/*
 * The SearchToolbar displays an entry when text is inserted.
 * 
 * BjbController is updated accordingly and makes note to be
 * displayed or not.
 */

#include "config.h"

#include <clutter-gtk/clutter-gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "bjb-controller.h"
#include "bjb-search-toolbar.h"

enum
{
  PROP_0,
  PROP_WINDOW,
  PROP_ACTOR,
  PROP_CONTROLLER,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

struct _BjbSearchToolbarPrivate
{
  GtkWidget         *search_entry;
  gchar             *needle;
  GtkEntryBuffer    *entry_buf;
  GtkTreeModel      *completion_model;
  BjbController     *controller;

  /* A pressed key shows search entry */
  gulong            key_pressed;
  gulong            key_released;

  /* Misc UI  */
  GtkWidget         *window;
  GtkWidget         *widget;
  ClutterActor      *actor;
  ClutterActor      *parent_actor;
  ClutterConstraint *width_constraint;
};

G_DEFINE_TYPE (BjbSearchToolbar, bjb_search_toolbar, G_TYPE_OBJECT);

static void
bjb_search_toolbar_fade_in (BjbSearchToolbar *self)
{
  BjbSearchToolbarPrivate *priv = self->priv;
  guint8 opacity;

  opacity = clutter_actor_get_opacity (priv->actor);
  
  if (opacity != 0)
    return;

  clutter_actor_set_opacity (priv->actor, 255);
  clutter_actor_show (priv->actor);

  gtk_widget_grab_focus (priv->search_entry);
}

static void
bjb_search_toolbar_fade_out (BjbSearchToolbar *self)
{
  BjbSearchToolbarPrivate *priv = self->priv;

  clutter_actor_set_opacity (priv->actor, 0);
  clutter_actor_hide (priv->actor);
  gtk_entry_set_text (GTK_ENTRY (priv->search_entry),"");
}

/* If some text has been input, handle position */
static gboolean
on_key_released (GtkWidget *widget,GdkEvent  *event,gpointer user_data)
{
  BjbSearchToolbar *self = BJB_SEARCH_TOOLBAR (user_data);
  BjbSearchToolbarPrivate *priv = self->priv;

  if (clutter_actor_get_opacity (priv->actor) != 0)
    gtk_editable_set_position (GTK_EDITABLE (priv->search_entry),-1);

  return FALSE;
}

static gboolean
on_key_pressed (GtkWidget *widget,GdkEvent  *event,gpointer user_data)
{
  BjbSearchToolbar *self = BJB_SEARCH_TOOLBAR (user_data);

  /* Reveal the entry is text is input. TODO add more keys not input*/
  if (clutter_actor_get_opacity (self->priv->actor) == 0)
  {
    switch (event->key.keyval)
    {
      case GDK_KEY_Control_L :
      case GDK_KEY_Control_R :
      case GDK_KEY_Shift_L :
      case GDK_KEY_Shift_R :
      case GDK_KEY_Alt_L :
      case GDK_KEY_Alt_R :
      case GDK_KEY_Escape :
        return TRUE ;

      default:
        bjb_search_toolbar_fade_in (self);
        return FALSE;
    }
  }

  /* If there is already an entry and escape pressed, hide entry */
  else
  {
    if (event->key.keyval == GDK_KEY_Escape)
    {
      bjb_search_toolbar_fade_out (self);
      return TRUE;
    }
  }

  return FALSE;
}

static void
bjb_search_toolbar_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  BjbSearchToolbar *self = BJB_SEARCH_TOOLBAR (object);

  switch (property_id)
  {
    case PROP_ACTOR:
      g_value_set_object (value, self->priv->parent_actor);
      break;
    case PROP_WINDOW:
      g_value_set_object (value, self->priv->window);
      break;
    case PROP_CONTROLLER:
      g_value_set_object(value, self->priv->controller);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
bjb_search_toolbar_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  BjbSearchToolbar *self = BJB_SEARCH_TOOLBAR (object);

  switch (property_id)
  {
    case PROP_WINDOW:
      self->priv->window = g_value_get_object (value);
      break;
    case PROP_ACTOR:
      self->priv->parent_actor = g_value_get_object (value);
      break;
    case PROP_CONTROLLER:
      self->priv->controller = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
clear_search_entry_callback(GtkEntry *entry,GtkEntryIconPosition icon_pos,
                            GdkEvent *event,BjbController *controller)
{
  gtk_entry_set_text(entry,"");
  bjb_controller_set_needle(controller,"");
}

static void
action_search_entry(GtkEntry *entry,BjbController *controller)
{
  bjb_controller_set_needle (controller, gtk_entry_get_text(entry));
}

static void
action_entry_insert_callback(GtkEntryBuffer *buffer,guint position,
                             gchar *chars,guint n_chars,BjbSearchToolbar *self)
{
  action_search_entry(GTK_ENTRY(self->priv->search_entry),
                      self->priv->controller);
}

static void
action_entry_delete_callback(GtkEntryBuffer *buffer,guint position,
                             guint n_chars,BjbSearchToolbar *self)
{ 
  action_search_entry(GTK_ENTRY(self->priv->search_entry),
                      self->priv->controller);
}

static void
bjb_search_toolbar_finalize (GObject *obj)
{
  BjbSearchToolbar *self = BJB_SEARCH_TOOLBAR (obj);
  BjbSearchToolbarPrivate *priv = self->priv ;
  
  g_signal_handler_disconnect (priv->window,priv->key_pressed);
  priv->key_pressed = 0;

  g_signal_handler_disconnect (priv->window,priv->key_released);
  priv->key_released = 0;

  G_OBJECT_CLASS (bjb_search_toolbar_parent_class)->finalize (obj);
}

static void
bjb_search_toolbar_constructed (GObject *obj)
{
  GtkEntryBuffer          *entry_buf;
  GtkEntryCompletion      *completion ;
  BjbSearchToolbar        *self = BJB_SEARCH_TOOLBAR(obj);
  BjbSearchToolbarPrivate *priv = self->priv ;

  G_OBJECT_CLASS (bjb_search_toolbar_parent_class)->constructed (obj);

  /* Get the needle from controller */
  priv->needle = bjb_controller_get_needle(priv->controller);

  /* Comletion model for buffer */
  completion = gtk_entry_completion_new ();
  gtk_entry_set_completion (GTK_ENTRY(priv->search_entry), completion);
  g_object_unref (completion);
  priv->completion_model = bjb_controller_get_completion(priv->controller);

  gtk_entry_completion_set_model (completion, priv->completion_model);  
  gtk_entry_completion_set_text_column (completion, 0);

  /* Connect to set the text */
  priv->key_pressed = g_signal_connect(priv->window,"key-press-event",
                                       G_CALLBACK(on_key_pressed),self);

  priv->key_released = g_signal_connect(priv->window,"key-release-event",
                                       G_CALLBACK(on_key_released),self);

  /* Connect to set the notes */
  g_signal_connect (priv->search_entry, "icon-press",
                    G_CALLBACK (clear_search_entry_callback),
                    self->priv->controller);

  entry_buf = gtk_entry_get_buffer(GTK_ENTRY(priv->search_entry));

  g_signal_connect(entry_buf,"inserted-text",
                   G_CALLBACK(action_entry_insert_callback),self);
  g_signal_connect(entry_buf,"deleted-text",
                   G_CALLBACK(action_entry_delete_callback),self);

  /* Constraints */
  priv->width_constraint = clutter_bind_constraint_new (priv->parent_actor,
                                                        CLUTTER_BIND_WIDTH,
                                                        0.0); 
  clutter_actor_add_constraint (priv->actor, priv->width_constraint);

  if (priv->needle && g_strcmp0 (priv->needle, "") !=0)
  { 
    gtk_entry_set_text (GTK_ENTRY (priv->search_entry), priv->needle);
    bjb_search_toolbar_fade_in (self);
    gtk_editable_set_position (GTK_EDITABLE(self->priv->search_entry),-1);
  }
}

static void
bjb_search_toolbar_init (BjbSearchToolbar *self)
{
  BjbSearchToolbarPrivate    *priv;
  GtkStyleContext            *context;
  GtkToolItem                *separator;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BJB_TYPE_SEARCH_TOOLBAR, BjbSearchToolbarPrivate);
  priv = self->priv;

  priv->widget = gtk_toolbar_new ();
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (priv->widget), FALSE);
  gtk_toolbar_set_icon_size (GTK_TOOLBAR (priv->widget), GTK_ICON_SIZE_LARGE_TOOLBAR);

  separator = gtk_tool_item_new ();
  gtk_tool_item_set_expand (separator,TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR(priv->widget),separator,-1);

  priv->search_entry = gtk_search_entry_new ();
  gtk_entry_set_icon_from_stock (GTK_ENTRY(priv->search_entry),
                                 GTK_ENTRY_ICON_SECONDARY,
                                 GTK_STOCK_CLEAR);
  gtk_entry_set_text (GTK_ENTRY (priv->search_entry),"");

  GtkToolItem *entry_item ;
  entry_item = gtk_tool_item_new();
  gtk_container_add(GTK_CONTAINER(entry_item),priv->search_entry);
  gtk_toolbar_insert(GTK_TOOLBAR(priv->widget),entry_item,-1);
  gtk_tool_item_set_expand(entry_item,TRUE);

  separator = gtk_tool_item_new ();
  gtk_tool_item_set_expand (separator,TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR(priv->widget),separator,-1);

  context = gtk_widget_get_style_context (priv->widget);
  gtk_style_context_add_class (context,"primary-toolbar");

  priv->actor = gtk_clutter_actor_new_with_contents (priv->widget);
  clutter_actor_set_opacity (priv->actor, 0);
  g_object_set (priv->actor, "show-on-set-parent", FALSE, NULL);

  gtk_widget_show_all (priv->widget);
}

static void
bjb_search_toolbar_class_init (BjbSearchToolbarClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->get_property = bjb_search_toolbar_get_property ;
  object_class->set_property = bjb_search_toolbar_set_property ;
  object_class->constructed = bjb_search_toolbar_constructed ;
  object_class->finalize = bjb_search_toolbar_finalize ;

  properties[PROP_WINDOW] = g_param_spec_object ("window",
                                                 "Window",
                                                 "Window",
                                                 GTK_TYPE_WIDGET,
                                                 G_PARAM_READWRITE |
                                                 G_PARAM_CONSTRUCT |
                                                 G_PARAM_STATIC_STRINGS);

  properties[PROP_ACTOR] = g_param_spec_object ("actor",
                                                "Actor",
                                                "ParentActor",
                                                CLUTTER_TYPE_ACTOR,
                                                G_PARAM_READWRITE |
                                                G_PARAM_CONSTRUCT |
                                                G_PARAM_STATIC_STRINGS);

  properties[PROP_CONTROLLER] = g_param_spec_object ("controller",
                                                     "Controller",
                                                     "Controller",
                                                     BJB_TYPE_CONTROLLER,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT |
                                                     G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, NUM_PROPERTIES, properties);
  g_type_class_add_private (class, sizeof (BjbSearchToolbarPrivate));
}

BjbSearchToolbar *
bjb_search_toolbar_new (GtkWidget     *window,
                        ClutterActor  *parent_actor,
                        BjbController *controller)
{
  return g_object_new (BJB_TYPE_SEARCH_TOOLBAR,
                       "window",window,
                       "actor",parent_actor,
                       "controller",controller,
                       NULL);
}

ClutterActor *
bjb_search_toolbar_get_actor (BjbSearchToolbar *self)
{
  return self->priv->actor;
}
