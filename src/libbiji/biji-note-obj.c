#include "biji-date-time.h"
#include "biji-note-id.h"
#include "biji-note-book.h"
#include "biji-note-obj.h"
#include "biji-note-watcher.h"
#include "editor/biji-webkit-editor.h"
#include "serializer/biji-lazy-serializer.h"
#include "libbiji.h"

#ifndef NO_NOTE_TITLE
#define NO_NOTE_TITLE
#endif

/* Default color (X11 rgb.txt)  */ 
#define DEFAULT_NOTE_COLOR "rgb(233,235,191)"

/* Icon */
#define ICON_WIDTH 200
#define ICON_HEIGHT 240
#define ICON_FONT "Purusa 10"

struct _BijiNoteObjPrivate
{
  /* Notebook might be null. Shared GtkTextTagTable */
  BijiNoteBook          *book;
  GtkTextTagTable       *gtk_tags;

  /* Metadata */
  BijiNoteID            *id;
  gchar                 *html;
  gchar                 *raw_text;

  /* XML Content */
  gchar                 *content;

  /* Buffer might be null */
  BijiWebkitEditor      *editor;
  gint                  changes_to_save;

  /* TAGS may be notebooks. */
  GList                 *tags ;

  /* Templates are just "system:notebook:" tags. */
  gboolean              is_template ;
  gint                  is_opened;

  gint                  left_margin;
  GdkRGBA               *color;

  /* Might be null. triggered by get_icon */
  GdkPixbuf             *icon;
  gboolean              icon_needs_update;

  /* Signals */
  gulong note_renamed;
};

#define BIJI_NOTE_OBJ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BIJI_TYPE_NOTE_OBJ, BijiNoteObjPrivate))

G_DEFINE_TYPE (BijiNoteObj, biji_note_obj, G_TYPE_OBJECT);

static void
biji_note_obj_init (BijiNoteObj *self)
{
  BijiNoteObjPrivate *priv ;
    
  priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BIJI_TYPE_NOTE_OBJ, BijiNoteObjPrivate);

  self->priv = priv ;

  BijiNoteID* id = g_object_new(biji_note_id_get_type(),NULL);
  priv->id = id ;

  priv->changes_to_save = 0 ;
  priv->book = NULL ;
  priv->content = NULL ;
  priv->tags = NULL ;
  priv->is_template = FALSE ;
  priv->is_opened = 0;
  priv->left_margin = 6 ; // defautl left margin.

  /* The editor is NULL so we know it's not opened
   * neither fully deserialized */
  priv->html = NULL;
  priv->editor = NULL;

  /* Icon is only computed when necessary */
  priv->icon = NULL;

  priv->color = g_new(GdkRGBA,1) ;
  gdk_rgba_parse ( priv->color , DEFAULT_NOTE_COLOR ) ; 
}

static void
biji_note_obj_finalize (GObject *object)
{    
  BijiNoteObj        *self = BIJI_NOTE_OBJ(object);
  BijiNoteObjPrivate *priv = self->priv;
    
  // Finalize id
  //biji_note_id_finalize(object);

  // gtk text tag table?

  // buffer

  // free content
  if (priv->html)
    g_free (priv->html);

  // tags
  g_list_free (priv->tags);

  g_object_unref (priv->icon);

  G_OBJECT_CLASS (biji_note_obj_parent_class)->finalize (object);
}

// Signals to be used by biji note obj
enum {
  NOTE_RENAMED,
  NOTE_DELETED,
  NOTE_CHANGED,
  NOTE_COLOR_CHANGED,
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

  biji_obj_signals[NOTE_CHANGED] = g_signal_new ( "changed" ,
                                                  G_OBJECT_CLASS_TYPE (klass),
                                                  G_SIGNAL_RUN_LAST,
                                                  0, 
                                                  NULL, 
                                                  NULL,
                                                  g_cclosure_marshal_VOID__VOID,
                                                  G_TYPE_NONE,
                                                  0);

