/* bjb-note-id.c
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

#include "biji-note-id.h"

/* Properties */
enum {
  PROP_0,
  PROP_PATH,
  BIJI_ID_PROPERTIES
};

static GParamSpec *properties[BIJI_ID_PROPERTIES] = { NULL, };

struct _BijiNoteIDPrivate
{
  GFile * location;
  gchar * title ;

  GTimeVal last_change_date;
  GTimeVal last_metadata_change_date;
  GTimeVal create_date ;
};

#define NOTE_OBJ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NOTE_TYPE_OBJ, NoteObjPrivate))

G_DEFINE_TYPE (BijiNoteID, biji_note_id, G_TYPE_OBJECT);

static void
biji_note_id_init (BijiNoteID *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, biji_note_id_get_type(),
                                            BijiNoteIDPrivate);

  self->priv->location = NULL;
  self->priv->title = NULL;
}

static void
biji_note_id_finalize (GObject *object)
{
  BijiNoteID *id = BIJI_NOTE_ID (object);
  BijiNoteIDPrivate *priv = id->priv;

  g_object_unref (priv->location);
  g_free (priv->title);

  G_OBJECT_CLASS (biji_note_id_parent_class)->finalize (object);
}

static void
biji_note_id_set_path (BijiNoteID *self, const gchar *path)
{
  g_warn_if_fail (!self->priv->location);
  self->priv->location = g_file_new_for_path (path);
}

static void
biji_note_id_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  BijiNoteID *self = BIJI_NOTE_ID (object);


  switch (property_id)
    {
    case PROP_PATH:
      biji_note_id_set_path (self,g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
biji_note_id_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  BijiNoteID *self = BIJI_NOTE_ID (object);

  switch (property_id)
    {
    case PROP_PATH:
      g_value_set_object (value, g_file_get_basename (self->priv->location));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
biji_note_id_class_init (BijiNoteIDClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = biji_note_id_finalize;
  object_class->get_property = biji_note_id_get_property;
  object_class->set_property = biji_note_id_set_property;

  properties[PROP_PATH] =
    g_param_spec_string("path",
                        "The note file",
                        "The location where the note is stored and saved",
                        NULL,
                        G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, BIJI_ID_PROPERTIES, properties);

  g_type_class_add_private (klass, sizeof (BijiNoteIDPrivate));
}

gboolean
biji_note_id_equal (BijiNoteID *a, BijiNoteID *b)
{
  return g_file_equal (a->priv->location, b->priv->location);
}

gchar * 
biji_note_id_get_path (BijiNoteID* n)
{
  return g_file_get_path (n->priv->location);
}

gchar *
biji_note_id_get_uuid (BijiNoteID *n)
{
  return g_file_get_basename (n->priv->location);
}

GFile *
biji_note_id_get_file (BijiNoteID *note)
{
  return note->priv->location;
}

void
biji_note_id_set_title  (BijiNoteID *n, gchar* title)
{
  n->priv->title = g_strdup (title);
}

gchar *
biji_note_id_get_title (BijiNoteID* n)
{
  return n->priv->title ;
}

static gboolean
set_date_from_string (gchar *iso8601, GTimeVal *date)
{
  g_return_val_if_fail (iso8601, FALSE);
  g_return_val_if_fail (date, FALSE);

  if (!g_time_val_from_iso8601 (iso8601, date))
  {
    g_get_current_time (date);
    return FALSE;
  }

  return TRUE;
}

gchar *
biji_note_id_get_last_change_date (BijiNoteID* n)
{
  return g_time_val_to_iso8601 (&(n->priv->last_change_date));
}

void
biji_note_id_set_last_change_date_now (BijiNoteID *n)
{
  g_get_current_time(&(n->priv->last_change_date));
}

glong
biji_note_id_get_last_change_date_sec (BijiNoteID *n)
{
  g_return_val_if_fail (BIJI_IS_NOTE_ID (n), 0);
  
  return n->priv->last_change_date.tv_sec ;
}

gboolean
biji_note_id_set_last_change_date (BijiNoteID* n,gchar* date)
{
  return set_date_from_string(date,&(n->priv->last_change_date));
}

gchar *
biji_note_id_get_last_metadata_change_date(BijiNoteID* n)
{
  return g_time_val_to_iso8601 (&n->priv->last_metadata_change_date);
}

gboolean
biji_note_id_set_last_metadata_change_date (BijiNoteID* n,gchar* date)
{
  return set_date_from_string(date,&(n->priv->last_metadata_change_date));
}

void
biji_note_id_set_last_metadata_change_date_now (BijiNoteID *n)
{
  g_get_current_time(&(n->priv->last_metadata_change_date));
}

gchar *
biji_note_id_get_create_date(BijiNoteID* n)
{
  return g_time_val_to_iso8601 (&n->priv->create_date);
}

gboolean
biji_note_id_set_create_date (BijiNoteID* n,gchar* date)
{
  return set_date_from_string (date, &(n->priv->create_date));
}

void
biji_note_id_set_create_date_now (BijiNoteID* n)
{
  g_get_current_time (&(n->priv->create_date));
}
