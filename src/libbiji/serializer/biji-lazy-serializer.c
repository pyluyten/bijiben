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
#include "../biji-note-id.h"

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
  xmlTextWriterPtr writer;
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
  /* create the xmlBuffer */
  priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                      BIJI_TYPE_LAZY_SERIALIZER,
                                      BijiLazySerializerPrivate);
  self->priv = priv;

  priv->buf = xmlBufferCreate();
  priv->writer = xmlNewTextWriterMemory(priv->buf, 0);
}

static void
biji_lazy_serializer_finalize (GObject *object)
{
  BijiLazySerializer *self = BIJI_LAZY_SERIALIZER (object);
  BijiLazySerializerPrivate *priv = self->priv;

  xmlBufferFree (priv->buf);  
  xmlFreeTextWriter(priv->writer);

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

BijiLazySerializer *
biji_lazy_serializer_new (BijiNoteObj *note)
{
  return g_object_new (BIJI_TYPE_LAZY_SERIALIZER,
                       "note", note,
                       NULL);
}

gboolean
biji_lazy_serialize (BijiLazySerializer *self)
{
  BijiLazySerializerPrivate *priv = self->priv;
  xmlTextWriterPtr writer = priv->writer;
  BijiNoteID *id;

  id = note_get_id (priv->note);

  /* Header */
  xmlTextWriterStartDocument (writer,"1.0","utf-8",NULL);

  xmlTextWriterStartElement (writer, BAD_CAST "note");
  xmlTextWriterWriteAttributeNS (writer, NULL, 
                                 BAD_CAST "version",NULL, 
                                 BAD_CAST "0.4");
  xmlTextWriterWriteAttributeNS (writer, BAD_CAST "xmlns",
                                 BAD_CAST "link", NULL, 
                                 BAD_CAST "http://projects.gnome.org/bijiben/link");
  xmlTextWriterWriteAttributeNS (writer, BAD_CAST "xmlns", BAD_CAST "size", NULL,
                                 BAD_CAST "http://projects.gnome.org/bijiben/size");
  xmlTextWriterWriteAttributeNS (writer, NULL, BAD_CAST "xmlns", NULL, 
                                 BAD_CAST "http://projects.gnome.org/bijiben");

  /* <Title> */
  xmlTextWriterWriteRaw (writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement (writer, BAD_CAST "title");
  xmlTextWriterWriteString (writer,
                           BAD_CAST biji_note_get_title (priv->note));
  xmlTextWriterEndElement (writer);

  /* <text> */
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "text");
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xml",
                                BAD_CAST "space", NULL, 
                                BAD_CAST "preserve");

  /* TODO : note content
   * and end metadata */

  /* <note> */
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n ");
  xmlTextWriterEndElement(writer);

  return g_file_set_contents (biji_note_id_get_path(id),
                              (gchar*) priv->buf->content,
                              -1,
                              NULL);
}
