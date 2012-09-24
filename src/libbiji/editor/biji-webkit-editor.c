/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * WebkitWebView
 * Copyright (C) buddho 2012 <buddho@localhost.localdomain>
 * 
WebkitWebView is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * WebkitWebView is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "biji-webkit-editor.h"
#include "biji-editor-selection.h"

/* Prop */
enum {
  PROP_0,
  PROP_NOTE,
  NUM_PROP
};

/* Signals */
enum {
  EDITOR_CLOSED,
  EDITOR_SIGNALS
};

static guint biji_editor_signals [EDITOR_SIGNALS] = { 0 };

static GParamSpec *properties[NUM_PROP] = { NULL, };

struct _BijiWebkitEditorPrivate
{
  BijiNoteObj *note;
};

G_DEFINE_TYPE (BijiWebkitEditor, biji_webkit_editor, WEBKIT_TYPE_WEB_VIEW);

gboolean
biji_webkit_editor_has_selection (BijiWebkitEditor *self)
{
  WebKitWebView *view = WEBKIT_WEB_VIEW (self);
  EEditorSelection *sel;
  const gchar *text = NULL ;

  sel = e_editor_selection_new (view);

  if (e_editor_selection_has_text (sel))
  {
    text = e_editor_selection_get_string (sel);

    if ( g_strcmp0 (text, "") != 0)
      return TRUE;
  }

  return FALSE;
}

void
biji_webkit_editor_apply_format (BijiWebkitEditor *self, gint format)
{
  EEditorSelection *sel;
  WebKitWebView *view = WEBKIT_WEB_VIEW (self);

  sel = e_editor_selection_new (view);

  if ( e_editor_selection_has_text (sel))
  {
    if ( format == BIJI_BOLD)
    {
      if ( e_editor_selection_get_bold (sel) )
        e_editor_selection_set_bold (sel, FALSE);

      else
        e_editor_selection_set_bold (sel, TRUE);
    }

    if ( format == BIJI_ITALIC)
    {
      if ( e_editor_selection_get_italic (sel) )
        e_editor_selection_set_italic (sel, FALSE);

      else
        e_editor_selection_set_italic (sel, TRUE);
    }

    if ( format == BIJI_STRIKE)
    {
      if ( e_editor_selection_get_strike_through (sel) )
        e_editor_selection_set_strike_through (sel, FALSE);

      else
        e_editor_selection_set_strike_through (sel, TRUE);
    }

  }
}

void
biji_webkit_editor_cut (BijiWebkitEditor *self)
{
  webkit_web_view_cut_clipboard (WEBKIT_WEB_VIEW (self));
}

void
biji_webkit_editor_copy (BijiWebkitEditor *self)
{
  webkit_web_view_copy_clipboard (WEBKIT_WEB_VIEW (self));
}

void
biji_webkit_editor_paste (BijiWebkitEditor *self)
{
  webkit_web_view_paste_clipboard (WEBKIT_WEB_VIEW (self));
}

static void
set_editor_color (GtkWidget *w, GdkRGBA *col)
{
  gtk_widget_override_background_color (w, GTK_STATE_FLAG_NORMAL, col);
}

static void
biji_webkit_editor_init (BijiWebkitEditor *self)
{
  WebKitWebView *view = WEBKIT_WEB_VIEW (self);

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BIJI_TYPE_WEBKIT_EDITOR, BijiWebkitEditorPrivate);

  /* set standard settings */
  webkit_web_view_set_editable (view, TRUE);
  webkit_web_view_set_transparent (view, TRUE);

  /* Padding */
//WK  gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW(priv->view),8);
//WK  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(priv->view),16);
}

static void
biji_webkit_editor_finalize (GObject *object)
{
  BijiWebkitEditor *self = BIJI_WEBKIT_EDITOR (object);

  /* Signal the note it has no more editor */
  g_warning ("biji webkit editor finalize");

  /*g_signal_emit (G_OBJECT (object),
                 biji_editor_signals[EDITOR_CLOSED],
                 0);*/
  on_biji_note_obj_editor_closed (self->priv->note);

  G_OBJECT_CLASS (biji_webkit_editor_parent_class)->finalize (object);
}

static void
on_content_changed (WebKitWebView *view)
{
  BijiWebkitEditor *self = BIJI_WEBKIT_EDITOR (self);

  g_warning ("content changed, should queue save. Instead we save directly");
}

static void
on_note_color_changed (BijiNoteObj *note, BijiWebkitEditor *self)
{
  set_editor_color (GTK_WIDGET (self), biji_note_obj_get_rgba (note));
}

static void
biji_webkit_editor_constructed (GObject *obj)
{
  BijiWebkitEditor *self = BIJI_WEBKIT_EDITOR (obj);
  BijiWebkitEditorPrivate *priv = self->priv;
  WebKitWebView *view = WEBKIT_WEB_VIEW (self);

  /* Load the note */
  /* Instead raw text we want to retrieve note HTML */
  webkit_web_view_load_string (view,
                               biji_note_get_raw_text (priv->note),
                               NULL, NULL, NULL);

  /* Apply color */
  set_editor_color (GTK_WIDGET (self),
                    biji_note_obj_get_rgba (priv->note));
  g_signal_connect (priv->note, "color-changed",
                    G_CALLBACK (on_note_color_changed), self);

  /* Save */
  g_signal_connect (WEBKIT_WEB_VIEW (self), "user-changed-contents",
                    G_CALLBACK (on_content_changed), NULL);
}

static void
biji_webkit_editor_get_property (GObject  *object,
                                 guint     property_id,
                                 GValue   *value,
                                 GParamSpec *pspec)
{
  BijiWebkitEditor *self = BIJI_WEBKIT_EDITOR (object);

  switch (property_id)
  {
    case PROP_NOTE:
      g_value_set_object (value, self->priv->note);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
biji_webkit_editor_set_property (GObject  *object,
                                 guint     property_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  BijiWebkitEditor *self = BIJI_WEBKIT_EDITOR (object);

  switch (property_id)
  {
    case PROP_NOTE:
      self->priv->note = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
biji_webkit_editor_class_init (BijiWebkitEditorClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = biji_webkit_editor_constructed;
  object_class->finalize = biji_webkit_editor_finalize;
  object_class->get_property = biji_webkit_editor_get_property;
  object_class->set_property = biji_webkit_editor_set_property;

  properties[PROP_NOTE] = g_param_spec_object ("note",
                                               "Note",
                                               "Biji Note Obj",
                                                BIJI_TYPE_NOTE_OBJ,
                                                G_PARAM_READWRITE  |
                                                G_PARAM_CONSTRUCT |
                                                G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_NOTE,properties[PROP_NOTE]);

  biji_editor_signals[EDITOR_CLOSED] = g_signal_new ("closed",
                                       G_OBJECT_CLASS_TYPE (klass),
                                       G_SIGNAL_RUN_FIRST,
                                       0,
                                       NULL,
                                       NULL,
                                       g_cclosure_marshal_VOID__VOID,
                                       G_TYPE_NONE,
                                       0);

  g_type_class_add_private (klass, sizeof (BijiWebkitEditorPrivate));
}

BijiWebkitEditor *
biji_webkit_editor_new (BijiNoteObj *note)
{
  return g_object_new (BIJI_TYPE_WEBKIT_EDITOR,
                       "note", note,
                       NULL);
}
