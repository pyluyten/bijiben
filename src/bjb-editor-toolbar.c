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

#include "bjb-bijiben.h"
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

  priv->widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL,0);
  context = gtk_widget_get_style_context (priv->widget);
  gtk_style_context_add_class (context, "osd");
  gtk_style_context_add_class (context, "toolbar");

  priv->actor = gtk_clutter_actor_new_with_contents (priv->widget);
  clutter_actor_set_opacity (priv->actor, 0);
  g_object_set (priv->actor, "show-on-set-parent", FALSE, NULL);

  clutter_actor_set_easing_mode (priv->actor, CLUTTER_EASE_IN_QUAD);
  clutter_actor_set_easing_duration (priv->actor, 300.0);

  clutter_actor_set_easing_mode (priv->actor, CLUTTER_EASE_OUT_QUAD);
  clutter_actor_set_easing_duration (priv->actor, 200.0);

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
  priv->toolbar_paste = gtk_toggle_button_new_with_label ("Paste");
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
  full_path = g_strdup_printf ("%s/bijiben/icons/hicolor/scalable/actions/link.svg", icons_path);
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
    case PROP_EDITOR:
      self->priv->editor = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
editor_toolbar_align (BjbEditorToolbar *self)
{
  BjbEditorToolbarPrivate *priv = self->priv;
  GtkTextIter              iter;
  GdkRectangle             iter_rect, win_rect;
  gint                     x_alignment, y_alignment;
  GtkTextView             *view;
  GtkTextBuffer           *buf;
  ClutterConstraint       *constraint;

  /* we do align the editor toolbar few pixels below the cursor
   * (we have to remove the invisible part of textView
   * x : we do align middle of bar to iter    */

//WK : should libbiji should return us the GdkRect?
  g_warning ("editor_toolbar_align : not yet ported to wk");
/*WK  view = GTK_TEXT_VIEW (priv->editor);
  buf = gtk_text_view_get_buffer (view);
  gtk_text_buffer_get_iter_at_mark (buf,
                                    &iter,
                                    gtk_text_buffer_get_insert (buf));
  
  gtk_text_view_get_iter_location (view, &iter, &iter_rect);
  gtk_text_view_get_visible_rect (view, &win_rect);

  y_alignment = iter_rect.y - win_rect.y + EDITOR_TOOLBAR_Y_OFFSET;
  x_alignment = iter_rect.x + EDITOR_TOOLBAR_X_OFFSET ;

  if ( x_alignment < 0)
    x_alignment = 0;

  constraint = clutter_bind_constraint_new (priv->parent_actor,
                                            CLUTTER_BIND_Y,
                                            y_alignment);
  clutter_actor_add_constraint (priv->actor, constraint);

  constraint = clutter_bind_constraint_new (priv->parent_actor,
                                            CLUTTER_BIND_X,
                                            x_alignment);   
  clutter_actor_add_constraint (priv->actor, constraint); */
}

static void
show_edit_bar(BjbEditorToolbar *self, gboolean sticky)
{
  editor_toolbar_align (self);
  bjb_editor_toolbar_fade_in (self);
}

static void
on_text_selected(GObject *toto,BjbEditorToolbar *self)
{
  self->priv->glued = FALSE;
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
  self->priv->glued = TRUE;
  show_edit_bar(self,TRUE);
  return TRUE ;
}

/* only keep visible if the user wants to paste with right-click */
static void
on_text_not_selected(GObject *toto,BjbEditorToolbar *self)
{
  if ( self->priv->glued == FALSE )
  {
    bjb_editor_toolbar_fade_out (self);
  }

  else
  {
    editor_toolbar_align (self);
    self->priv->glued = FALSE;
  }
}

static gboolean
on_cut_clicked (GtkWidget *button, BjbEditorToolbar *self)
{
  GtkTextView *view = GTK_TEXT_VIEW (self->priv->editor);

  g_signal_emit_by_name (view,"cut-clipboard");
  return TRUE ;
}

static gboolean
on_copy_clicked (GtkWidget *button, BjbEditorToolbar *self)
{
  GtkTextView *view = GTK_TEXT_VIEW (self->priv->editor);

  g_signal_emit_by_name (view,"copy-clipboard");
  return TRUE ;
}

static gboolean
on_paste_clicked (GtkWidget *button, BjbEditorToolbar *self)
{
  GtkTextView *view = GTK_TEXT_VIEW (self->priv->editor);

  g_signal_emit_by_name (view,"paste-clipboard");
  return TRUE ;
}

static void
bold_button_callback (GtkWidget *button,GtkTextView *view)
{
//WKbiji_toggle_bold_tag (view);
  //webkit_editor_apply_format (
}

static void
italic_button_callback (GtkWidget *button,GtkTextView *view)
{
//WKbiji_toggle_italic_tag (view);
}

static void
strike_button_callback (GtkWidget *button,GtkTextView *view)
{
//WKbiji_toggle_strike_tag (view);
}

/* TODO : Libiji : BijiNote * bjb_notebook_note_new (notebook,string); */
static void
link_callback (GtkWidget *button, BjbEditorToolbar *self)
{
  gchar                   *link, *folder ;
  GtkWidget               *window;
  BijiNoteObj             *result;
  BijiNoteBook            *book;
  BjbEditorToolbarPrivate *priv = self->priv;

//WK  link = gtk_text_view_get_selection( GTK_TEXT_VIEW (priv->editor));
  g_warning ("Link is not yet ported to WK");
  link = NULL;

  if (link == NULL)
    return;

  window = bjb_note_view_get_base_window (priv->view);

  book = bjb_window_base_get_book(window);
  folder = g_strdup_printf("%s/bijiben",g_get_user_data_dir());
  result = biji_note_get_new_from_string(link,folder);
  g_free(folder);

  note_book_append_new_note(book,result);
  create_new_window_for_note(bjb_window_base_get_app(window), result);
}

static void
bjb_editor_toolbar_constructed(GObject *obj)
{
  BjbEditorToolbar        *self = BJB_EDITOR_TOOLBAR(obj);
  BjbEditorToolbarPrivate *priv = self->priv ;
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
                    G_CALLBACK(on_cut_clicked), self);

  g_signal_connect (priv->toolbar_copy,"clicked",
                    G_CALLBACK(on_copy_clicked), self);

  g_signal_connect (priv->toolbar_paste,"clicked",
                    G_CALLBACK(on_paste_clicked), self);

  g_signal_connect (priv->toolbar_bold,"clicked",
                    G_CALLBACK(bold_button_callback), view);

  g_signal_connect (priv->toolbar_italic,"clicked",
                    G_CALLBACK(italic_button_callback), view);

  g_signal_connect (priv->toolbar_strike,"clicked",
                    G_CALLBACK(strike_button_callback), view);

  g_signal_connect (priv->toolbar_link,"clicked",
                    G_CALLBACK(link_callback), self);
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
