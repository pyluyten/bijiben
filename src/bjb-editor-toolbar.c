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

/* Offset for toolbar related to cursor.
 * (Pixels)
 *
 * X offset might be replaced by something like -(toolbar size/2)
 * Y offset might not be replaced                    */
#define EDITOR_TOOLBAR_X_OFFSET -120;
#define EDITOR_TOOLBAR_Y_OFFSET   30;

#include "config.h"

#include <clutter-gtk/clutter-gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libbiji/libbiji.h>

#include "bjb-bijiben.h"
#include "bjb-editor-toolbar.h"
#include "bjb-window-base.h"


enum
{
  PROP_0,
  PROP_ACTOR,
  PROP_NOTE,
  PROP_BJB_NOTE_VIEW,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

struct _BjbEditorToolbarPrivate
{
  /* Note provide us the WebKitWebView editor */
  BjbNoteView        *view;
  BijiNoteObj        *note;

  ClutterActor       *actor;
  GtkWidget          *widget;
  GtkAccelGroup      *accel;
  ClutterActor       *parent_actor;
  ClutterConstraint  *width_constraint;

  /* If user rigth-clicks we want to keep the toolbar visible
   * untill user changes his mind */
  gboolean           glued;

  /* Do not use toggle buttons. uggly there.
   * Paste : the user might want to paste overriding selected text.
   * Other : when no selection the user won't try to bold "null".*/
  GtkToolItem        *group;
  GtkWidget          *box;
  GtkWidget          *toolbar_cut;
  GtkWidget          *toolbar_copy;
  GtkWidget          *toolbar_paste;
  GtkWidget          *toolbar_bold;
  GtkWidget          *toolbar_italic;
  GtkWidget          *toolbar_strike;
  GtkWidget          *toolbar_link;
};

G_DEFINE_TYPE (BjbEditorToolbar, bjb_editor_toolbar, G_TYPE_OBJECT);

static void
bjb_editor_toolbar_fade_in (BjbEditorToolbar *self)
{
  BjbEditorToolbarPrivate *priv = self->priv;
  guint8 opacity;

  opacity = clutter_actor_get_opacity (priv->actor);

  if (opacity != 0)
    return;

  clutter_actor_set_opacity (priv->actor, 255);
}


static void
bjb_editor_toolbar_fade_out (BjbEditorToolbar *self)
{
  BjbEditorToolbarPrivate *priv = self->priv;

  clutter_actor_set_opacity (priv->actor, 0);
}

static void
bjb_editor_toolbar_init (BjbEditorToolbar *self)
{
  BjbEditorToolbarPrivate   *priv;
  GtkWidget                 *bin;
  GtkWidget                 *image;
  GdkPixbuf                 *pixbuf;
  GtkStyleContext           *context;
  GdkRGBA                    transparent = {0.0, 0.0, 0.0, 0.0};
  GdkRGBA                    black = {0.0, 0.0, 0.0, 0.6};
  gchar                     *icons_path, *full_path;
  GError                    *error = NULL;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BJB_TYPE_EDITOR_TOOLBAR, BjbEditorToolbarPrivate);
  priv = self->priv;

  priv->accel = gtk_accel_group_new ();