  biji_obj_signals[NOTE_COLOR_CHANGED] = g_signal_new ("color-changed" ,
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

glong 
biji_note_obj_get_last_change_date_sec ( BijiNoteObj *n )
{
    return biji_note_id_get_last_change_date_sec(note_get_id(n)); 
}

gchar *
biji_note_obj_get_last_change_date_string (BijiNoteObj *self)
{
  return biji_get_time_diff_with_time (
             biji_note_id_get_last_change_date_sec(note_get_id(self)));
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

static void
biji_note_obj_set_rgba_internal (BijiNoteObj *n, GdkRGBA *rgba)
{
  n->priv->color = gdk_rgba_copy(rgba);
  n->priv->icon_needs_update = TRUE;

  _biji_note_id_set_metadata_change_now (n->priv->id);
  _biji_note_obj_propose_saving (n);

  /* Make editor & notebook know about this change */
  g_signal_emit (G_OBJECT (n), biji_obj_signals[NOTE_COLOR_CHANGED],0);
  g_signal_emit (G_OBJECT (n), biji_obj_signals[NOTE_CHANGED],0);
}


void
biji_note_obj_set_rgba(BijiNoteObj *n,GdkRGBA *rgba)
{
  if (!n->priv->color)
  {    
    biji_note_obj_set_rgba_internal (n, rgba);
    return;
  }

  if (!gdk_rgba_equal (n->priv->color,rgba))
  {
    gdk_rgba_free (n->priv->color);
    biji_note_obj_set_rgba_internal (n, rgba);
  }
}

gboolean
biji_note_obj_get_rgba(BijiNoteObj *n,
                       GdkRGBA *rgba)
{
  if (n->priv->color && rgba)
    {
      *rgba = *(n->priv->color);
      return TRUE;
    }

  return FALSE;
}

gchar *
_biji_note_obj_get_raw_text(BijiNoteObj *n)
{
  if (n->priv->raw_text)
    return n->priv->raw_text;

  return "";
}

void biji_note_obj_set_raw_text (BijiNoteObj *note, gchar *plain_text)
{
  if (note->priv->raw_text)
    g_free (note->priv->raw_text);

  note->priv->raw_text = g_strdup (plain_text);
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

gboolean
note_obj_save_note_using_buffer (gpointer note_obj)
{
  gboolean result;
  BijiLazySerializer *serializer;
  BijiNoteObj *note = BIJI_NOTE_OBJ(note_obj);

  // Change the last change date propery
  _biji_note_id_set_change_date_now (note->priv->id);
  
  // Work on the content
  serializer = biji_lazy_serializer_new (note);
  result = biji_lazy_serialize (serializer);
  g_object_unref (serializer);

  // Update the icon
  if (note->priv->icon)
  {
    g_object_unref (note->priv->icon);
    note->priv->icon = NULL ;
  }

  biji_note_obj_get_icon (note);

  // Alert
  g_signal_emit ( G_OBJECT (note_obj), 
                  biji_obj_signals[NOTE_CHANGED],
                  0);

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
_biji_note_obj_mark_as_need_save(gpointer note_obj)
{
  BIJI_NOTE_OBJ(note_obj)->priv->changes_to_save ++ ;
}

/* Borders are just temp before libiji includes
 * some hardcoded frame svg */
static GdkPixbuf *
biji_note_icon_add_frame (GdkPixbuf *pixbuf)
{
  gint                   height, width;
  gint                   border = 10;
  cairo_pattern_t       *pattern;
  cairo_t               *cr;
  cairo_surface_t       *surface = NULL;
  GdkPixbuf *framed;

  width = gdk_pixbuf_get_width (pixbuf) + 2*border;
  height = gdk_pixbuf_get_height (pixbuf) + 2*border;

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
  cr = cairo_create (surface);

  /* Draw the left-shadow. */
  cairo_save(cr);
  pattern = cairo_pattern_create_linear (border, border, 0, border);
  cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.5);
  cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);
  cairo_rectangle (cr, 0, border, border, height - 2*border);
  cairo_clip (cr);
  cairo_set_source (cr, pattern);
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);
  cairo_restore (cr);

  /* Draw the up-left quarter-circle. */
  cairo_save(cr);
  pattern = cairo_pattern_create_radial (border, border, 0, border, border, border);
  cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0,  0.5);
  cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);
  cairo_rectangle (cr, 0, 0, border, border);
  cairo_clip (cr);
  cairo_set_source (cr, pattern);
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);
  cairo_restore(cr);

  cairo_save(cr);
  pattern = cairo_pattern_create_linear (border, border, border, 0);
  cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.5);
  cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);
  cairo_rectangle (cr, border, 0, width - 2*border, border);
  cairo_clip (cr);
  cairo_set_source (cr, pattern);
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);
  cairo_restore (cr);

  cairo_save(cr);
  pattern = cairo_pattern_create_radial (width - border, border, 0, width - border, border, border);
  cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0,  0.5);
  cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);
  cairo_rectangle (cr, width - border, 0, border, border);
  cairo_clip (cr);
  cairo_set_source (cr, pattern);
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);
  cairo_restore(cr);

  cairo_save(cr);
  pattern = cairo_pattern_create_linear (width - border, border, width, border);
  cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.5);
  cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);
  cairo_rectangle (cr, width - border, border, width, height - 2*border);
  cairo_clip (cr);
  cairo_set_source (cr, pattern);
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);
  cairo_restore (cr);

  cairo_save(cr);
  pattern = cairo_pattern_create_radial (border, height - border, 0, border, height - border, border);
  cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0,  0.5);
  cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);
  cairo_rectangle (cr, 0, height - border, border, border);
  cairo_clip (cr);
  cairo_set_source (cr, pattern);
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);
  cairo_restore(cr);

  cairo_save(cr);
  pattern = cairo_pattern_create_linear (border, height - border, border, height);
  cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.5);
  cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);
  cairo_rectangle (cr, border, height - border, width - 2*border, border);
  cairo_clip (cr);
  cairo_set_source (cr, pattern);
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);
  cairo_restore (cr);

  cairo_save(cr);
  pattern = cairo_pattern_create_radial (width - border, height - border, 0, width - border, height - border, border);
  cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0,  0.5);
  cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);
  cairo_rectangle (cr, width - border, height - border, border, border);
  cairo_clip (cr);
  cairo_set_source (cr, pattern);
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);
  cairo_restore(cr);

  gdk_cairo_set_source_pixbuf (cr, pixbuf, border, border);
  cairo_rectangle (cr, border, border, width - 2*border, height - 2*border);
  cairo_clip(cr);
  cairo_paint (cr);

  framed = gdk_pixbuf_get_from_surface (surface, 0, 0, width, height);

  cairo_destroy (cr);
  cairo_surface_destroy (surface);

  return framed;
}

