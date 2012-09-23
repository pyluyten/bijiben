
#include <gtk/gtk.h>

#include "libbiji.h"
#include "biji-note-editor.h"
#include "biji-note-book.h"
#include "biji-read-tomboy.h"

struct _BijiNoteBookPrivate
{
  GList *notes;           // GList of BijiNoteObj
  GList *tags;            // GList of TagBooks
  GtkTextTagTable *table; // TODO common tags table for all notes
  gint length;            // simpler than crossing GList 

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
biji_note_book_init (BijiNoteBook *object)
{
  object->priv = G_TYPE_INSTANCE_GET_PRIVATE (object, BIJI_TYPE_NOTE_BOOK,
	                                          BijiNoteBookPrivate);
  object->priv->notes = NULL;
  object->priv->tags = NULL ;
  object->priv->table = NULL ; //create_all_note_tags();
  object->priv->length = 0 ;
}

static void
destroy_note_if_needed(gpointer note)
{
  // FIXME : if ref = 0
  g_free(BIJI_NOTE_OBJ(note));
}

static void
biji_note_book_finalize (GObject *object)
{
  BijiNoteBook *book = BIJI_NOTE_BOOK (object) ;

  if (book->priv->load_cancellable)
    g_cancellable_cancel (book->priv->load_cancellable);

  g_clear_object (&book->priv->load_cancellable);
  g_clear_object (&book->priv->location);

  g_list_free(book->priv->tags) ;
  g_list_free_full(book->priv->notes,destroy_note_if_needed);

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
  title = biji_note_get_title(note);
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
   _biji_note_obj_set_title(note,new_title);
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
  biji_note_set_gtk_tags(note,book->priv->table);
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
  book->priv->notes = g_list_append(book->priv->notes,note);
  book->priv->length++;
  //g_signal_connect(note,"renamed",G_CALLBACK(notify_changed),book);
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

	for ( i=0 ; i < book->priv->length ; i++ )
	{
		BijiNoteObj *iter = BIJI_NOTE_OBJ(g_list_nth_data (book->priv->notes,i));
		if ( g_strcmp0 (biji_note_get_title(iter),title) == 0 )
		{
			return FALSE ;
		}
	}
	return TRUE ;
}

void
_biji_note_book_add_note_to_tag_book(BijiNoteBook *book,
                                     BijiNoteObj *note,
                                     gchar *tag)
{
  TagBook *tag_book;
  tag_book = biji_book_get_or_create_tag_book(book,tag);
  tag_book->notes = g_list_append(tag_book->notes,note) ;
  book->priv->length++; ;
}

gboolean 
_note_book_remove_one_note(BijiNoteBook *book,BijiNoteObj *note)
{
  int i;

  for ( i = 0 ; i< book->priv->length ; i++ )
  {
    // Find the note to remove
    if ( note_obj_are_same(g_list_nth_data(book->priv->notes,i),note) )
    {
      // Update the Book
      book->priv->notes = g_list_remove( book->priv->notes,(gpointer)note);
      book->priv->length-- ;

      // Destroy the Note (including file) and return
      biji_note_obj_delete(note);
      g_signal_emit ( G_OBJECT (book), biji_book_signals[BOOK_AMENDED],0);

      return TRUE ;
    }
  }

  return FALSE ;
}

GList *
_biji_note_book_get_notes (BijiNoteBook *book)
{
  return g_list_copy(book->priv->notes);
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
    
  g_list_foreach(book->priv->notes,(GFunc)add_note_to_list_if_tag_prefix,&booklet);
  return booklet.notes ;
}

static void
add_note_to_list_if_no_tag(BijiNoteObj *note,GList **notes)
{
  if ( _biji_note_obj_get_tags(note) == NULL )
  {
    *(notes) = g_list_append(*(notes),note);
  }
}

GList *
_biji_note_book_get_no_tag_notes(BijiNoteBook *book)
{
  GList * notes = NULL ;
    
  g_list_foreach(book->priv->notes,(GFunc)add_note_to_list_if_no_tag,&notes);
  return notes ;
}

BijiNoteObj * 
_biji_book_get_nth(BijiNoteBook *book,int i)
{
  return BIJI_NOTE_OBJ(g_list_nth_data(book->priv->notes,i));
}

/* Notes collection */
void note_book_append_new_note(BijiNoteBook *book,BijiNoteObj *note)
{    
  if (BIJI_IS_NOTE_BOOK(book))
  {
    if ( BIJI_IS_NOTE_OBJ(note) )
    {
      _biji_note_book_add_one_note(book,note);
    }
    else
    {
      g_message("You trying to add a car to a list of planes");  
    }
  }
  else
  {
    // TODO return biji_note_book_with_single_note(NoteObj *note)
    g_message("ERROR. NOT A BOOK.");
  }
}

gboolean 
biji_note_book_remove_note(BijiNoteBook *book,BijiNoteObj *note)
{
  g_return_val_if_fail(BIJI_IS_NOTE_BOOK(book),FALSE);
  g_return_val_if_fail(BIJI_IS_NOTE_OBJ(note),FALSE);

  return _note_book_remove_one_note(book,note);
}

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
note_book_get_note_at_path(BijiNoteBook *book,gchar *path)
{
  gint i,len;
  len = book->priv->length ;

  for (i=0 ; i<len; i++)
  {

    BijiNoteObj *o = _biji_book_get_nth(book,i);
    gchar *current = note_obj_get_path (o);
    
    if ( g_strcmp0 (current,path) == 0 )
      return o ;
  }
  
  return NULL ;
}


BijiNoteObj *
note_book_get_note(BijiNoteBook *book,gchar *title)
{
  gint i,len;
  len = book->priv->length;

  for (i=0 ; i<len; i++)
  {
    BijiNoteObj *o = _biji_book_get_nth(book,i);
    gchar *current = biji_note_get_title (o);
    
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
  g_list_foreach(notes,(GFunc)biji_note_obj_remove_tag,tag);
  g_list_foreach(notes,(GFunc)note_obj_save_note_using_buffer,NULL);
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

/* Todo : check file type */
BijiNoteObj*
biji_note_get_new_from_file (const gchar* path)
{
  BijiNoteObj* ret ;

  ret = g_object_new(BIJI_TYPE_NOTE_OBJ,NULL);
  set_note_id_path(note_get_id(ret),path);

  load_tomboy_note((gpointer)ret);
  return ret ;
}

BijiNoteObj*
biji_note_get_new_from_string (gchar* title, gchar *folder)
{
  BijiNoteObj *ret;
  BijiNoteID *id;
  GRand *random;
  gint suffix;

  ret = g_object_new(BIJI_TYPE_NOTE_OBJ,NULL);
  id = note_get_id(ret);
  _biji_note_obj_set_title(ret,title);

  random = g_rand_new();
  suffix = g_rand_int(random);
  g_rand_free (random);
  set_note_id_path(id,g_strdup_printf ("%s/%i.note",folder,suffix));

  return ret ;
}
