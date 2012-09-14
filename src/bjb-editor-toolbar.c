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

#include "bjb-editor-toolbar.h"
#include "bjb-window-base.h"

enum
{
  PROP_0,
  PROP_ACTOR,
  PROP_EDITOR,
  PROP_BJB_NOTE_VIEW,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

struct _BjbEditorToolbarPrivate
{
  /* Editor is the text view */
  BjbNoteView        *view;
  ClutterActor       *actor;
  GtkWidget          *widget;
  BijiNoteEditor     *editor;
  ClutterActor       *parent_actor;
  ClutterConstraint  *width_constraint;

  /* TODO
   * If text is cut/clipped we should have the toolbar remain visible */
  gboolean           *clipboard;

  /* BUTTONS */
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

  clutter_actor_show (priv->actor);
  clutter_actor_animate (priv->actor,
                         CLUTTER_EASE_OUT_QUAD,300,
                         "opacity",255,
                         NULL);
}


static void
bjb_editor_toolbar_fade_out (BjbEditorToolbar *self)
{
  ClutterAnimation *animation;
  BjbEditorToolbarPrivate *priv = self->priv;

  animation = clutter_actor_animate (priv->actor, CLUTTER_EASE_OUT_QUAD, 300, "opacity", 0, NULL);
  g_signal_connect_swapped (animation, "completed", G_CALLBACK (clutter_actor_hide), priv->actor);
}

static void
bjb_editor_toolbar_init (BjbEditorToolbar *self)
{
  BjbEditorToolbarPrivate   *priv;
  GtkWidget                 *bin;
  GtkWidget                 *image;
  GtkStyleContext           *context;
  GdkRGBA                    transparent = {0.0, 0.0, 0.0, 0.0};
  GdkRGBA                    black = {0.0, 0.0, 0.0, 0.6};

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BJB_TYPE_EDITOR_TOOLBAR, BjbEditorToolbarPrivate);
  priv = self->priv;

  priv->widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL,0);
  context = gtk_widget_get_style_context (priv->widget);
  gtk_style_context_add_class (context, "osd");
  gtk_style_context_add_class (context, "toolbar");

  priv->actor = gtk_clutter_actor_new_with_contents (priv->widget);
  clutter_actor_set_opacity (priv->actor, 0);
  g_object_set (priv->actor, "show-on-set-parent", FALSE, NULL);

  bin = gtk_clutter_actor_get_widget (GTK_CLUTTER_ACTOR (priv->actor));
  gtk_widget_override_background_color (bin,
                                        GTK_STATE_FLAG_NORMAL,
                                        &transparent);

  priv->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  priv->group = gtk_tool_item_new ();
  gtk_container_add (GTK_CONTAINER (priv->group), priv->box);
  gtk_container_add (GTK_CONTAINER (priv->widget), priv->group);
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
  image = gtk_image_new_from_icon_name ("emblem-symbolic-link", GTK_ICON_SIZE_INVALID);
  gtk_image_set_pixel_size (GTK_IMAGE (image), 24);
  gtk_container_add (GTK_CONTAINER (priv->toolbar_link), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (priv->toolbar_link), _("Strike"));
  gtk_container_add (GTK_CONTAINER (priv->box), priv->toolbar_link);
  gtk_widget_override_background_color (priv->toolbar_link,
                                        GTK_STATE_FLAG_NORMAL,
                                        &black);

  gtk_widget_show_all (priv->group);
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
    case PROP_EDITOR:
      self->priv->editor = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