  priv->widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL,0);
  context = gtk_widget_get_style_context (priv->widget);
  gtk_style_context_add_class (context, "osd");
  gtk_style_context_add_class (context, "toolbar");

  priv->actor = gtk_clutter_actor_new_with_contents (priv->widget);
  clutter_actor_set_opacity (priv->actor, 0);
  g_object_set (priv->actor, "show-on-set-parent", FALSE, NULL);

  clutter_actor_set_easing_mode (priv->actor, CLUTTER_EASE_IN_QUAD);
  clutter_actor_set_easing_duration (priv->actor, 300.0);

  bin = gtk_clutter_actor_get_widget (GTK_CLUTTER_ACTOR (priv->actor));
  gtk_widget_override_background_color (bin,
                                        GTK_STATE_FLAG_NORMAL,
                                        &transparent);

  priv->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  priv->group = gtk_tool_item_new ();
  gtk_container_add (GTK_CONTAINER (priv->group), priv->box);
  gtk_container_add (GTK_CONTAINER (priv->widget), GTK_WIDGET(priv->group));
  gtk_widget_show_all (GTK_WIDGET (priv->group));

  /* Cut */
  priv->toolbar_cut = gtk_button_new_with_label ("Cut");
  gtk_container_add (GTK_CONTAINER (priv->box), priv->toolbar_cut);
  gtk_widget_override_background_color (priv->toolbar_cut,
                                        GTK_STATE_FLAG_NORMAL,
                                        &black);

  /* Copy */
  priv->toolbar_copy = gtk_button_new_with_label ("Copy");
  gtk_container_add (GTK_CONTAINER (priv->box), priv->toolbar_copy);
  gtk_widget_override_background_color (priv->toolbar_copy,
                                        GTK_STATE_FLAG_NORMAL,
                                        &black);

  /* 'n paste */
  priv->toolbar_paste = gtk_button_new_with_label ("Paste");
  gtk_container_add (GTK_CONTAINER (priv->box), priv->toolbar_paste);
  gtk_widget_override_background_color (priv->toolbar_paste,
                                        GTK_STATE_FLAG_NORMAL,
                                        &black);

  /* GtkWidget         *toolbar_bold   */
  priv->toolbar_bold = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("gtk-bold", GTK_ICON_SIZE_INVALID);
  gtk_image_set_pixel_size (GTK_IMAGE (image), 24);
  gtk_container_add (GTK_CONTAINER (priv->toolbar_bold), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (priv->toolbar_bold), _("Bold"));
  gtk_container_add (GTK_CONTAINER (priv->box), priv->toolbar_bold);
  gtk_widget_override_background_color (priv->toolbar_bold,
                                        GTK_STATE_FLAG_NORMAL,
                                        &black);

  /* GtkWidget          *toolbar_italic; */
  priv->toolbar_italic = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("gtk-italic", GTK_ICON_SIZE_INVALID);
  gtk_image_set_pixel_size (GTK_IMAGE (image), 24);
  gtk_container_add (GTK_CONTAINER (priv->toolbar_italic), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (priv->toolbar_italic), _("Italic"));
  gtk_container_add (GTK_CONTAINER (priv->box), priv->toolbar_italic);
  gtk_widget_override_background_color (priv->toolbar_italic,
                                        GTK_STATE_FLAG_NORMAL,
                                        &black);

  /* GtkWidget          *toolbar_strike; */
  priv->toolbar_strike = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("gtk-strikethrough", GTK_ICON_SIZE_INVALID);
  gtk_image_set_pixel_size (GTK_IMAGE (image), 24);
  gtk_container_add (GTK_CONTAINER (priv->toolbar_strike), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (priv->toolbar_strike), _("Strike"));
  gtk_container_add (GTK_CONTAINER (priv->box), priv->toolbar_strike);
  gtk_widget_override_background_color (priv->toolbar_strike,
                                        GTK_STATE_FLAG_NORMAL,
                                        &black);

  /* GtkWidget          *toolbar_link; */
  priv->toolbar_link = gtk_button_new ();

  icons_path = (gchar*) bijiben_get_bijiben_dir ();
  full_path = g_build_filename (icons_path,
                                "bijiben",
                                "icons",
                                "hicolor",
                                "scalable",
                                "actions",
                                "link.svg",
                                NULL);

  pixbuf = gdk_pixbuf_new_from_file (full_path, &error);
  g_free (full_path);

  if (error)
    g_warning ("error loading link icon : %s",error->message);

  image = gtk_image_new_from_pixbuf (pixbuf);
  gtk_image_set_pixel_size (GTK_IMAGE (image), 24);

  gtk_container_add (GTK_CONTAINER (priv->toolbar_link), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (priv->toolbar_link), _("Link"));
  gtk_container_add (GTK_CONTAINER (priv->box), priv->toolbar_link);
  gtk_widget_override_background_color (priv->toolbar_link,
                                        GTK_STATE_FLAG_NORMAL,
                                        &black);

  priv->glued = FALSE;
  gtk_widget_show_all (GTK_WIDGET(priv->group));
  clutter_actor_show (priv->actor);
}

