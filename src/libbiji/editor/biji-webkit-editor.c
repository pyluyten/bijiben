/* biji-webkit-editor.h
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
  gulong changed;
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

gchar *
biji_webkit_editor_get_selection (BijiWebkitEditor *self)
{
  WebKitWebView *view = WEBKIT_WEB_VIEW (self);
  EEditorSelection *sel;

  sel = e_editor_selection_new (view);

  if (e_editor_selection_has_text (sel))
    return (gchar*) e_editor_selection_get_string (sel);

  return NULL;
}

typedef gboolean GetFormatFunc (EEditorSelection*);
typedef void     SetFormatFunc (EEditorSelection*, gboolean);

static void
biji_toggle_format (EEditorSelection *sel,
                    GetFormatFunc get_format,
                    SetFormatFunc set_format)
{
  set_format (sel, !get_format (sel));
}

void
biji_webkit_editor_apply_format (BijiWebkitEditor *self, gint format)
{
  EEditorSelection *sel;
  WebKitWebView *view = WEBKIT_WEB_VIEW (self);

  sel = e_editor_selection_new (view);

  if ( e_editor_selection_has_text (sel))
  {
    switch (format)
    {
      case BIJI_BOLD:
        biji_toggle_format (sel, e_editor_selection_get_bold,
                                 e_editor_selection_set_bold);
        break;

      case BIJI_ITALIC:
        biji_toggle_format (sel, e_editor_selection_get_italic,
                                 e_editor_selection_set_italic);
        break;

      case BIJI_STRIKE:
        biji_toggle_format (sel, e_editor_selection_get_strike_through,
                                 e_editor_selection_set_strike_through);
        break;

      case BIJI_BULLET_LIST:
        e_editor_selection_set_block_format(
                        sel,
                        E_EDITOR_SELECTION_BLOCK_FORMAT_UNORDERED_LIST);
        break;

      case BIJI_ORDER_LIST:
        e_editor_selection_set_block_format(
                        sel,
                        E_EDITOR_SELECTION_BLOCK_FORMAT_ORDERED_LIST);
        break;

      default:
        g_warning ("biji_webkit_editor_apply_format : Invalid format");
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

  /* set standard settings
   * pixels above line, left margin might be for CSS */
  webkit_web_view_set_editable (view, TRUE);
  webkit_web_view_set_transparent (view, TRUE);

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BIJI_TYPE_WEBKIT_EDITOR, BijiWebkitEditorPrivate);
}

static void
biji_webkit_editor_finalize (GObject *object)
{
  BijiWebkitEditor *self = BIJI_WEBKIT_EDITOR (object);
  
  g_signal_handler_disconnect (self, self->priv->changed);
  G_OBJECT_CLASS (biji_webkit_editor_parent_class)->finalize (object);
}

/* Borrowed from evolution webkit composer
 * rather untested */
static void
process_elements (WebKitDOMNode *node, GString *buffer)
{
  WebKitDOMDocument *document;
  WebKitDOMDOMWindow *window;
  WebKitDOMNodeList *nodes;
  WebKitDOMCSSStyleDeclaration *style;
  gchar *display;
  gulong ii, length;
  GRegex *regex;

  document = webkit_dom_node_get_owner_document (node);
  window = webkit_dom_document_get_default_view (document);

  /* Is this a block element? */
  style = webkit_dom_dom_window_get_computed_style (
                                 window, WEBKIT_DOM_ELEMENT (node), "");
  display = webkit_dom_css_style_declaration_get_property_value (
                                                      style, "display");

  nodes = webkit_dom_node_get_child_nodes (node);
  length = webkit_dom_node_list_get_length (nodes);
  regex = g_regex_new ("\x9", 0, 0, NULL);

  for (ii = 0; ii < length; ii++)
  {
    WebKitDOMNode *child;

    child = webkit_dom_node_list_item (nodes, ii);
    if (webkit_dom_node_get_node_type (child) == 3)
    {
      gchar *content, *tmp;

      tmp = webkit_dom_node_get_text_content (child);

      /* Replace tabs with 4 whitespaces, otherwise they got
         replaced by single whitespace */
      content = g_regex_replace (regex, tmp, -1, 0, "    ", 0, NULL);

      g_string_append (buffer, content);
      g_free (tmp);
      g_free (content);
    }

    if (webkit_dom_node_has_child_nodes (child))
      process_elements (child, buffer);

  }

  if (g_strcmp0 (display, "block") == 0)
    g_string_append (buffer, "\n");

  g_free (display);
  g_regex_unref (regex);
}

static void
on_content_changed (WebKitWebView *view)
{
  BijiWebkitEditor     *self = BIJI_WEBKIT_EDITOR (view);
  WebKitDOMDocument    *dom;
  WebKitDOMHTMLElement *elem;
  WebKitDOMNode        *body;
  gchar                *result;
  GString              *plain_text;

  /* First html serializing */
  dom = webkit_web_view_get_dom_document (view);
  elem = webkit_dom_document_get_body (dom);
  result = webkit_dom_html_element_get_inner_html (elem);

  biji_note_obj_set_html_content (self->priv->note, result);
  g_free (result);

  /* Then the raw text for icon tracker or whatever else */
  body = (WebKitDOMNode *) webkit_dom_document_get_body (dom);

  plain_text = g_string_sized_new (1024);
  process_elements (body, plain_text);

  /* Return text content between <body> and </body> */
  biji_note_obj_set_raw_text (self->priv->note, plain_text->str);
  g_string_free (plain_text, TRUE);

  note_obj_save_note_using_buffer (self->priv->note);
}

static void
on_note_color_changed (BijiNoteObj *note, BijiWebkitEditor *self)
{
  GdkRGBA color;

  if (biji_note_obj_get_rgba(note,&color))
    set_editor_color (GTK_WIDGET (self), &color);
}

static void
biji_webkit_editor_constructed (GObject *obj)
{
  BijiWebkitEditor *self;
  BijiWebkitEditorPrivate *priv;
  WebKitWebView *view;
  gchar *html;
  GdkRGBA color;

  self = BIJI_WEBKIT_EDITOR (obj);
  view = WEBKIT_WEB_VIEW (self);
  priv = self->priv;

  /* Load the note */
  html = biji_note_obj_get_html (priv->note);

  if (html)
    webkit_web_view_load_string (view, html, NULL, NULL, NULL);

  /* Apply color */
  if (biji_note_obj_get_rgba (priv->note,&color))
    set_editor_color (GTK_WIDGET (self), &color);

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
