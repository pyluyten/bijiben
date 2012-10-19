/* bjb-note-book.c
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

#include <gtk/gtk.h>
#include <uuid/uuid.h>

#include "libbiji.h"
#include "biji-note-book.h"
#include "deserializer/biji-lazy-deserializer.h"


struct _BijiNoteBookPrivate
{
  /* Notes & TagBooks */
  GHashTable *notes;
  GList *tags;

  /* Signals */
  gulong note_renamed ;

  GFile *location;
  GCancellable *load_cancellable;
};

/* Properties */
enum {
  PROP_0,
  PROP_LOCATION,
  BIJI_BOOK_PROPERTIES
};

/* Signals */
enum {
  BOOK_AMENDED,
  BIJI_BOOK_SIGNALS
};

static guint biji_book_signals[BIJI_BOOK_SIGNALS] = { 0 };
static GParamSpec *properties[BIJI_BOOK_PROPERTIES] = { NULL, };

#define BIJI_NOTE_BOOK_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BIJI_TYPE_NOTE_BOOK, BijiNoteBookPrivate))

G_DEFINE_TYPE (BijiNoteBook, biji_note_book, G_TYPE_OBJECT);

static void
biji_note_book_init (BijiNoteBook *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BIJI_TYPE_NOTE_BOOK,
                                            BijiNoteBookPrivate);

  /* Note path is key for table. It's freed by note itself */
  self->priv->notes = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             NULL,
                                             g_object_unref);
  self->priv->tags = NULL;
}

static void
biji_note_book_finalize (GObject *object)
{
  BijiNoteBook *book = BIJI_NOTE_BOOK (object) ;

  if (book->priv->load_cancellable)
    g_cancellable_cancel (book->priv->load_cancellable);

  g_clear_object (&book->priv->load_cancellable);
  g_clear_object (&book->priv->location);

  g_list_free (book->priv->tags); // g_list_free_full ?
  g_hash_table_destroy (book->priv->notes);

  G_OBJECT_CLASS (biji_note_book_parent_class)->finalize (object);
}