static void
bjb_editor_toolbar_get_property (GObject  *object,
                                 guint     property_id,
                                 GValue   *value,
                                 GParamSpec *pspec)
{
  BjbEditorToolbar *self = BJB_EDITOR_TOOLBAR (object);

  switch (property_id)
  {
    case PROP_ACTOR:
      g_value_set_object (value, self->priv->actor);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
bjb_editor_toolbar_set_property (GObject  *object,
                                 guint     property_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  BjbEditorToolbar *self = BJB_EDITOR_TOOLBAR (object);

  switch (property_id)
  {
    case PROP_ACTOR:
      self->priv->parent_actor = g_value_get_object (value);
      break;
    case PROP_BJB_NOTE_VIEW:
      self->priv->view = g_value_get_object (value);
      break;
    case PROP_NOTE:
      self->priv->note = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
editor_toolbar_align (BjbEditorToolbar *self, GdkEvent  *event)
{
  gint                     x_alignment, y_alignment;
  ClutterConstraint       *constraint;
  BjbEditorToolbarPrivate *priv = self->priv;

  x_alignment = event->button.x + EDITOR_TOOLBAR_X_OFFSET;
  y_alignment = event->button.y + EDITOR_TOOLBAR_Y_OFFSET;

  if ( x_alignment < 0)
    x_alignment = 0;

  constraint = clutter_bind_constraint_new (priv->parent_actor,
                                            CLUTTER_BIND_Y,
                                            y_alignment);
  clutter_actor_add_constraint (priv->actor, constraint);

  constraint = clutter_bind_constraint_new (priv->parent_actor,
                                            CLUTTER_BIND_X,
                                            x_alignment);   
  clutter_actor_add_constraint (priv->actor, constraint);
}

static void
show_edit_bar (BjbEditorToolbar *self, GdkEvent *event)
{
  if (event)
    editor_toolbar_align (self, event);

  bjb_editor_toolbar_fade_in (self);
}

static gboolean
on_button_released (GtkWidget *widget,
                    GdkEvent *event,
                    BjbEditorToolbar *self)
{
  switch (event->button.button)
  {
    /* If left click, see if selection */
    case 1:
      if (biji_note_obj_editor_has_selection (self->priv->note))
        show_edit_bar (self, event);

      else
        bjb_editor_toolbar_fade_out (self);

      return FALSE;

    default:
      return FALSE;
  }
}

static gboolean
on_key_released                     (GtkWidget *widget,
                                     GdkEvent  *event,
                                     gpointer   user_data)
{
  BjbEditorToolbar *self = BJB_EDITOR_TOOLBAR (user_data);

  if (biji_note_obj_editor_has_selection (self->priv->note))
    show_edit_bar (self, event);

  else
    bjb_editor_toolbar_fade_out (self);

  return FALSE;
}

static gboolean
on_button_pressed (GtkWidget *widget,
                   GdkEvent  *event,
                   BjbEditorToolbar *self)
{
  switch (event->button.button)
  {
    /* Show toolbar on right-click */
    case 3:
      show_edit_bar (self, event);
      return TRUE;

    /* Do not break stuff otherwise */
    default :
      return FALSE;
  }
}

static gboolean
on_cut_clicked (GtkWidget *button, BjbEditorToolbar *self)
{
  biji_note_obj_editor_cut (self->priv->note);
  return TRUE ;
}

static gboolean
on_copy_clicked (GtkWidget *button, BjbEditorToolbar *self)
{
  biji_note_obj_editor_copy (self->priv->note);
  return TRUE ;
}

static gboolean
on_paste_clicked (GtkWidget *button, BjbEditorToolbar *self)
{
  biji_note_obj_editor_paste (self->priv->note);
  return TRUE ;
}

static void
bold_button_callback (GtkWidget *button, BijiNoteObj *note)
{
  biji_note_obj_editor_apply_format (note, BIJI_BOLD);
}

static void
italic_button_callback (GtkWidget *button, BijiNoteObj *note)
{
  biji_note_obj_editor_apply_format (note, BIJI_ITALIC);
}

static void
strike_button_callback (GtkWidget *button, BijiNoteObj *note)
{
  biji_note_obj_editor_apply_format (note, BIJI_STRIKE);
}

static void
link_callback (GtkWidget *button, BjbEditorToolbar *self)
{
  gchar                   *link;
  GtkWidget               *window;
  BijiNoteObj             *result;
  GdkRGBA                 color;
  BijiNoteBook            *book;
  BjbEditorToolbarPrivate *priv = self->priv;

  link = biji_note_obj_editor_get_selection (priv->note);

  if (link == NULL)
    return;

  window = bjb_note_view_get_base_window (priv->view);
  book = bjb_window_base_get_book(window);

  result = biji_note_book_get_new_note_from_string (book, link);

  /* Change result color. */
  if (biji_note_obj_get_rgba (priv->note, &color))
    biji_note_obj_set_rgba (result, &color);

  bijiben_new_window_for_note(g_application_get_default(), result);
}

static void
bjb_editor_toolbar_constructed (GObject *obj)
{
  BjbEditorToolbar        *self = BJB_EDITOR_TOOLBAR(obj);
  BjbEditorToolbarPrivate *priv = self->priv ;
  GtkWidget               *view;
  GtkWidget               *window;

  G_OBJECT_CLASS (bjb_editor_toolbar_parent_class)->constructed (obj);

  window = bjb_note_view_get_base_window (priv->view);
  gtk_window_add_accel_group (GTK_WINDOW (window), priv->accel);

  /* text selected --> fade in , and not selected --> fade out */
  view = biji_note_obj_get_editor (priv->note);

  g_signal_connect(view,"button-press-event",
                   G_CALLBACK(on_button_pressed),self);

  g_signal_connect(view,"button-release-event",
                   G_CALLBACK(on_button_released),self);

  g_signal_connect(view,"key-release-event",
                   G_CALLBACK(on_key_released),self);

  /* buttons */
  
  g_signal_connect (priv->toolbar_cut,"clicked",
                    G_CALLBACK(on_cut_clicked), self);

  g_signal_connect (priv->toolbar_copy,"clicked",
                    G_CALLBACK(on_copy_clicked), self);

  g_signal_connect (priv->toolbar_paste,"clicked",
                    G_CALLBACK(on_paste_clicked), self);

  g_signal_connect (priv->toolbar_bold,"clicked",
                    G_CALLBACK(bold_button_callback), priv->note);
  gtk_widget_add_accelerator (priv->toolbar_bold,
                              "activate", priv->accel, GDK_KEY_b,
                              GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  g_signal_connect (priv->toolbar_italic,"clicked",
                    G_CALLBACK(italic_button_callback), priv->note);
  gtk_widget_add_accelerator (priv->toolbar_italic,
                              "activate", priv->accel, GDK_KEY_i,
                              GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  g_signal_connect (priv->toolbar_strike,"clicked",
                    G_CALLBACK(strike_button_callback), priv->note);
  gtk_widget_add_accelerator (priv->toolbar_strike,
                              "activate", priv->accel, GDK_KEY_s,
                              GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  g_signal_connect (priv->toolbar_link,"clicked",
                    G_CALLBACK(link_callback), self);
}

static void
bjb_editor_toolbar_finalize (GObject *obj)
{
  BjbEditorToolbar *self = BJB_EDITOR_TOOLBAR (obj);
  BjbEditorToolbarPrivate *priv = self->priv;
  GtkWidget *window;

  window = bjb_note_view_get_base_window (priv->view);
  gtk_window_remove_accel_group (GTK_WINDOW (window), priv->accel);
  g_object_unref (priv->accel);

  clutter_actor_destroy (priv->actor);
}

static void
bjb_editor_toolbar_class_init (BjbEditorToolbarClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->get_property = bjb_editor_toolbar_get_property ;
  object_class->set_property = bjb_editor_toolbar_set_property ;
  object_class->constructed = bjb_editor_toolbar_constructed ;
  object_class->finalize = bjb_editor_toolbar_finalize;

  properties[PROP_ACTOR] = g_param_spec_object ("actor",
                                                "Actor",
                                                "ParentActor",
                                                CLUTTER_TYPE_ACTOR,
                                                G_PARAM_READWRITE |
                                                G_PARAM_CONSTRUCT |
                                                G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_ACTOR,properties[PROP_ACTOR]);

  properties[PROP_BJB_NOTE_VIEW] = g_param_spec_object ("bjbnoteview",
                                                        "bjbnoteview",
                                                        "bjbnoteview",
                                                        BJB_TYPE_NOTE_VIEW,
                                                        G_PARAM_READWRITE  |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_BJB_NOTE_VIEW,properties[PROP_BJB_NOTE_VIEW]);

  properties[PROP_NOTE] = g_param_spec_object ("note",
                                               "Note",
                                               "Biji Note Obj",
                                                BIJI_TYPE_NOTE_OBJ,
                                                G_PARAM_READWRITE  |
                                                G_PARAM_CONSTRUCT |
                                                G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_NOTE,properties[PROP_NOTE]);

  g_type_class_add_private (class, sizeof (BjbEditorToolbarPrivate));
}


BjbEditorToolbar *
bjb_editor_toolbar_new (ClutterActor   *parent_actor,
                        BjbNoteView    *bjb_note_view,
                        BijiNoteObj    *biji_note_obj)
{
  return g_object_new (BJB_TYPE_EDITOR_TOOLBAR,
                       "actor"       , parent_actor,
                       "bjbnoteview" , bjb_note_view,
                       "note"        , biji_note_obj,
                       NULL);
}

ClutterActor *
bjb_editor_toolbar_get_actor (BjbEditorToolbar *self)
{
  return self->priv->actor;
}