bjb_editor_toolbar_notify_width (GObject *object,
                                 GParamSpec *pspec,
                                 gpointer user_data)
{
  BjbEditorToolbar *self = BJB_EDITOR_TOOLBAR (user_data);
  BjbEditorToolbarPrivate *priv = self->priv;
  
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
show_edit_bar(BjbEditorToolbar *self, gboolean sticky)
{
//  BjbEditorToolbarPrivate *priv = self->priv;

  bjb_editor_toolbar_fade_in (self);

  /* TODO */
  /*
  gtk_text_view_scroll_mark_onscreen(priv->view, 
                              gtk_text_buffer_get_insert(
                                       gtk_text_view_get_buffer(
                                                          priv->view))); */
}

static void
on_text_selected(GObject *toto,BjbEditorToolbar *self)
{
  show_edit_bar(self,FALSE);
}

static gboolean
on_button_pressed (GtkWidget *widget,GdkEvent  *event,BjbEditorToolbar *self)
{
  /* If anything else than right-click, do not break things. */
  if ( event->button.button != 3 )
  {
    return FALSE ;    
  }

  /* If right click, show toolbar and that's all */
  show_edit_bar(self,TRUE);
  return TRUE ;
  
}

static void
on_text_not_selected(GObject *toto,BjbEditorToolbar *self)
{
  /* TODO : check if (private?) clipboard first */
  bjb_editor_toolbar_fade_out (self);
}

static gboolean
on_cut_clicked (GtkWidget *button, GtkTextView *view)
{
  g_signal_emit_by_name (view,"cut-clipboard");
  return TRUE ;
}

static gboolean
on_copy_clicked (GtkWidget *button, GtkTextView *view)
{
  g_signal_emit_by_name (view,"copy-clipboard");
  return TRUE ;
}

static gboolean
on_paste_clicked (GtkWidget *button, GtkTextView *view)
{
  g_signal_emit_by_name (view,"paste-clipboard");
  return TRUE ;
}

static void
bold_button_callback (GtkWidget *button,GtkTextView *view)
{
  biji_toggle_bold_tag (view);
}

static void
italic_button_callback (GtkWidget *button,GtkTextView *view)
{
  biji_toggle_italic_tag (view);
}

static void
strike_button_callback (GtkWidget *button,GtkTextView *view)
{
  biji_toggle_strike_tag (view);
}

/* TODO : Libiji : BijiNote * bjb_notebook_note_new (notebook,string);
 *
 * Toggle button
 *
 * Also, this is unstable since as soon as the parent win is
 * destroyed, the child crashes. */
static void
link_callback (GtkWidget *button, BjbEditorToolbar *self)
{
  gchar                   *link, *folder ;
  GtkWidget               *window;
  BijiNoteObj             *result;
  BijiNoteBook            *book;
  BjbWindowBase           *base;
  BjbEditorToolbarPrivate *priv = self->priv;

  link = gtk_text_view_get_selection(priv->editor);

  if (link == NULL )
    return;

  base = BJB_WINDOW_BASE (bjb_note_view_get_base_window (priv->view));

  book = bjb_window_base_get_book(base);
  folder = g_strdup_printf("%s/bijiben",g_get_user_data_dir());
  result = biji_note_get_new_from_string(link,folder);
  g_free(folder);

  note_book_append_new_note(book,result);
  create_new_window_for_note(bjb_window_base_get_app(base), result);
}

static void
bjb_editor_toolbar_constructed(GObject *obj)
{
  BjbEditorToolbar        *self = BJB_EDITOR_TOOLBAR(obj);
  BjbEditorToolbarPrivate *priv = self->priv ;
  ClutterConstraint       *constraint ;
  GtkTextView             *view;


  G_OBJECT_CLASS (bjb_editor_toolbar_parent_class)->constructed (obj);

  /* text selected --> fade in */
  g_signal_connect(priv->editor,"selection",
                   G_CALLBACK(on_text_selected),self);

  g_signal_connect(priv->editor,"button-press-event",
                   G_CALLBACK(on_button_pressed),self);

  g_signal_connect(priv->editor,"button-release-event",
                   G_CALLBACK(on_button_pressed),self);

  g_signal_connect(priv->editor,"no-more-selection",
                   G_CALLBACK(on_text_not_selected),self);

  /* buttons */
  view = GTK_TEXT_VIEW (priv->editor);
  
  g_signal_connect (priv->toolbar_cut,"clicked",
                    G_CALLBACK(on_cut_clicked), view);

  g_signal_connect (priv->toolbar_copy,"clicked",
                    G_CALLBACK(on_copy_clicked), view);

  g_signal_connect (priv->toolbar_paste,"clicked",
                    G_CALLBACK(on_paste_clicked), view);

  g_signal_connect (priv->toolbar_bold,"clicked",
                    G_CALLBACK(bold_button_callback), view);

  g_signal_connect (priv->toolbar_italic,"clicked",
                    G_CALLBACK(italic_button_callback), view);

  g_signal_connect (priv->toolbar_strike,"clicked",
                    G_CALLBACK(strike_button_callback), view);

  g_signal_connect (priv->toolbar_link,"clicked",
                    G_CALLBACK(link_callback), self);

/*
clutter_bind_constraint_new (priv->parent_actor,
*                      CLUTTER_BIND_WIDTH, -300.0); 
*/
  
  g_signal_connect (priv->actor,
                    "notify::width",
                    G_CALLBACK (bjb_editor_toolbar_notify_width),
                    self);

  constraint = clutter_align_constraint_new (priv->parent_actor, CLUTTER_ALIGN_X_AXIS, 0.50);
  clutter_actor_add_constraint (priv->actor, constraint);

  constraint = clutter_align_constraint_new (priv->parent_actor, CLUTTER_ALIGN_Y_AXIS, 0.95);
  clutter_actor_add_constraint (priv->actor, constraint);
}

static void
bjb_editor_toolbar_class_init (BjbEditorToolbarClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->get_property = bjb_editor_toolbar_get_property ;
  object_class->set_property = bjb_editor_toolbar_set_property ;
  object_class->constructed = bjb_editor_toolbar_constructed ;

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

  properties[PROP_EDITOR] = g_param_spec_object ("editor",
                                                     "editor",
                                                     "editor",
                                                      BIJI_TYPE_NOTE_EDITOR,
                                                      G_PARAM_READWRITE  |
                                                      G_PARAM_CONSTRUCT |
                                                      G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_EDITOR,properties[PROP_EDITOR]);

  g_type_class_add_private (class, sizeof (BjbEditorToolbarPrivate));
}


BjbEditorToolbar *
bjb_editor_toolbar_new (ClutterActor   *parent_actor,
                        BjbNoteView    *bjb_note_view,
                        BijiNoteEditor *biji_note_editor)
{
  return g_object_new (BJB_TYPE_EDITOR_TOOLBAR,
                       "actor", parent_actor,
                       "bjbnoteview",bjb_note_view,
                       "editor",biji_note_editor,
                       NULL);
}


ClutterActor *
bjb_editor_toolbar_get_actor (BjbEditorToolbar *self)
{
  return self->priv->actor;
}
