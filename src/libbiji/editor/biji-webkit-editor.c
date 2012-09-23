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

G_DEFINE_TYPE (BijiWebkitEditor, biji_webkit_editor, WEBKIT_TYPE_WEB_VIEW);

gboolean
biji_webkit_editor_has_selection (BijiWebkitEditor *self)
{
  WebKitWebView *view = WEBKIT_WEB_VIEW (self);
  EEditorSelection *sel;
  gchar *text = NULL ;

  sel = e_editor_selection_new (view);

  if (e_editor_selection_has_text (sel))
  {
    text = e_editor_selection_get_string (sel);
    g_warning ("selection is %s", text);

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
biji_webkit_editor_init (BijiWebkitEditor *self)
{
  WebKitWebView *view = WEBKIT_WEB_VIEW (self);

  webkit_web_view_load_string (view, "test", NULL, NULL, NULL);
  webkit_web_view_set_editable (view, TRUE);
  webkit_web_view_set_transparent (view, TRUE);
}

static void
biji_webkit_editor_finalize (GObject *object)
{
  /* TODO */

  G_OBJECT_CLASS (biji_webkit_editor_parent_class)->finalize (object);
}

static void
biji_webkit_editor_class_init (BijiWebkitEditorClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = biji_webkit_editor_finalize;
}

BijiWebkitEditor *
biji_webkit_editor_new (BijiNoteObj *note)
{
  return g_object_new (BIJI_TYPE_WEBKIT_EDITOR, NULL);
}
