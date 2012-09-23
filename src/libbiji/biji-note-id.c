#include "biji-note-id.h"

struct _BijiNoteIDPrivate
{
  gchar * path ;
  gchar * title ;
  GTimeVal last_change_date;
  GTimeVal last_metadata_change_date;
  gchar * create_date ;
};

#define NOTE_OBJ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NOTE_TYPE_OBJ, NoteObjPrivate))

G_DEFINE_TYPE (BijiNoteID, biji_note_id, G_TYPE_OBJECT);

static void
biji_note_id_init (BijiNoteID *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, biji_note_id_get_type(),
                                            BijiNoteIDPrivate);
  self->priv->path = NULL;
  self->priv->create_date = NULL;
}

static void
biji_note_id_finalize (GObject *object)
{
  BijiNoteID *id = BIJI_NOTE_ID (object);

  g_free (id->priv->path);

  G_OBJECT_CLASS (biji_note_id_parent_class)->finalize (object);
}

static void
biji_note_id_class_init (BijiNoteIDClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = biji_note_id_finalize;
  g_type_class_add_private (klass, sizeof (BijiNoteIDPrivate));
}

//

gboolean 
_biji_note_id_are_same(BijiNoteID *a, BijiNoteID *b)
{
  gint result = g_strcmp0 (a->priv->title, b->priv->title) ;
        
  if ( result != 0 )
    return FALSE ;

  result = g_strcmp0 (a->priv->path, b->priv->path) ;

  if ( result != 0 )
    return FALSE ;

  return TRUE ;
}

gchar* 
biji_note_id_get_path(BijiNoteID* n)
{
  return n->priv->path ;
}

void
set_note_id_path(BijiNoteID* n,
                 const gchar* path)
{
  g_return_if_fail (BIJI_IS_NOTE_ID (n));

  g_free (n->priv->path);
  n->priv->path = g_strdup (path);
}

void _biji_note_id_set_title(BijiNoteID *n,gchar* title)
{
  n->priv->title = title ;
}

gchar* _biji_note_id_get_title(BijiNoteID* n)
{
  return n->priv->title ;
}

gchar * biji_note_id_get_last_change_date(BijiNoteID* n)
{
  return g_time_val_to_iso8601 (&(n->priv->last_change_date));
}

void
_biji_note_id_set_change_date_now(BijiNoteID *n)
{
  g_get_current_time(&(n->priv->last_change_date));
}

glong
biji_note_id_get_last_change_date_sec(BijiNoteID *n)
{
  return n->priv->last_change_date.tv_sec ;
}

static gboolean
set_date_from_string(gchar *iso8601,GTimeVal *date)
{
	if (g_time_val_from_iso8601(iso8601,date) == FALSE )
	{
			g_get_current_time(date);
			return FALSE;
	}
	return TRUE ;
}

int
set_note_id_last_change_date(BijiNoteID* n,gchar* date)
{
	set_date_from_string(date,&(n->priv->last_change_date));
	return 0 ;
}

gchar * biji_note_id_get_last_metadata_change_date(BijiNoteID* n)
{
	return g_time_val_to_iso8601 (&n->priv->last_metadata_change_date);
}

int
set_note_id_last_metadata_change_date(BijiNoteID* n,gchar* date)
{
	set_date_from_string(date,&(n->priv->last_metadata_change_date));
	return 0 ;
}

void
_biji_note_id_set_metadata_change_now(BijiNoteID *n)
{
  g_get_current_time(&(n->priv->last_metadata_change_date));
}

gchar * biji_note_id_get_create_date(BijiNoteID* n)
{
	return n->priv->create_date;
}

int
set_note_id_create_date(BijiNoteID* n,gchar* date)
{
	n->priv->create_date = date ;
	return 0 ;
}
