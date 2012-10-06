/*
 * Bijiben
 * Copyright (C) Pierre-Yves Luyten 2012 <py@luyten.fr>
 *
 * Bijiben is free software: you can redistribute it and/or modify it
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

#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <libxml/xmlreader.h>

#include <string.h>

#include "biji-lazy-deserializer.h"
#include "../libbiji.h"

enum
{
  PROP_0,
  PROP_NOTE,
  NUM_PROPERTIES
};

/* Notes type : bijiben namespace, tomboy one, then note types */
#define BIJI_NS "http://projects.gnome.org/bijiben"

#define TOMBOY_NS "http://beatniksoftware.com/tomboy"

typedef enum 
{
  NO_TYPE,
  BIJIBEN_1,
  TOMBOY_1,
  TOMBOY_2,
  TOMBOY_3,
  NUM_NOTE_TYPES
} BijiXmlType;

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

G_DEFINE_TYPE (BijiLazyDeserializer, biji_lazy_deserializer, G_TYPE_OBJECT);

struct _BijiLazyDeserializerPrivate
{
  /* Note, type, first reader for metadata */
  BijiNoteObj *note;
  BijiXmlType type;
  xmlTextReaderPtr r;

  /* Reader for internal content, either tomboy XML or Bijiben html */
  xmlTextReaderPtr inner;
  gchar *content;

  /* Result for both raw_text & html */
  GString *raw_text;
  GString *html;
};