GdkPixbuf *
biji_note_obj_get_icon (BijiNoteObj *note)
{
  GdkRGBA               note_color;
  gchar                 *text;
  cairo_t               *cr;
  PangoLayout           *layout;
  PangoFontDescription  *desc;
  GdkPixbuf             *ret = NULL;
  cairo_surface_t       *surface = NULL;

  if (note->priv->icon && !note->priv->icon_needs_update)
    return note->priv->icon;

  /* Create & Draw surface */ 
  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32 , 
                                        ICON_WIDTH,
                                        ICON_HEIGHT) ;
  cr = cairo_create (surface);

  /* Background */
  cairo_rectangle (cr, 0.5, 0.5, ICON_WIDTH, ICON_HEIGHT);
  if (biji_note_obj_get_rgba (note, &note_color))
    gdk_cairo_set_source_rgba (cr, &note_color);

  cairo_fill (cr);

  /* Text */
  text = biji_note_get_raw_text (note);
  if (text != NULL)
  {
    cairo_translate (cr, 10, 10);
    layout = pango_cairo_create_layout (cr);

    pango_layout_set_width (layout, 180000 );
    pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);
    pango_layout_set_height (layout, 180000 ) ;

    pango_layout_set_text (layout, text, -1);
    desc = pango_font_description_from_string (ICON_FONT);
    pango_layout_set_font_description (layout, desc);
    pango_font_description_free (desc);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    pango_cairo_update_layout (cr, layout);
    pango_cairo_show_layout (cr, layout);

    g_object_unref (layout);
    /*g_free (text); Webkit text no longer to be freed */
  }

  cairo_destroy (cr);

  ret = gdk_pixbuf_get_from_surface (surface,
                                     0, 0,
                                     ICON_WIDTH,
                                     ICON_HEIGHT);
  cairo_surface_destroy (surface);

  note->priv->icon = biji_note_icon_add_frame(ret);
  note->priv->icon_needs_update = FALSE;

  return note->priv->icon ;
}

/* Single Note */

gchar*
biji_note_get_title(BijiNoteObj *note_obj_ptr)
{
  if (BIJI_IS_NOTE_OBJ(note_obj_ptr))
  {
    return _biji_note_obj_get_title(note_obj_ptr);
  }
  else
  {
    return "This is NOT a NoteObj" ;
  }
}

gchar *
biji_note_get_raw_text(BijiNoteObj *note)
{
  g_return_val_if_fail(BIJI_IS_NOTE_OBJ(note),NULL);

  return _biji_note_obj_get_raw_text(note);
}

int
set_note_title(BijiNoteObj *note_obj_ptr,gchar *title)
{
  BijiNoteObj* n = BIJI_NOTE_OBJ(note_obj_ptr) ;
  _biji_note_obj_set_title(n,title);

  return 0 ;
}

gboolean
biji_note_obj_is_template(BijiNoteObj *note)
{
  return note_obj_is_template(note);
}

