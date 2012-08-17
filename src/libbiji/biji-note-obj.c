#include "biji-note-id.h"
#include "biji-note-book.h"
#include "biji-note-buffer.h"
#include "biji-note-editor.h"
#include "biji-note-obj.h"
#include "biji-note-watcher.h"
#include "biji-read-tomboy.h"
#include "biji-serialize.h"
#include "libbiji.h"

#ifndef NO_NOTE_TITLE
#define NO_NOTE_TITLE
#endif

struct _BijiNoteObjPrivate
{
  BijiNoteBook *book ; // the book the note belongs to, may be NULL.
  BijiNoteID *id; // Metadata : title, date...
  GtkTextTagTable *gtk_tags ;// the common tags with notebook.

  /* EDITION */
  gchar *content ; // XML content of the buffer
  BijiNoteBuffer *buffer; // the GtkTextBuffer might be null
  gint changes_to_save ;

  // TAGS may be notebooks. Templates are just "system:notebook:" tags.
  GList *tags ;
  gboolean is_template ;
  gint is_opened ; // use it to keep track of opened notes

  // Settings? TODO Generaly BijiNoteBook settings are used ?
  gint left_margin ;
  gchar *color ;     /* GdkRGBA. NOT TOMBOY COMPLIANT, but opening these notes with tomboy should work */

  /* Signals */
  gulong note_renamed;
};

#define BIJI_NOTE_OBJ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BIJI_TYPE_NOTE_OBJ, BijiNoteObjPrivate))

G_DEFINE_TYPE (BijiNoteObj, biji_note_obj, G_TYPE_OBJECT);

static void
biji_note_obj_init (BijiNoteObj *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BIJI_TYPE_NOTE_OBJ,
	                                          BijiNoteObjPrivate);

  BijiNoteID* id = g_object_new(biji_note_id_get_type(),NULL);
  self->priv->id = id ;

  self->priv->changes_to_save = 0 ;
  self->priv->book = NULL ;
  self->priv->content = NULL ;
  self->priv->buffer = NULL;
  self->priv->tags = NULL ;
  self->priv->is_template = FALSE ;
  self->priv->is_opened = 0;
  self->priv->left_margin = 6 ; // defautl left margin.
  self->priv->color = NULL ;
}

static void
biji_note_obj_finalize (GObject *object)
{    
  BijiNoteObj *note = BIJI_NOTE_OBJ(object);
    
  // Finalize id
  //biji_note_id_finalize(object);

  // gtk text tag table?

  // buffer

  // free content

  // tags
  g_list_free(note->priv->tags);

  G_OBJECT_CLASS (biji_note_obj_parent_class)->finalize (object);
}

// Signals to be used by biji note obj
enum {
  NOTE_RENAMED,
  NOTE_DELETED,
  BIJI_OBJ_SIGNALS
};

static guint biji_obj_signals [BIJI_OBJ_SIGNALS] = { 0 };

static void
biji_note_obj_class_init (BijiNoteObjClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = biji_note_obj_finalize;

  biji_obj_signals[NOTE_RENAMED] = g_signal_new ( "renamed" ,
	                                              G_OBJECT_CLASS_TYPE (klass),
	    	  	    	  	    	  	    	  G_SIGNAL_RUN_LAST,
	     		                                  0, 
	                                              NULL, 
	                                              NULL,
	                                              g_cclosure_marshal_VOID__VOID,
	                                              G_TYPE_NONE,
	                                              0);

  biji_obj_signals[NOTE_DELETED] = g_signal_new ( "deleted" ,
	                                              G_OBJECT_CLASS_TYPE (klass),
	    	  	    	  	    	  	    	  G_SIGNAL_RUN_LAST,
	     		                                  0, 
	                                              NULL, 
	                                              NULL,
	                                              g_cclosure_marshal_VOID__VOID,
	                                              G_TYPE_NONE,
	                                              0);

  g_type_class_add_private (klass, sizeof (BijiNoteObjPrivate));
}

void 
_biji_note_obj_set_book(BijiNoteObj *note, gpointer biji_note_book)
{
  if ( BIJI_IS_NOTE_BOOK(biji_note_book) )
  {   
    note->priv->book = biji_note_book ;
  }
  else
  {
    g_message("criticical : BIJI NOTE OBJ SET BOOK received wrong parameter");
  }
}

gpointer
biji_note_obj_get_note_book(BijiNoteObj *note)
{
  if ( note->priv->book != NULL )
  {
    return note->priv->book ;
  }
  g_message("biji note obj _ biji note obj get note book returns NULL");
  return NULL ;
}

gboolean 
note_obj_are_same(BijiNoteObj *a, BijiNoteObj* b)
{ 
  if ( _biji_note_id_are_same(a->priv->id,b->priv->id) )
  {
    if ( g_strcmp0 (a->priv->content ,b->priv->content) == 0 )
      return TRUE ;
  }

  return FALSE ;
}