static void
biji_lazy_deserializer_get_property (GObject  *object,
                                     guint     property_id,
                                     GValue   *value,
                                     GParamSpec *pspec)
{
  BijiLazyDeserializer *self = BIJI_LAZY_DESERIALIZER (object);

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
biji_lazy_deserializer_set_property (GObject  *object,
                                     guint     property_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
  BijiLazyDeserializer *self = BIJI_LAZY_DESERIALIZER (object);

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
biji_lazy_deserializer_init (BijiLazyDeserializer *self)
{
  BijiLazyDeserializerPrivate *priv;

  priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                      BIJI_TYPE_LAZY_DESERIALIZER,
                                      BijiLazyDeserializerPrivate);
  self->priv = priv;

  priv->raw_text = g_string_new ("");
  priv->html = g_string_new ("");
}

static void
biji_lazy_deserializer_finalize (GObject *object)
{
  BijiLazyDeserializer *self= BIJI_LAZY_DESERIALIZER (object);
  BijiLazyDeserializerPrivate *priv = self->priv;

  g_string_free (priv->raw_text, TRUE);
  g_string_free (priv->html, TRUE);
  g_free (priv->content);

  xmlFreeTextReader (priv->r);
  xmlFreeTextReader (priv->inner);

  G_OBJECT_CLASS (biji_lazy_deserializer_parent_class)->finalize (object);
}

static void
biji_lazy_deserializer_class_init (BijiLazyDeserializerClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = biji_lazy_deserializer_get_property;
  object_class->set_property = biji_lazy_deserializer_set_property;
  object_class->finalize = biji_lazy_deserializer_finalize;

  properties[PROP_NOTE] = g_param_spec_object ("note",
                                               "Note",
                                               "Biji Note Obj",
                                               BIJI_TYPE_NOTE_OBJ,
                                               G_PARAM_READWRITE  |
                                               G_PARAM_CONSTRUCT |
                                               G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_NOTE,properties[PROP_NOTE]);

  g_type_class_add_private (klass, sizeof (BijiLazyDeserializerPrivate));
}

/* Tomboy Inner XML */

static void
process_tomboy_start_elem (BijiLazyDeserializer *self)
{
  BijiLazyDeserializerPrivate *priv = self->priv;
  const gchar *element_name;

  element_name = (const gchar *) xmlTextReaderConstName(priv->inner);

  if (g_strcmp0 (element_name, "note-content")==0)
    return;

  if (g_strcmp0 (element_name, "bold")==0)
    priv->html = g_string_append (priv->html, "<b>");

  if (g_strcmp0 (element_name, "italic")==0)
    priv->html = g_string_append (priv->html, "<i>");

  if (g_strcmp0 (element_name, "strikethrough")==0)
    priv->html = g_string_append (priv->html, "<strike>");

  /* Currently tomboy has unordered list */

  if (g_strcmp0 (element_name, "list")==0)
    priv->html = g_string_append (priv->html, "<ul>");

  if (g_strcmp0 (element_name, "list-item")==0)
    priv->html = g_string_append (priv->html, "<li>");
}

static void
process_tomboy_end_elem (BijiLazyDeserializer *self)
{
  BijiLazyDeserializerPrivate *priv = self->priv;
  const gchar *element_name;
  
  element_name = (const gchar *) xmlTextReaderConstName (priv->inner);

  if (g_strcmp0 (element_name, "note-content")==0)
    return;

  if (g_strcmp0 (element_name, "bold")==0)
    priv->html = g_string_append (priv->html, "</b>");

  if (g_strcmp0 (element_name, "italic")==0)
    priv->html = g_string_append (priv->html, "</i>");

  if (g_strcmp0 (element_name, "strikethrough")==0)
    priv->html = g_string_append (priv->html, "</strike>");

  /* Currently tomboy has unordered list */

  if (g_strcmp0 (element_name, "list")==0)
    priv->html = g_string_append (priv->html, "</ul>");

  if (g_strcmp0 (element_name, "list-item")==0)
    priv->html = g_string_append (priv->html, "</li>");
}

static void
process_tomboy_text_elem (BijiLazyDeserializer *self)
{
  const gchar *text;
  BijiLazyDeserializerPrivate *priv = self->priv;

  text = (const gchar *) xmlTextReaderConstValue (priv->inner);

  /* Simply append the text to both raw & html */
  priv->raw_text = g_string_append (priv->raw_text, text);
  priv->html = g_string_append (priv->html, text);
}

static void
process_tomboy_node (BijiLazyDeserializer *self)
{
  int            type;
  const xmlChar *name ;
  BijiLazyDeserializerPrivate *priv = self->priv;

  type  = xmlTextReaderNodeType (priv->inner);
  name  = xmlTextReaderConstName (priv->inner);
  
  if (name == NULL)
    name = BAD_CAST "(NULL)";

  switch (type)
  {
    case XML_ELEMENT_NODE:
      process_tomboy_start_elem (self);
      break;

    case XML_ELEMENT_DECL:
      process_tomboy_end_elem (self);
      break;

    case XML_TEXT_NODE:
      process_tomboy_text_elem (self);
      break;

    case XML_DTD_NODE:
      process_tomboy_text_elem (self);
      break;
  }
}

static void
process_tomboy_xml_content (BijiLazyDeserializer *self)
{
  BijiLazyDeserializerPrivate *priv = self->priv;
  int ret;
  gchar *revamped_html;

  priv->inner = xmlReaderForMemory (priv->content,
                                    strlen(priv->content),
                                    "", "UTF-8", 0);

  ret = xmlTextReaderRead (priv->inner);

  /* Make the GString grow as we read */
  while (ret == 1) 
  {
    process_tomboy_node (self);
    ret = xmlTextReaderRead (priv->inner);
  }

  /* Now the inner content is known, we can
   * assign note values and let deserialization work on last elements*/
  biji_note_obj_set_raw_text (priv->note, priv->raw_text->str);

  revamped_html = biji_str_replace (priv->html->str, "\n", "<br/>");
  biji_note_obj_set_html_content (priv->note, revamped_html);
  g_free (revamped_html);
}

/* Bijiben Inner HTML */

static void
process_bijiben_start_elem (BijiLazyDeserializer *self)
{
  BijiLazyDeserializerPrivate *priv = self->priv;
  const gchar *element_name;

  element_name = (const gchar *) xmlTextReaderConstName(priv->inner);

  if (g_strcmp0 (element_name, "note-content")==0)
    return;

  if (g_strcmp0 (element_name, "div")==0)
    priv->html = g_string_append (priv->html, "&#xA;");

  if (g_strcmp0 (element_name, "b")==0)
    priv->html = g_string_append (priv->html, "<b>");

  if (g_strcmp0 (element_name, "i")==0)
    priv->html = g_string_append (priv->html, "<i>");

  if (g_strcmp0 (element_name, "strike")==0)
    priv->html = g_string_append (priv->html, "<strike>");

  if (g_strcmp0 (element_name, "ul")==0)
    priv->html = g_string_append (priv->html, "<ul>");

  if (g_strcmp0 (element_name, "ol")==0)
    priv->html = g_string_append (priv->html, "<ol>");

  if (g_strcmp0 (element_name, "li")==0)
    priv->html = g_string_append (priv->html, "<li>");
}

static void
process_bijiben_end_elem (BijiLazyDeserializer *self)
{
  BijiLazyDeserializerPrivate *priv = self->priv;
  const gchar *element_name;
  
  element_name = (const gchar *) xmlTextReaderConstName (priv->inner);

  if (g_strcmp0 (element_name, "note-content")==0)
    return;

  if (g_strcmp0 (element_name, "b")==0)
    priv->html = g_string_append (priv->html, "</b>");

  if (g_strcmp0 (element_name, "i")==0)
    priv->html = g_string_append (priv->html, "</i>");

  if (g_strcmp0 (element_name, "strike")==0)
    priv->html = g_string_append (priv->html, "</strike>");

  if (g_strcmp0 (element_name, "ul")==0)
    priv->html = g_string_append (priv->html, "</ul>");

  if (g_strcmp0 (element_name, "ol")==0)
    priv->html = g_string_append (priv->html, "</ol>");

  if (g_strcmp0 (element_name, "li")==0)
    priv->html = g_string_append (priv->html, "</li>");
}

static void
process_bijiben_text_elem (BijiLazyDeserializer *self)
{
  gchar *text;
  BijiLazyDeserializerPrivate *priv = self->priv;

  text = (gchar *) xmlTextReaderConstValue (priv->inner);

  if (text)
  {
    /* Simply append the text to both raw & html */
    priv->html = g_string_append (priv->html, text);

    text = biji_str_replace (text, "&#xA;", "    ");
    priv->raw_text = g_string_append (priv->raw_text, text);
    
    g_free (text);
  }
}

static void
process_bijiben_node (BijiLazyDeserializer *self)
{
  int            type;
  const xmlChar *name ;
  BijiLazyDeserializerPrivate *priv = self->priv;

  type  = xmlTextReaderNodeType (priv->inner);
  name  = xmlTextReaderConstName (priv->inner);
  
  if (name == NULL)
    name = BAD_CAST "(NULL)";

  switch (type)
  {
    case XML_ELEMENT_NODE:
      process_bijiben_start_elem (self);
      break;

    case XML_ELEMENT_DECL:
      process_bijiben_end_elem (self);
      break;

    case XML_TEXT_NODE:
    case XML_DTD_NODE:
      process_bijiben_text_elem (self);
      break;
  }
}

static void
process_bijiben_html_content (BijiLazyDeserializer *self)
{
  BijiLazyDeserializerPrivate *priv = self->priv;
  int ret;
  gchar *revamped_html;

  priv->inner = xmlReaderForMemory (priv->content,
                                    strlen(priv->content),
                                    "", "UTF-8", 0);
  
  ret = xmlTextReaderRead (priv->inner);

  /* Make the GString grow as we read */
  while (ret == 1) 
  {
    process_bijiben_node (self);
    ret = xmlTextReaderRead (priv->inner);
  }

  /* Now the inner content is known, we can
   * assign note values and let deserialization work on last elements*/
  biji_note_obj_set_raw_text (priv->note, priv->raw_text->str);

  revamped_html = biji_str_replace (priv->html->str, "&amp;", "&");
  biji_note_obj_set_html_content (priv->note, revamped_html);
  g_free (revamped_html);
}

/* Common XML format for both Bijiben / Tomboy */

static void
processNode (BijiLazyDeserializer *self) 
{
  xmlTextReaderPtr r = self->priv->r;
  BijiNoteObj * n = self->priv->note;
  xmlChar   *name;
  GdkRGBA   *color;
  gchar     *tag;
  GString   *norm;
  gchar *debug;

  name = xmlTextReaderName (r);

  if ( g_strcmp0((gchar*)name,"title") == 0 )
    set_note_title(n, (gchar*) xmlTextReaderReadString(r));

  if ( g_strcmp0((gchar*)name,"text") == 0 )
  {
    if (self->priv->type == BIJIBEN_1)
    {
      self->priv->content = (gchar*) xmlTextReaderReadInnerXml (r);
      process_bijiben_html_content (self);
    }

    else if (self->priv->type == TOMBOY_1 ||
             self->priv->type == TOMBOY_2 ||
             self->priv->type == TOMBOY_3 )
    {
      self->priv->content = (gchar*) xmlTextReaderReadInnerXml (r);
      process_tomboy_xml_content (self);
    }
  }

  if (g_strcmp0 ((gchar*) name, "last-change-date") == 0)
    set_note_last_change_date (n,(gchar*) xmlTextReaderReadString (r));  

  if (g_strcmp0 ((gchar*) name, "last-metadata-change-date") == 0)
    set_note_last_metadata_change_date(n,(gchar*) xmlTextReaderReadString (r));

  if (g_strcmp0 ((gchar*) name, "create-date") == 0)
    set_note_create_date (n, (gchar*) xmlTextReaderReadString (r));

  if (g_strcmp0 ((gchar*) name, "color") == 0 )  
  {
    color = g_new (GdkRGBA,1);
    debug = (gchar*) xmlTextReaderReadString (r);

    if ( gdk_rgba_parse (color,debug))
    {
      biji_note_obj_set_rgba (n, color);
    }
    else
    {
      g_warning ("color invalid:%s",debug);
    }

  }

  if ( g_strcmp0((gchar*)name,"tag") == 0 )  
  {
    tag = (gchar*) xmlTextReaderReadString(r);

    if (g_str_has_prefix (tag,"system:template"))
    {
      note_obj_set_is_template(n,TRUE);
    }

    else if (g_str_has_prefix (tag,"system:notebook:"))
    {
      norm = g_string_new (tag);
      g_string_erase (norm,0,16);
      tag = g_string_free (norm,FALSE);
      _biji_note_obj_set_tags (n,g_list_append((GList*)_biji_note_obj_get_tags(n),
                                            (gpointer)tag));
    }
  }
}

static void
biji_parse_file (BijiLazyDeserializer *self)
{
  while ( xmlTextReaderRead(self->priv->r) == 1 )
  {
    if ( xmlTextReaderNodeType(self->priv->r) == 1 )
    {
      processNode(self);
    }
  }
}

gboolean
biji_lazy_deserialize_internal (BijiLazyDeserializer *self)
{
  BijiNoteObj* n = self->priv->note;
  xmlDocPtr doc;
  xmlNodePtr cur;
  xmlChar     *version; 

  doc = xmlParseFile (get_note_path (n));

  if (doc == NULL ) 
  {
    g_warning ("File not parsed successfully");
    return FALSE;
  }

  cur = xmlDocGetRootElement (doc);

  if (cur == NULL) 
  {
    g_warning ("File empty");
    xmlFreeDoc(doc);
    return FALSE;
  }

  if (xmlStrcmp(cur->name, (const xmlChar *) "note")) 
  {
    g_message ("document of the wrong type, root node != note");
    xmlFreeDoc(doc);
    return FALSE;
  }

  /* Switch there for note type
   * Despite not yet handled */

  version = xmlGetNoNsProp (cur, BAD_CAST "version");

  /* Bijiben type */
  if (g_strcmp0 ((gchar*) cur->ns->href, BIJI_NS) ==0) {
    self->priv->type = BIJIBEN_1;
  }

  /* Tomboy type */
  else {
    if (g_strcmp0 ((gchar*) cur->ns->href, TOMBOY_NS) == 0)
    {
      if (g_strcmp0 ((const gchar*) version, "0.1") == 0)
        self->priv->type = TOMBOY_1;

      if (g_strcmp0 ((const gchar*) version, "0.2") == 0)
        self->priv->type = TOMBOY_2;

      if (g_strcmp0 ((const gchar*) version, "0.3") == 0)
        self->priv->type = TOMBOY_3;
    }

  /* Wow this note won't be loaded i guess */
    else {
      self->priv->type = NO_TYPE;
    }
  }

  xmlFree (version);

  self->priv->r = xmlNewTextReaderFilename (get_note_path (n));
  biji_parse_file (self);
  xmlFreeDoc (doc);

  return TRUE ;
}

static BijiLazyDeserializer *
biji_lazy_deserializer_new (BijiNoteObj *note)
{
  return g_object_new (BIJI_TYPE_LAZY_DESERIALIZER,
                       "note", note,
                       NULL);
}

gboolean
biji_lazy_deserialize (BijiNoteObj *note)
{
  BijiLazyDeserializer *bld;
  gboolean result;

  bld = biji_lazy_deserializer_new (note);
  result = biji_lazy_deserialize_internal (bld);
  g_clear_object (&bld);

  return result;
}
