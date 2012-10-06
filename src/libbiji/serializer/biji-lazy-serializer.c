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

#include "biji-lazy-serializer.h"
#include "../libbiji.h"

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

  xmlBufferPtr buf;
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

gboolean
biji_lazy_serialize_internal (BijiLazySerializer *self)
{
  BijiLazySerializerPrivate *priv = self->priv;
  BijiNoteID                *id;
  gchar                     *html;
  GList                     *tags;
  GdkRGBA                   color;
  xmlTextWriterPtr          writer;

  id = note_get_id (priv->note);
  writer = xmlNewTextWriterMemory(priv->buf, 0);

  // Header
  xmlTextWriterStartDocument (writer,"1.0","utf-8",NULL);

  xmlTextWriterStartElement (writer, BAD_CAST "note");
  xmlTextWriterWriteAttributeNS (writer, NULL, 
                                 BAD_CAST "version",NULL, 
                                 BAD_CAST "1");
  xmlTextWriterWriteAttributeNS (writer, BAD_CAST "xmlns",
                                 BAD_CAST "link", NULL, 
                                 BAD_CAST "http://projects.gnome.org/bijiben/link");
  xmlTextWriterWriteAttributeNS (writer, BAD_CAST "xmlns", BAD_CAST "size", NULL,
                                 BAD_CAST "http://projects.gnome.org/bijiben/size");
  xmlTextWriterWriteAttributeNS (writer, NULL, BAD_CAST "xmlns", NULL, 
                                 BAD_CAST "http://projects.gnome.org/bijiben");

  // <Title>
  serialize_node (writer, "title", biji_note_get_title (priv->note));

  // <text> 
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "text");
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xml",
                                BAD_CAST "space", NULL, 
                                BAD_CAST "preserve");

  // <note-content>
  xmlTextWriterStartElement(writer, BAD_CAST "note-content");
  html = biji_note_obj_get_html (priv->note);

  if (html)
  {
    // Sanitize html, just replace non Xml char with something else
    html = biji_str_mass_replace (html, "&nbsp;", " ",
                                        "<br>", "&#xA;",
                                        "<div>", "&#xA;",
                                        "</div>", "",
                                        "&", "&amp;", NULL);

    xmlTextWriterWriteRaw(writer, BAD_CAST (html));
    g_free (html);
  }

  xmlTextWriterEndElement (writer);

  // </text>  
  xmlTextWriterEndElement(writer);

  // <last-change-date>
  serialize_node (writer, "last-change-date",
                  biji_note_id_get_last_change_date(id));

  serialize_node (writer, "last-metadata-change-date",
                  biji_note_id_get_last_metadata_change_date(id));

  serialize_node (writer, "create-date",
                  biji_note_id_get_create_date(id));

  serialize_node (writer, "cursor-position", "0");
  serialize_node (writer, "selection-bound-position", "0");
  serialize_node (writer, "width", "0");
  serialize_node (writer, "height", "0");
  serialize_node (writer, "x", "0");
  serialize_node (writer, "y", "0");
  
  if (biji_note_obj_get_rgba (priv->note, &color))
    serialize_node (writer, "color", gdk_rgba_to_string (&color));

  //<tags>
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n ");
  xmlTextWriterStartElement (writer, BAD_CAST "tags");
  tags = _biji_note_obj_get_tags (priv->note);
  g_list_foreach (tags, (GFunc) serialize_tags, writer);
  xmlTextWriterEndElement (writer);
  g_list_free (tags);

  // <open-on-startup>
  serialize_node (writer, "open-on-startup", "False");

  // <note>
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n ");
  xmlTextWriterEndElement(writer);

  xmlFreeTextWriter(writer);

  return g_file_set_contents (biji_note_id_get_path(id),
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