void
biji_note_obj_delete(BijiNoteObj *dead)
{
  GFile *file ;
  //GError *error = NULL ;

  file = g_file_new_for_path(biji_note_id_get_path(dead->priv->id));
    
  // TODO get the note book GCancellable and set it,
  // or create one and append it to BijiNoteBook
  if ( g_file_trash(file,NULL,NULL) == FALSE )
  {
    if ( g_file_delete(file,NULL,NULL) == FALSE )
    {
      g_message("Critical, file not deleted:%s",
      biji_note_id_get_path(dead->priv->id));
    }
  }

  g_signal_emit ( G_OBJECT (dead), biji_obj_signals[NOTE_DELETED],0);
  biji_note_obj_finalize(G_OBJECT(dead));
  
  return ;
}

int
set_note_content(BijiNoteObj* note,gchar* cont)
{
  note->priv->content = cont ;
  return 0 ;
}

gchar* get_note_path (BijiNoteObj* n)
{
  return biji_note_id_get_path(n->priv->id) ;
}

BijiNoteID* note_get_id(BijiNoteObj* n)
{
  return n->priv->id;
}

gchar *
_biji_note_obj_get_title(BijiNoteObj *obj)
{
  return _biji_note_id_get_title(obj->priv->id);
}

void
_biji_note_obj_set_title(BijiNoteObj *note,gchar *title)
{
  /* FIXME if we change title only when different,
   * for some reason it does not work.
   * but it's 02:55 AM and i'm sleeping 
  if ( g_strcmp0 (title,
                  _biji_note_id_get_title(note->priv->id)) == 0 )
  {
    g_message("same title:oldtitle:%s:newtitle:%s",
              title,
              _biji_note_id_get_title(note->priv->id));
    
    return ;
  } */

  // Set title
  _biji_note_id_set_title(note->priv->id,title);

  // Change last metadata change date
  _biji_note_id_set_metadata_change_now(note->priv->id);

  // Emit one signal, notebook might also want to? 
  g_signal_emit ( G_OBJECT (note), 
                     biji_obj_signals[NOTE_RENAMED],0);
}

int
set_note_last_change_date(BijiNoteObj* n,gchar* date)
{
  set_note_id_last_change_date(note_get_id(n),date);
  return 0 ;
}

int
set_note_last_metadata_change_date(BijiNoteObj* n,gchar* date)
{
  set_note_id_last_metadata_change_date(note_get_id(n),date);
  return 0 ;
}

int
set_note_create_date(BijiNoteObj* n,gchar *date)
{
  set_note_id_create_date(note_get_id(n),date);
  return 0 ;
}

void
biji_note_set_content(BijiNoteObj* n, gchar* content)
{
	n->priv->content = content ;
}

gchar* 
note_obj_get_content(BijiNoteObj* n)
{
  return n->priv->content;
}

void
note_obj_set_rgba(BijiNoteObj *n,gchar *rgba)
{
  n->priv->color = rgba ;
}

gchar *
note_obj_get_rgba(BijiNoteObj *n)
{
  return n->priv->color;
}

// FIXME find something better that 10000 char...
// the issue is ... well i don't really remember why ut8 len is not good. 
gchar *
_biji_note_obj_get_raw_text(BijiNoteObj *n)
{    
  xmlDocPtr doc = xmlParseMemory(n->priv->content,
                                 10000);

  if (doc == NULL ) 
  {
    g_message("File not parsed successfully. \n");
    return FALSE;
  }

  xmlNodePtr cur = xmlDocGetRootElement(doc);        // <note-content>
  gchar *result = (gchar*) xmlNodeGetContent (cur) ; //get raw content
  xmlFreeDoc(doc);
    
  return result ;
}

gint
_biji_note_obj_get_left_margin(BijiNoteObj *obj)
{
  return obj->priv->left_margin ; 
}

GList *
_biji_note_obj_get_tags(BijiNoteObj *n)
{
  return g_list_copy(n->priv->tags);
}

gboolean
_biji_note_obj_has_tag(BijiNoteObj *note,gchar *tag)
{
  gint i ;

  if ( note->priv-> tags == NULL )
    return FALSE ;

  for ( i=0 ; i<g_list_length (note->priv->tags) ; i++ )
  {
    if ( g_strcmp0 ((gchar*)g_list_nth_data(note->priv->tags,i),tag)==0)
    {
      return TRUE ;
    }
  }
  return FALSE ;
}

void
_biji_note_obj_add_tag(BijiNoteObj *note, gchar *tag)
{    
  // Add the tag
  GList * current = _biji_note_obj_get_tags(note);
  _biji_note_obj_set_tags(note,g_list_append(current,tag));

  // Update the TagBook
  if ( BIJI_IS_NOTE_BOOK(note->priv->book) )
  {
    _biji_note_book_add_note_to_tag_book(note->priv->book,note,tag);
  }

  else
  {
    g_message("Note has no Book");
  }
}