static void
biji_note_book_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  BijiNoteBook *self = BIJI_NOTE_BOOK (object);


  switch (property_id)
    {
    case PROP_LOCATION:
      self->priv->location = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
biji_note_book_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  BijiNoteBook *self = BIJI_NOTE_BOOK (object);

  switch (property_id)
    {
    case PROP_LOCATION:
      g_value_set_object (value, self->priv->location);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

/* tag books are a trial. not really necessary, but make life easier*/

static void
biji_note_book_add_tag_book(BijiNoteBook *book, TagBook *tag)
{
  book->priv->tags = g_list_append(book->priv->tags,tag);
}

static TagBook *
biji_tag_book_new(gchar *name)
{
  TagBook *result =  (TagBook*) g_malloc0 ( sizeof(TagBook) ) ;

  result->name = name ;
  result->notes = NULL ;
  result->length = 0 ;

  return result ;
}

static TagBook *
biji_note_book_get_tag_book(BijiNoteBook *book, gchar *tag)
{
  gint i;
  for (i=0 ; i < g_list_length(book->priv->tags) ; i++ )
  {
    TagBook *iter = g_list_nth_data (book->priv->tags,i) ;
      
    if ( g_strcmp0 (iter->name , tag) == 0 )
      return iter ;
  }

  return NULL ;
}

// Return the book or create and return a brand new book
static TagBook *
biji_book_get_or_create_tag_book(BijiNoteBook *book, gchar *tag)
{
  TagBook *result = biji_note_book_get_tag_book(book,tag);

  if ( result == NULL )
  {
    result = biji_tag_book_new(tag);
    biji_note_book_add_tag_book(book,result);
  }

  return result ;
}

static void
_biji_note_book_sanitize_title(BijiNoteBook *book,BijiNoteObj *note)
{   
  gchar *title, *new_title ;
  gint suffix ;
    
  /* Find a proper title
   * We start with adding 2 to non unique titles */
  suffix = 2 ;
  title = biji_note_obj_get_title(note);
  new_title = g_strdup(title);

  /* Test title untill we find something unique.
   * Block signal because there is loop */
  while (_biji_note_book_is_title_unique(book,new_title) == FALSE )
  {
    
    g_free(new_title);
    new_title = g_strdup_printf("%s (%i)", title, suffix);
    suffix++;
  }

  /* We also have to know if note has been renamed 
   * So we rename only if necessary */
  if ( new_title == title )
  {
    g_free(new_title) ;
    return ;
  }

  else 
  {
   biji_note_obj_set_title(note,new_title);
    g_free(title);
    return ;
  }
}

static gboolean
notify_changed(BijiNoteObj *note, BijiNoteBook *book)
{
  g_signal_emit ( G_OBJECT (book), biji_book_signals[BOOK_AMENDED],0);
  return FALSE ;
}

static void
_biji_note_book_add_one_note(BijiNoteBook *book,BijiNoteObj *note)
{
  g_return_if_fail(BIJI_IS_NOTE_OBJ(note));
  gint i ; 

  _biji_note_book_sanitize_title(book,note);

  /* Welcome to the book ! */
  _biji_note_obj_set_book(note,(gpointer)book);

  // Handle tags
  GList *note_tags = _biji_note_obj_get_tags(note) ;
  for ( i=0 ; i<g_list_length (note_tags) ; i++ )
  {  
    TagBook *tag_book;
    tag_book = biji_book_get_or_create_tag_book(book,
                                                g_list_nth_data (note_tags,i));

    // Handle template
    if ( note_obj_is_template(note) )
    {
      tag_book->template_note = note ;
    }

    else 
    {
      tag_book->notes = g_list_append(tag_book->notes,note) ;
      tag_book->length ++ ;
    }
  }

  // Add it to the list and emit signal
  g_hash_table_insert (book->priv->notes,
                       biji_note_obj_get_path (note), note);

  book->priv->note_renamed = g_signal_connect(note,"renamed",
                                              G_CALLBACK(notify_changed),book);
  g_signal_connect (note,"changed", G_CALLBACK(notify_changed),book);
    
  //TODO g_signal_connect(note,"deleted"...) ;
  g_signal_emit ( G_OBJECT (book), biji_book_signals[BOOK_AMENDED],0);
}

#define ATTRIBUTES_FOR_NOTEBOOK "standard::content-type,standard::name"

static void
load_location_error (GFile *location,
                     GError *error)
{
  gchar *path = g_file_get_path (location);
  g_printerr ("Unable to load location %s: %s", path, error->message);

  g_free (path);
  g_error_free (error);
}

static void
enumerate_next_files_ready_cb (GObject *source,
                               GAsyncResult *res,
                               gpointer user_data)
{
  GFileEnumerator *enumerator = G_FILE_ENUMERATOR (source);
  BijiNoteBook *self;
  GList *files, *l;
  GError *error = NULL;
  gchar *base_path;

  files = g_file_enumerator_next_files_finish (enumerator, res, &error);

  if (error != NULL)
    {
      load_location_error (g_file_enumerator_get_container (enumerator), error);
      return;
    }

  self = user_data;
  base_path = g_file_get_path (self->priv->location);

  // now load the notes
  for (l = files; l != NULL; l = l->next)
    {
      GFileInfo *info;
      const gchar *name;
      gchar *path;
      BijiNoteObj *note;

      info = l->data;
      name = g_file_info_get_name (info);

      if (!g_str_has_suffix (name, ".note"))
        continue;

      path = g_build_filename (base_path, name, NULL);
      note = biji_note_get_new_from_file (path);

      _biji_note_book_add_one_note (self, note);

      g_free (path);
    }

  g_free (base_path);
  g_list_free_full (files, g_object_unref);
}

static void
enumerate_children_ready_cb (GObject *source,
                             GAsyncResult *res,
                             gpointer user_data)
{
  GFile *location = G_FILE (source);
  GFileEnumerator *enumerator;
  GError *error = NULL;
  BijiNoteBook *self;

  enumerator = g_file_enumerate_children_finish (location,
                                                 res, &error);

  if (error != NULL)
    {
      load_location_error (location, error);
      return;
    }

  self = user_data;

  // enumerate all files
  g_file_enumerator_next_files_async (enumerator, G_MAXINT,
                                      G_PRIORITY_DEFAULT,
                                      self->priv->load_cancellable,
                                      enumerate_next_files_ready_cb,
                                      self);
}

static void
note_book_load_from_location (BijiNoteBook *self)
{
  self->priv->load_cancellable = g_cancellable_new ();
  g_file_enumerate_children_async (self->priv->location,
                                   ATTRIBUTES_FOR_NOTEBOOK, 0,
                                   G_PRIORITY_DEFAULT,
                                   self->priv->load_cancellable,
                                   enumerate_children_ready_cb,
                                   self);
}

static void
biji_note_book_constructed (GObject *object)
{
  BijiNoteBook *self = BIJI_NOTE_BOOK (object);

  G_OBJECT_CLASS (biji_note_book_parent_class)->constructed (object);

  note_book_load_from_location (self);
}

static void
biji_note_book_class_init (BijiNoteBookClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = biji_note_book_finalize;
  object_class->constructed = biji_note_book_constructed;
  object_class->set_property = biji_note_book_set_property;
  object_class->get_property = biji_note_book_get_property;

  biji_book_signals[BOOK_AMENDED] = g_signal_new ( "changed" ,
                                                   G_OBJECT_CLASS_TYPE (klass),
                                                   G_SIGNAL_RUN_LAST,
                                                   0, NULL, NULL,
                                                   g_cclosure_marshal_VOID__VOID,
                                                   G_TYPE_NONE,
                                                   0);
  properties[PROP_LOCATION] =
    g_param_spec_object("location",
                        "The book location",
                        "The location where the notes are loaded and saved",
                        G_TYPE_FILE,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, BIJI_BOOK_PROPERTIES, properties);
  g_type_class_add_private (klass, sizeof (BijiNoteBookPrivate));
}

gboolean
_biji_note_book_is_title_unique(BijiNoteBook *book,gchar *title)
{
  gint i;
  gboolean result = TRUE;
  BijiNoteObj *iter;
  GList *notes = g_hash_table_get_values (book->priv->notes);

  for ( i=0 ; i < g_hash_table_size (book->priv->notes) ; i++)
  {
    iter = BIJI_NOTE_OBJ (g_list_nth_data (notes, i));

    if (g_strcmp0 (biji_note_obj_get_title (iter), title) == 0)
     result = FALSE;
  }

  g_list_free (notes);
  return result;
}

void
_biji_note_book_add_note_to_tag_book(BijiNoteBook *book,
                                     BijiNoteObj *note,
                                     gchar *tag)
{
  TagBook *tag_book;
  tag_book = biji_book_get_or_create_tag_book(book,tag);
  tag_book->notes = g_list_append(tag_book->notes,note);
}

gboolean 
_note_book_remove_one_note(BijiNoteBook *book,BijiNoteObj *note)
{
  BijiNoteObj *to_delete = NULL;

  to_delete = g_hash_table_lookup (book->priv->notes,
                                  biji_note_obj_get_path(note));

  if (to_delete)
  {
    /* Ref note first, hash_table won't finalize it & we can delete it*/
    g_object_ref (to_delete);
    g_hash_table_remove (book->priv->notes, biji_note_obj_get_path (note));
    biji_note_obj_delete (note);
    g_signal_emit ( G_OBJECT (book), biji_book_signals[BOOK_AMENDED],0);
    return TRUE;
  }

  return FALSE;
}

GList *
_biji_note_book_get_notes (BijiNoteBook *book)
{
  return g_hash_table_get_values (book->priv->notes);
}

static void
add_note_to_list_if_tag_prefix(BijiNoteObj *note,TagBook *booklet)
{
  if (  _biji_note_obj_has_tag_prefix(note,booklet->name) )
  {
    booklet->notes = g_list_append(booklet->notes,note);
  }
}

GList * 
_biji_note_book_get_notes_with_tag(BijiNoteBook *book,gchar *tag)
{
  TagBook *tag_book = biji_book_get_or_create_tag_book(book,tag);
  return tag_book->notes ;
}

GList * 
_biji_note_book_get_notes_with_tag_prefix(BijiNoteBook *book,gchar *tag)
{
  // we do create a pseudo tag book for convenience...
  TagBook booklet ;
  booklet.name = tag ;
  booklet.notes = NULL ;

  GList *notes;
  notes = g_hash_table_get_values (book->priv->notes);

  g_list_foreach (notes,(GFunc)add_note_to_list_if_tag_prefix,&booklet);
  g_list_free (notes);
  return booklet.notes ;
}

static void
add_note_to_list_if_no_tag (BijiNoteObj *note, GList **notes)
{
  if ( _biji_note_obj_get_tags(note) == NULL )
  {
    *(notes) = g_list_append(*(notes),note);
  }
}

GList *
_biji_note_book_get_no_tag_notes(BijiNoteBook *book)
{
  GList *result = NULL ;
  GList *notes = g_hash_table_get_values (book->priv->notes);

  g_list_foreach (notes, (GFunc) add_note_to_list_if_no_tag, &result);
  g_list_free (notes);
  return result;
}

/* TODO : remove this & see what needs to be updated */
BijiNoteObj * 
_biji_book_get_nth(BijiNoteBook *book,int i)
{
  GList *notes = g_hash_table_get_values (book->priv->notes);
  BijiNoteObj *result = g_list_nth_data (notes, i);
  g_list_free (notes);

  return result;
}

/* Notes collection */
void note_book_append_new_note(BijiNoteBook *book,BijiNoteObj *note)
{    
  if (BIJI_IS_NOTE_BOOK(book) && BIJI_IS_NOTE_OBJ(note))
    _biji_note_book_add_one_note (book,note);
}

gboolean 
biji_note_book_remove_note(BijiNoteBook *book,BijiNoteObj *note)
{
  g_return_val_if_fail(BIJI_IS_NOTE_BOOK(book),FALSE);
  g_return_val_if_fail(BIJI_IS_NOTE_OBJ(note),FALSE);

  return _note_book_remove_one_note(book,note);
}

/* TODO REMOVE THIS!!! */
BijiNoteObj * note_book_get_nth_data(BijiNoteBook *book,int nth)
{
  if (BIJI_IS_NOTE_BOOK(book))
  {
    return _biji_book_get_nth(book,nth);
  }
  else return NULL ;
}

GList * biji_note_book_get_notes(BijiNoteBook *book)
{
  return _biji_note_book_get_notes(book);
}

BijiNoteObj *
note_book_get_note_at_path (BijiNoteBook *book, gchar *path)
{
  return g_hash_table_lookup (book->priv->notes, path);
}

/* No hash table key, still TODO = better than biji_book_get_nth */
BijiNoteObj *
note_book_get_note(BijiNoteBook *book,gchar *title)
{
  gint i;

  for (i=0 ; i < g_hash_table_size (book->priv->notes); i++)
  {
    BijiNoteObj *o = _biji_book_get_nth(book,i);
    gchar *current = biji_note_obj_get_title (o);
    
    if ( g_strcmp0 (current,title) == 0 )
     return o ;
  }

  return NULL ;
}

GList * 
biji_note_book_get_notes_with_tag(BijiNoteBook *book,gchar* tag)
{
  g_return_val_if_fail(BIJI_IS_NOTE_BOOK(book),NULL);
  return _biji_note_book_get_notes_with_tag(book,tag);
}

GList * 
biji_note_book_get_notes_with_tag_prefix(BijiNoteBook *book,gchar* tag)
{
  return _biji_note_book_get_notes_with_tag_prefix(book,tag);
}

GList * 
biji_note_book_get_no_tag_notes(BijiNoteBook *book)
{
  return _biji_note_book_get_no_tag_notes(book);
}

void 
biji_note_book_remove_tag(BijiNoteBook *book,gchar *tag)
{
  GList *notes = _biji_note_book_get_notes_with_tag(book,tag) ;

  // Remove the tag then save the note.
  g_list_foreach(notes,(GFunc) biji_note_obj_remove_tag,tag);
  g_list_foreach(notes,(GFunc) biji_note_obj_save_note,NULL);
}

BijiNoteObj *
biji_note_book_get_tag_template(BijiNoteBook *book, gchar *tag)
{
  GList *notes = _biji_note_book_get_notes_with_tag(book,tag) ;

  if ( notes == NULL ) 
    return NULL ;

  gint i ;
  for ( i=0 ; i < g_list_length (notes) ; i++ )
  {
    BijiNoteObj *note = BIJI_NOTE_OBJ ( g_list_nth_data(notes,i));
    if ( note_obj_is_template(note))
     return note ;
  }

  return NULL ;
}

BijiNoteBook *
biji_note_book_new (GFile *location)
{
  return g_object_new(BIJI_TYPE_NOTE_BOOK,
                      "location", location,
                      NULL);
}

BijiNoteObj *
biji_note_get_new_from_file (const gchar* path)
{
  BijiNoteObj* ret = biji_note_obj_new_from_path (path);

  /* The deserializer will handle note type */
  biji_lazy_deserialize (ret);

  return ret ;
}

gchar *
biji_note_book_get_uuid (void)
{
  uuid_t unique;
  char out[40];

  uuid_generate (unique);
  uuid_unparse_lower (unique, out);
  return g_strdup_printf ("%s.note", out);
}

/* Common UUID skeleton for new notes.
 * Maybe the right place to set up dates = now */
static BijiNoteObj *
get_note_skeleton (BijiNoteBook *book)
{
  BijiNoteObj *ret = NULL;
  gchar * folder, *name, *path;

  folder = g_file_get_path (book->priv->location);

  while (!ret)
  {
    name = biji_note_book_get_uuid ();
    path = g_build_filename (folder, name, NULL);
    g_free (name);

    if (!g_hash_table_lookup (book->priv->notes, path))
      ret = biji_note_obj_new_from_path (path);

    g_free (path);
  }

  return ret;
}

/* TODO : different New notes shall call a common
 * biji_note_obj_new with different properties : path mandatory,
 * optional title, raw_text, html, ... */

BijiNoteObj*
biji_note_book_get_new_note_from_string (BijiNoteBook *book,
                                         gchar *title)
{
  BijiNoteObj *ret = get_note_skeleton (book);

  /* Note will copy title */
  biji_note_obj_set_title (ret, title);

  biji_note_obj_save_note (ret);
  note_book_append_new_note (book,ret);

  return ret ;
}

BijiNoteObj *
biji_note_book_new_note_with_text (BijiNoteBook *book,
                                   gchar *plain_text)
{
  BijiNoteObj *ret = get_note_skeleton (book);

  /* Note will copy title, raw_text and html strings */
  biji_note_obj_set_title (ret, DEFAULT_NOTE_TITLE);
  biji_note_obj_set_raw_text (ret, g_strdup (plain_text));
  biji_note_obj_set_html_content (ret, plain_text);

  biji_note_obj_save_note (ret);
  note_book_append_new_note (book,ret);

  return ret;
}
