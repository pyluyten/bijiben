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

  }

  else 
  {
    g_warning ("no text selected");
  }
}

static void
biji_webkit_editor_init (BijiWebkitEditor *self)
{
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

GtkWidget *
biji_webkit_editor_new (BijiNoteObj *note)
{
  WebKitWebView *self;

  self = WEBKIT_WEB_VIEW (g_object_new (WEBKIT_TYPE_WEB_VIEW, NULL));
  webkit_web_view_load_string (self, "test", NULL, NULL, NULL);
  webkit_web_view_set_editable (self, TRUE);

  return GTK_WIDGET (self);
}