gboolean
_biji_note_obj_has_tag_prefix(BijiNoteObj *note,gchar *tag)
{
    gint i ;

  if ( note->priv-> tags == NULL )
    return FALSE ;

  for ( i=0 ; i<g_list_length (note->priv->tags) ; i++ )
  {
    if ( g_str_has_prefix ((gchar*)g_list_nth_data(note->priv->tags,i),tag)) 
    {
      return TRUE ;
    }
  }
  return FALSE ;
}


void
_biji_note_obj_set_tags(BijiNoteObj *n, GList *tags)
{
  if ( n->priv->tags != NULL )
  {
    g_list_free(n->priv->tags);
  }
    
  n->priv->tags = tags ;
  _biji_note_id_set_metadata_change_now(n->priv->id);
}

gboolean
note_obj_is_template(BijiNoteObj *n)
{
  g_return_val_if_fail(BIJI_IS_NOTE_OBJ(n),FALSE);	
  return n->priv->is_template;
}

void
note_obj_set_is_template(BijiNoteObj *n,gboolean is_template)
{
  n->priv->is_template = is_template ;
}

gchar *
_biji_note_template_get_tag(BijiNoteObj *template)
{
  if ( template->priv->is_template == FALSE )
  {
    g_message("BIJI NOTE TEMPLATE GET TAG ONLY WORKS WITH TEMPLATES");
    return NULL ;
  }

  if ( template->priv->tags == NULL )
  {
    g_message("template has no tag, which should never happen");
  }
    
  return g_list_nth_data (template->priv->tags,0);
}

GtkTextTagTable *
note_obj_get_gtk_tags(BijiNoteObj *note)
{
  return note->priv->gtk_tags;
}

void 
biji_note_set_gtk_tags(BijiNoteObj *note, GtkTextTagTable *table)
{
  note->priv->gtk_tags = table ;
}

void 
_biji_note_obj_increment_opened(BijiNoteObj *note)
{
  note->priv->is_opened ++ ;
}

gboolean 
_biji_note_obj_is_opened(BijiNoteObj *note)
{
  return note->priv->is_opened ;
}

GtkTextBuffer *
biji_note_get_or_create_buffer(gpointer note_obj)
{
  BijiNoteObj *n = BIJI_NOTE_OBJ(note_obj);

  if ( !n->priv->buffer )
  {
    g_message("no buffer yet , we do create it");
    n->priv->buffer = create_note_buffer(note_obj);

#ifndef NO_NOTE_TITLE
    if ( n->priv->content == NULL )
    {
      g_message("Note has no content. Creating fake content.");
      n->priv->content = g_strdup_printf("<note-content>%s\n\n</note-content>",
                                         biji_note_get_title(n));
    }
#endif
      
    note_buffer_set_xml(n->priv->buffer,n->priv->content);
  }

  GtkTextBuffer *buffer = GTK_TEXT_BUFFER(n->priv->buffer);
  on_note_opened(buffer);
  return buffer;
}

GtkTextView *
note_obj_get_editor(gpointer note_obj)
{
  BIJI_NOTE_OBJ(note_obj)->priv->is_opened ++ ;
  return biji_gtk_view_new(note_obj);
}

gboolean
note_obj_save_note_using_buffer(gpointer note_obj)
{
  xmlBufferPtr buf;
  gboolean result ;
  BijiNoteObj *note = BIJI_NOTE_OBJ(note_obj);
  BijiNoteID *id = note_get_id (note);

  // Change the last change date propery.
  _biji_note_id_set_change_date_now(note->priv->id);
  
  // Work on the content. this func also updates note_obj->priv->content.
  buf = note_obj_serialize(note_obj,3);
  result = g_file_set_contents (biji_note_id_get_path(id),
	                            (gchar*)buf->content,-1,NULL);

  xmlBufferFree(buf);
  return result ;
}

void
_biji_note_obj_propose_saving(gpointer note_obj)
{
  BijiNoteObj *note = BIJI_NOTE_OBJ(note_obj);

  if ( note->priv->changes_to_save > 4 )
  {
    note_obj_save_note_using_buffer(note_obj);
    note->priv->changes_to_save = 0 ;
    return ;
  }

  note->priv->changes_to_save ++ ;
}

void
_biji_note_obj_close_note(gpointer note_obj)
{
  BijiNoteObj *note = BIJI_NOTE_OBJ(note_obj);

  if ( note->priv->changes_to_save != 0 )
  {
    note_obj_save_note_using_buffer(note_obj);
    note->priv->changes_to_save = 0 ;
  }

  note->priv->is_opened -- ;
  g_message("is opened = %i",note->priv->is_opened);

  if ( note->priv->is_opened < 1 )
  {  
    if (BIJI_IS_NOTE_BUFFER(note->priv->buffer) )
    {
      g_message("closing note");
      g_object_unref(note->priv->buffer) ;
      note->priv->buffer = NULL ;
    }
  }
  else {  g_message("keep buffer") ; } 
}

void
_biji_note_obj_mark_as_need_save(gpointer note_obj)
{
  BIJI_NOTE_OBJ(note_obj)->priv->changes_to_save ++ ;
}
