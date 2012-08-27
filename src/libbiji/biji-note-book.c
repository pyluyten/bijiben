
#include <gtk/gtk.h>

#include "libbiji.h"
#include "biji-note-editor.h"
#include "biji-note-book.h"

struct _BijiNoteBookPrivate
{
  GList *notes;           // GList of BijiNoteObj
  GList *tags;            // GList of TagBooks
  GtkTextTagTable *table; // TODO common tags table for all notes
  gint length;            // simpler than crossing GList 
  // GCancellable // when a note is deleted. it could even be a list of ?

  /* Signals */
  gulong note_renamed ;
};

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
    
  g_list_free(book->priv->tags) ;
  g_list_free_full(book->priv->notes,destroy_note_if_needed);

  G_OBJECT_CLASS (biji_note_book_parent_class)->finalize (object);
}

/* Signals */
enum {
  BOOK_AMENDED,
  BIJI_BOOK_SIGNALS
};

static guint biji_book_signals [BIJI_BOOK_SIGNALS] = { 0 };

static void
biji_note_book_class_init (BijiNoteBookClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (BijiNoteBookPrivate));
  object_class->finalize = biji_note_book_finalize;

  biji_book_signals[BOOK_AMENDED] = g_signal_new ( "changed" ,
	                                              G_OBJECT_CLASS_TYPE (klass),
	    	  	    	  	    	  	    	  G_SIGNAL_RUN_LAST,
	     		                                  0, 
	                                              NULL, 
	                                              NULL,
	                                              g_cclosure_marshal_VOID__VOID,
	                                              G_TYPE_NONE,
	                                              0);
	                                              
  g_message("notebook changed");
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

BijiNoteBook *
_biji_note_book_get_new_from_dir(gchar *folder)
{
  BijiNoteBook *ret;
  GDir *dir;

  ret = g_object_new(BIJI_TYPE_NOTE_BOOK,NULL);		
  dir = g_dir_open (folder,0, NULL);
  if (dir)
  {
    const gchar *file = NULL;
    while ( (file = g_dir_read_name (dir)) )
    {
      if ( g_str_has_suffix(file,".note") )
      {
        gchar * path = g_strdup_printf("%s/%s",folder,file);
        BijiNoteObj* cur = biji_note_get_new_from_file(path);
        if ( BIJI_IS_NOTE_OBJ ( cur ) ) 
        {
          // Add the note to the book
          _biji_note_book_add_one_note(ret,cur);            
        }
      }
    }
      
  // FIXME 
  g_dir_close (dir), dir = NULL;      
  }
	
  return ret;
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

void
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

gint _biji_book_get_number(BijiNoteBook *book)
{
  return book->priv->length ;
}

BijiNoteObj * 
_biji_book_get_nth(BijiNoteBook *book,int i)
{
  return BIJI_NOTE_OBJ(g_list_nth_data(book->priv->notes,i));
}
