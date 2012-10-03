/* biji-webkit-editor
 * Copyright (C) Pierre-Yves LUYTEN 2012 <py@luyten.fr>
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

#ifndef _BIJI_WEBKIT_EDITOR_H_
#define _BIJI_WEBKIT_EDITOR_H_

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "../biji-note-obj.h"

G_BEGIN_DECLS

#define BIJI_TYPE_WEBKIT_EDITOR             (biji_webkit_editor_get_type ())
#define BIJI_WEBKIT_EDITOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_WEBKIT_EDITOR, BijiWebkitEditor))
#define BIJI_WEBKIT_EDITOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_WEBKIT_EDITOR, BijiWebkitEditorClass))
#define BIJI_IS_WEBKIT_EDITOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_WEBKIT_EDITOR))
#define BIJI_IS_WEBKIT_EDITOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_WEBKIT_EDITOR))
#define BIJI_WEBKIT_EDITOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_WEBKIT_EDITOR, BijiWebkitEditorClass))

typedef struct _BijiWebkitEditorClass BijiWebkitEditorClass;
typedef struct _BijiWebkitEditor BijiWebkitEditor;
typedef struct _BijiWebkitEditorPrivate BijiWebkitEditorPrivate;

struct _BijiWebkitEditorClass
{
  WebKitWebViewClass parent_class;
};

struct _BijiWebkitEditor
{
  WebKitWebView parent_instance;
  BijiWebkitEditorPrivate * priv;
};

GType biji_webkit_editor_get_type (void) G_GNUC_CONST;

BijiWebkitEditor * biji_webkit_editor_new (BijiNoteObj *note);

void biji_webkit_editor_apply_format (BijiWebkitEditor *self, gint format);

gboolean biji_webkit_editor_has_selection (BijiWebkitEditor *self);

gchar * biji_webkit_editor_get_selection (BijiWebkitEditor *self);

void biji_webkit_editor_cut (BijiWebkitEditor *self);

void biji_webkit_editor_copy (BijiWebkitEditor *self);

void biji_webkit_editor_paste (BijiWebkitEditor *self);

G_END_DECLS

#endif /* _WEBKIT_EDITOR_H_ */
