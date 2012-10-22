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

struct _BijiNoteIDPrivate
{
  gchar * path ;
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
  self->priv->path = NULL;
  self->priv->title = NULL;
}

static void
biji_note_id_finalize (GObject *object)
{
  BijiNoteID *id = BIJI_NOTE_ID (object);
  BijiNoteIDPrivate *priv = id->priv;

  g_free (priv->path);
  g_free (priv->title);

  G_OBJECT_CLASS (biji_note_id_parent_class)->finalize (object);
}

static void
biji_note_id_class_init (BijiNoteIDClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = biji_note_id_finalize;
  g_type_class_add_private (klass, sizeof (BijiNoteIDPrivate));
}

gboolean 
biji_note_id_equal (BijiNoteID *a, BijiNoteID *b)
{
  gint result = g_strcmp0 (a->priv->title, b->priv->title) ;

  if ( result != 0 )
    return FALSE ;

  result = g_strcmp0 (a->priv->path, b->priv->path) ;

  if ( result != 0 )
    return FALSE ;

  return TRUE ;
}

gchar * 
biji_note_id_get_path(BijiNoteID* n)
{
  return n->priv->path ;
}

void
biji_note_id_set_path (BijiNoteID* n, const gchar* path)
{
  g_return_if_fail (BIJI_IS_NOTE_ID (n));

  if (n->priv->path)
    g_free (n->priv->path);

  n->priv->path = g_strdup (path);
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