gchar *note_obj_get_path(BijiNoteObj *note)
{
  return get_note_path(note);
}

gboolean
biji_note_obj_has_tag(BijiNoteObj *note,gchar *tag)
{
  return _biji_note_obj_has_tag(note,tag);
}

GList *biji_note_obj_get_tags(BijiNoteObj *note)
{
  return _biji_note_obj_get_tags(note);
}

gboolean
biji_note_obj_add_tag(BijiNoteObj *note, gchar *tag)
{
  g_message("Biji note obj add tag - NEW way");

  // If the note has already the tag, return FALSE
  if  ( _biji_note_obj_has_tag(note,tag) )
  {
    return FALSE ;
  }

  _biji_note_obj_add_tag(note,tag);
  return TRUE ;
}

gboolean
biji_note_obj_remove_tag(BijiNoteObj *note,gchar *tag)
{
  // If the note has tag, remove it.
  if ( _biji_note_obj_has_tag(note,tag) )
  {
    GList *current = _biji_note_obj_get_tags(note);
    gint i ;
    gchar *to_remove = NULL ;
    
    for ( i = 0 ; i < g_list_length(current) ; i++ )
    {
      if ( g_strcmp0 (g_list_nth_data(current,i),tag) == 0 )
      {
        to_remove = g_list_nth_data(current,i);
      }
    }
    
    _biji_note_obj_set_tags(note,g_list_remove(current,to_remove));
    return TRUE ;
  }

  // Else return false
  return FALSE ;
}

gchar *biji_note_obj_get_last_change_date(BijiNoteObj *note)
{
  return biji_note_id_get_last_change_date(note_get_id(note));
}

gchar *biji_note_obj_get_create_date(BijiNoteObj *note)
{
  return biji_note_id_get_create_date(note_get_id(note));
}

/* Webkit */

gchar *
biji_note_obj_get_html (BijiNoteObj *note)
{
  return note->priv->html;
}

void
biji_note_obj_set_html_content (BijiNoteObj *note,
                                gchar *html)
{
  // TODO : queue_save with timeout struct

  if (html)
  {
    g_free (note->priv->html);
    note->priv->html = g_strdup (html);

    /* This func will apply when first serialisazing
     * but it handles note icon*/
    note_obj_save_note_using_buffer (note);
  }
}

gboolean
biji_note_obj_is_opened (BijiNoteObj *note)
{
  return BIJI_IS_WEBKIT_EDITOR (note->priv->editor);
}

/* Should connect to "destroy" there
 * to auto set editor=NULL when widget killed */
GtkWidget *
biji_note_obj_open (BijiNoteObj *note)
{
  note->priv->editor = biji_webkit_editor_new (note);
  return GTK_WIDGET (note->priv->editor);
}

GtkWidget *
biji_note_obj_get_editor (BijiNoteObj *note)
{
  if (!biji_note_obj_is_opened (note))
  {
    g_warning ("note not opened");
    return NULL;
  }

  return GTK_WIDGET (note->priv->editor);
}

void
biji_note_obj_close (BijiNoteObj *note)
{
  if (biji_note_obj_is_opened (note))
  {
    gtk_widget_destroy (GTK_WIDGET (note->priv->editor));
    note->priv->editor = NULL;
  }

  else
  {
    g_warning ("Note not opened");
  }
}

void
biji_note_obj_editor_apply_format (BijiNoteObj *note, gint format)
{
  if (biji_note_obj_is_opened (note))
    biji_webkit_editor_apply_format ( note->priv->editor , format);
}

gboolean
biji_note_obj_editor_has_selection (BijiNoteObj *note)
{
  if (biji_note_obj_is_opened (note))
    return biji_webkit_editor_has_selection (note->priv->editor);

  return FALSE;
}

gchar *
biji_note_obj_editor_get_selection (BijiNoteObj *note)
{
  if (biji_note_obj_is_opened (note))
    return biji_webkit_editor_get_selection (note->priv->editor);

  return NULL;
}

void biji_note_obj_editor_cut (BijiNoteObj *note)
{
  if (biji_note_obj_is_opened (note))
    biji_webkit_editor_cut (note->priv->editor);
}

void biji_note_obj_editor_copy (BijiNoteObj *note)
{
  if (biji_note_obj_is_opened (note))
    biji_webkit_editor_copy (note->priv->editor);
}

void biji_note_obj_editor_paste (BijiNoteObj *note)
{
  if (biji_note_obj_is_opened (note))
    biji_webkit_editor_paste (note->priv->editor);
}

