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

#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <string.h>

#include "biji-lazy-serializer.h"
#include "../biji-note-obj.h"
#include "../biji-string.h"

enum
{
  PROP_0,
  PROP_NOTE,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

G_DEFINE_TYPE (BijiLazySerializer, biji_lazy_serializer, G_TYPE_OBJECT);

struct _BijiLazySerializerPrivate
{
  BijiNoteObj *note;

  xmlBufferPtr     buf;
  xmlTextWriterPtr writer;

  /* To get accross the html tree */
  xmlTextReaderPtr inner;
};

static void
biji_lazy_serializer_get_property (GObject  *object,
                                   guint     property_id,
                                   GValue   *value,
                                   GParamSpec *pspec)
{
  BijiLazySerializer *self = BIJI_LAZY_SERIALIZER (object);

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
biji_lazy_serializer_set_property (GObject  *object,
                                   guint     property_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  BijiLazySerializer *self = BIJI_LAZY_SERIALIZER (object);

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
biji_lazy_serializer_init (BijiLazySerializer *self)
{
  BijiLazySerializerPrivate *priv;

  priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                      BIJI_TYPE_LAZY_SERIALIZER,
                                      BijiLazySerializerPrivate);
  self->priv = priv;

  priv->buf = xmlBufferCreate();
}

static void
biji_lazy_serializer_finalize (GObject *object)
{
  BijiLazySerializer *self = BIJI_LAZY_SERIALIZER (object);
  BijiLazySerializerPrivate *priv = self->priv;

  xmlBufferFree (priv->buf);
  xmlFreeTextReader (priv->inner);

  G_OBJECT_CLASS (biji_lazy_serializer_parent_class)->finalize (object);
}

static void
biji_lazy_serializer_class_init (BijiLazySerializerClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = biji_lazy_serializer_get_property;
  object_class->set_property = biji_lazy_serializer_set_property;
  object_class->finalize = biji_lazy_serializer_finalize;

  properties[PROP_NOTE] = g_param_spec_object ("note",
                                               "Note",
                                               "Biji Note Obj",
                                               BIJI_TYPE_NOTE_OBJ,
                                               G_PARAM_READWRITE  |
                                               G_PARAM_CONSTRUCT |
                                               G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_NOTE,properties[PROP_NOTE]);

  g_type_class_add_private (klass, sizeof (BijiLazySerializerPrivate));
}

static void
serialize_node (xmlTextWriterPtr writer, gchar *node, gchar *value)
{
  xmlTextWriterWriteRaw     (writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement (writer, BAD_CAST node);
  xmlTextWriterWriteString  (writer,BAD_CAST value);
  xmlTextWriterEndElement   (writer);
}

static void
serialize_tags (gchar *tag, xmlTextWriterPtr writer)
{
  gchar *string;

  string = g_strdup_printf("system:notebook:%s", tag);
  serialize_node (writer, "tag", string);

  g_free (string);
}

static void
process_text_elem (BijiLazySerializer *self)
{
  BijiLazySerializerPrivate *priv = self->priv;

  xmlTextWriterWriteRaw (priv->writer, xmlTextReaderConstValue (priv->inner));
}

// TODO & check : ul li ol image
static void
process_start_elem (BijiLazySerializer *self)
{
  BijiLazySerializerPrivate *priv = self->priv;
  const gchar *name;

  name = (const gchar *) xmlTextReaderConstName(priv->inner);

  if (g_strcmp0 (name, "b")==0)
    xmlTextWriterStartElement (priv->writer, BAD_CAST "b");

  if (g_strcmp0 (name, "i")==0)
    xmlTextWriterStartElement (priv->writer, BAD_CAST "i");

  if (g_strcmp0 (name, "strike")==0)
    xmlTextWriterStartElement (priv->writer, BAD_CAST "strike");

  /* Do not serialize div. Close br. Everything is <br/>. Simpler. */
  if (g_strcmp0 (name, "div")== 0 || g_strcmp0 (name, "br") == 0)
  {
    xmlTextWriterStartElement (priv->writer, BAD_CAST "br");
    xmlTextWriterEndElement (priv->writer);
  }

  if (g_strcmp0 (name, "ul")==0)
    xmlTextWriterStartElement (priv->writer, BAD_CAST "ul");

  if (g_strcmp0 (name, "ol")==0)
    xmlTextWriterStartElement (priv->writer, BAD_CAST "ol");

  if (g_strcmp0 (name, "li")==0)
    xmlTextWriterStartElement (priv->writer, BAD_CAST "li");
}

static void
process_end_elem (BijiLazySerializer *self)
{
  BijiLazySerializerPrivate *priv = self->priv;
  const gchar *element_name;

  element_name = (const gchar *) xmlTextReaderConstName(priv->inner);

  if (g_strcmp0 (element_name, "b")==0)
    xmlTextWriterEndElement (priv->writer);

  if (g_strcmp0 (element_name, "i")==0)
    xmlTextWriterEndElement (priv->writer);

  if (g_strcmp0 (element_name, "strike")==0)
    xmlTextWriterEndElement (priv->writer);

  if (g_strcmp0 (element_name, "ul")==0)
    xmlTextWriterEndElement (priv->writer);

  if (g_strcmp0 (element_name, "ol")==0)
    xmlTextWriterEndElement (priv->writer);

  if (g_strcmp0 (element_name, "li")==0)
    xmlTextWriterEndElement (priv->writer);
}

/* Webkit html is _not_ xhtml, but we need valid xml here.
 * Thus some start elem are to be manually close.
 * until XHTML rules the world. or until we use .html note format... */
static void
serialize_html (BijiLazySerializer *self)
{
  BijiLazySerializerPrivate *priv = self->priv;
  gchar *html = biji_note_obj_get_html (priv->note);

  if (!html)
    return;

  /* We need a start & end node to obtain a Xml reader
   * and we need to suffer html = recover from errors */
  GString * noded_html = g_string_new ("<bijihtml>");
  noded_html = g_string_append (noded_html, html);
  noded_html = g_string_append (noded_html, "</bijihtml>");

  priv->inner = xmlReaderForMemory (noded_html->str,
                                    strlen(noded_html->str),
                                    "", "UTF-8", XML_PARSE_RECOVER);

  while (xmlTextReaderRead (priv->inner) ==1)
  {
    gint type = xmlTextReaderNodeType (priv->inner);
    const xmlChar *name = xmlTextReaderConstName (priv->inner);

    if (!name)
      continue;

    switch (type)
    {
      case XML_ELEMENT_NODE:
        process_start_elem (self);
        break;

      case XML_ELEMENT_DECL:
        process_end_elem (self);
        break;

      case XML_TEXT_NODE:
        process_text_elem (self);
        break;

      case XML_DTD_NODE:
        process_text_elem (self);
        break;
    }
  }

  g_string_free (noded_html, TRUE);
}

gboolean
biji_lazy_serialize_internal (BijiLazySerializer *self)
{
  BijiLazySerializerPrivate *priv = self->priv;
  GList                     *tags;
  GdkRGBA                   color;

  priv->writer = xmlNewTextWriterMemory(priv->buf, 0);

  // Header
  xmlTextWriterStartDocument (priv->writer,"1.0","utf-8",NULL);

  xmlTextWriterStartElement (priv->writer, BAD_CAST "note");
  xmlTextWriterWriteAttributeNS (priv->writer, NULL, 
                                 BAD_CAST "version",NULL, 
                                 BAD_CAST "1");
  xmlTextWriterWriteAttributeNS (priv->writer, BAD_CAST "xmlns",
                                 BAD_CAST "link", NULL, 
                                 BAD_CAST "http://projects.gnome.org/bijiben/link");
  xmlTextWriterWriteAttributeNS (priv->writer, BAD_CAST "xmlns", BAD_CAST "size", NULL,
                                 BAD_CAST "http://projects.gnome.org/bijiben/size");
  xmlTextWriterWriteAttributeNS (priv->writer, NULL, BAD_CAST "xmlns", NULL, 
                                 BAD_CAST "http://projects.gnome.org/bijiben");

  // <Title>
  serialize_node (priv->writer, "title", biji_note_get_title (priv->note));

  // <text> 
  xmlTextWriterWriteRaw(priv->writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(priv->writer, BAD_CAST "text");
  xmlTextWriterWriteAttributeNS(priv->writer, BAD_CAST "xml",
                                BAD_CAST "space", NULL, 
                                BAD_CAST "preserve");

  // <note-content>
  xmlTextWriterStartElement(priv->writer, BAD_CAST "note-content");
  serialize_html (self);
  xmlTextWriterEndElement (priv->writer);

  // </text>  
  xmlTextWriterEndElement(priv->writer);

  // <last-change-date>
  serialize_node (priv->writer, "last-change-date",
                  biji_note_obj_get_last_change_date (priv->note));

  serialize_node (priv->writer, "last-metadata-change-date",
                  biji_note_obj_get_last_metadata_change_date(priv->note));

  serialize_node (priv->writer, "create-date",
                  biji_note_obj_get_create_date (priv->note));

  serialize_node (priv->writer, "cursor-position", "0");
  serialize_node (priv->writer, "selection-bound-position", "0");
  serialize_node (priv->writer, "width", "0");
  serialize_node (priv->writer, "height", "0");
  serialize_node (priv->writer, "x", "0");
  serialize_node (priv->writer, "y", "0");
  
  if (biji_note_obj_get_rgba (priv->note, &color))
    serialize_node (priv->writer, "color", gdk_rgba_to_string (&color));

  //<tags>
  xmlTextWriterWriteRaw(priv->writer, BAD_CAST "\n ");
  xmlTextWriterStartElement (priv->writer, BAD_CAST "tags");
  tags = _biji_note_obj_get_tags (priv->note);
  g_list_foreach (tags, (GFunc) serialize_tags, priv->writer);
  xmlTextWriterEndElement (priv->writer);
  g_list_free (tags);

  // <open-on-startup>
  serialize_node (priv->writer, "open-on-startup", "False");

  // <note>
  xmlTextWriterWriteRaw(priv->writer, BAD_CAST "\n ");
  xmlTextWriterEndElement(priv->writer);

  xmlFreeTextWriter(priv->writer);

  g_warning ("content:%s", (gchar*) priv->buf->content);

  return g_file_set_contents (biji_note_obj_get_path (priv->note),
                              (gchar*) priv->buf->content,
                              -1,
                              NULL);
}

gboolean
biji_lazy_serialize (BijiNoteObj *note)
{
  BijiLazySerializer *self;
  gboolean result ;

  self = g_object_new (BIJI_TYPE_LAZY_SERIALIZER,
                       "note", note, NULL);
  result = biji_lazy_serialize_internal (self);
  g_object_unref (self);

  return result;
}

