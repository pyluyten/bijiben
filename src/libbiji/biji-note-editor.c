#include "biji-note-buffer.h"
#include "biji-note-editor.h"
#include "biji-note-obj.h"
#include "biji-note-watcher.h"

#ifndef NO_NOTE_TITLE
#define NO_NOTE_TITLE
#endif

struct _BijiNoteEditorPrivate
{
  gboolean text_is_selected ;
};

#define NOTE_EDITOR_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), NOTE_TYPE_EDITOR, NoteEditorPrivate))

G_DEFINE_TYPE (BijiNoteEditor, biji_note_editor, GTK_TYPE_TEXT_VIEW);

static void
biji_note_editor_init (BijiNoteEditor *object)
{
  object->priv = G_TYPE_INSTANCE_GET_PRIVATE (object, BIJI_TYPE_NOTE_EDITOR,
	                                          BijiNoteEditorPrivate);
  object->priv->text_is_selected = FALSE ;
}

static void
biji_note_editor_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

  G_OBJECT_CLASS (biji_note_editor_parent_class)->finalize (object);
}

// Signals to be used by biji_note_editor class
enum {
  NO_MORE_SELECTION,
  SELECTION,
  BIJI_EDITOR_SIGNALS
};

static guint biji_editor_signals [BIJI_EDITOR_SIGNALS] = { 0 };

static void
biji_note_editor_class_init (BijiNoteEditorClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  biji_editor_signals[NO_MORE_SELECTION] = g_signal_new 
        ( "no-more-selection" ,                 
	      G_OBJECT_CLASS_TYPE (klass),	// itype
	      G_SIGNAL_RUN_FIRST,		// signal flags
	     0,				        // class offset 
	        NULL,			// accumulator 
	        NULL,				 // accu data
	        g_cclosure_marshal_VOID__VOID,		  // marshaller
	        G_TYPE_NONE,				 // return type
	        0);				 // 0 params. (so last arg)

  biji_editor_signals[SELECTION] = g_signal_new 
        ( "selection" ,                         
	  G_OBJECT_CLASS_TYPE (klass),				// itype
	  G_SIGNAL_RUN_FIRST,					// signal flags
	  0,						// class offset 
	  NULL,							// accumulator 
	  NULL,							// accu data
	  g_cclosure_marshal_VOID__VOID,				// marshaller
	  G_TYPE_NONE,						// return type
	  0);							// 0 params. (so last arg)

  g_type_class_add_private (klass, sizeof (BijiNoteEditorPrivate));
  object_class->finalize = biji_note_editor_finalize;
}

// Accessors

void
_biji_note_editor_set_text_is_selected(gpointer biji_note_editor,gboolean val)
{
  BijiNoteEditor *editor = BIJI_NOTE_EDITOR(biji_note_editor);

  if ( val == FALSE )
  {
    editor->priv->text_is_selected = FALSE ;
    g_signal_emit ( G_OBJECT (biji_note_editor), 
                    biji_editor_signals[NO_MORE_SELECTION],0);
  }
  else
  {
    editor->priv->text_is_selected = TRUE ;
    g_signal_emit ( G_OBJECT (biji_note_editor), 
                    biji_editor_signals[SELECTION],0);
  }
}

gboolean
_biji_note_editor_get_text_is_selected(gpointer biji_note_editor)
{
  BijiNoteEditor *editor = BIJI_NOTE_EDITOR(biji_note_editor);
    
  return editor->priv->text_is_selected ;
}


// Callbacks

void
on_text_buffer_insert_text	(GtkTextBuffer *buffer,
				 GtkTextIter   *iter,
				 gchar	       *text,
				 gint           len,
				 gpointer       user_data)
{
  GtkTextIter start_iter ;

#ifndef NO_NOTE_TITLE
  if (gtk_text_iter_get_line(iter) == 0)
    return;
#endif

  apply_active_tags(buffer, iter, text, user_data);

  start_iter = *iter;
  //end_iter = *iter;

  // Move start iter back to the position before the insert
  //gtk_text_iter_backward_chars(&start_iter, g_utf8_strlen(text, -1));

  //auto_highlight_links(ui, &start_iter, &end_iter);
  //auto_highlight_urls(ui, &start_iter, &end_iter);
}

/* TODO: The signal "mark-set" is emitted 4 times when clicking into the text. While selecting
 * it's emitted continuesly. */
void
on_textview_cursor_moved		(GtkTextBuffer	*buffer,
								GtkTextIter		*location,
								GtkTextMark		*mark,
								gpointer		 user_data)
{
  const gchar *mark_name;

  // We only do something if the "insert" mark changed
  mark_name = gtk_text_mark_get_name(mark);
    
  if ((mark_name == NULL) || (g_strcmp0(mark_name, "insert") != 0))
    return;

  note_buffer_update_active_tags(BIJI_NOTE_BUFFER(buffer));
}

gboolean 
key_pressed (GtkWidget *gtk_text_view, GdkEventKey *ev)
{
  gboolean result = FALSE; // allows the input
  GtkTextView *view = GTK_TEXT_VIEW(gtk_text_view);
  BijiNoteEditor *editor = BIJI_NOTE_EDITOR(view);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(view) ;
  BijiNoteBuffer *biji_buffer = BIJI_NOTE_BUFFER(buffer);

  switch (ev->keyval)
  {
      
    case GDK_KEY_KP_Enter:
    case GDK_KEY_Return:
      editor->priv->text_is_selected = FALSE ;
	    result = note_buffer_add_new_line(buffer);
  	  break;

    case GDK_KEY_Tab:
      editor->priv->text_is_selected = FALSE ;
	    result = note_buffer_add_tab(biji_buffer);
      break;

    case GDK_KEY_ISO_Left_Tab:
      editor->priv->text_is_selected = FALSE ;
      result = note_buffer_remove_tab(biji_buffer);

	  case GDK_KEY_BackSpace:
      editor->priv->text_is_selected = FALSE ;
	    result = note_buffer_backspace_handler(biji_buffer);
	    break;

    case GDK_KEY_Home:
    case GDK_KEY_Left:
    case GDK_KEY_Right:
    case GDK_KEY_Up:
    case GDK_KEY_Down:
    case GDK_KEY_End:
      break;
          
	default:
	  note_buffer_check_selection(biji_buffer,(gpointer)editor);
      break;
  }

  gtk_text_view_scroll_mark_onscreen(view, gtk_text_buffer_get_insert(buffer));
  return result;
}

gboolean
key_released(GtkWidget *gtk_text_view,GdkEvent  *event)
{
  GtkTextIter start,end ;
  GtkTextView *view = GTK_TEXT_VIEW(gtk_text_view);
  BijiNoteEditor *editor = BIJI_NOTE_EDITOR(view);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor));

  if ( gtk_text_buffer_get_selection_bounds(buffer, &start, &end) )
  {  
    _biji_note_editor_set_text_is_selected(editor,TRUE);
    return FALSE ;
  }
    
  _biji_note_editor_set_text_is_selected(editor,FALSE);    
  return FALSE;
}

gboolean 
button_released(GtkTextView *widget,GdkEvent  *event,gpointer user_data)
{
  BijiNoteBuffer *biji_buffer = BIJI_NOTE_BUFFER(gtk_text_view_get_buffer(widget));
  BijiNoteEditor *editor = BIJI_NOTE_EDITOR(widget);

  note_buffer_check_selection(biji_buffer,(gpointer)editor);

  return FALSE ;
}

GtkTextView * 
biji_gtk_view_new(gpointer *note_obj)
{
  BijiNoteObj *obj= BIJI_NOTE_OBJ(note_obj);

  /* Get buffer, create view */
  GtkTextBuffer *buf = biji_note_get_or_create_buffer(obj);
  _biji_note_obj_increment_opened(obj);

  BijiNoteEditor *editor = g_object_new(BIJI_TYPE_NOTE_EDITOR,NULL);
  GtkTextView *result = GTK_TEXT_VIEW(editor);
  gtk_text_view_set_buffer(result,buf);

  /* Wrap mode, margin, color, lines */
  gtk_text_view_set_wrap_mode(result,GTK_WRAP_WORD);
  gtk_text_view_set_left_margin(result,_biji_note_obj_get_left_margin(obj));

  /* buffer signals */
  g_signal_connect(buf,"mark-set",G_CALLBACK(on_textview_cursor_moved),NULL);
  g_signal_connect(buf,"changed",G_CALLBACK(watch_buffer),editor);
  g_signal_connect(buf,"insert-text",G_CALLBACK(on_text_buffer_insert_text),NULL);

  /* text view signals */
  g_signal_connect(result,"key-press-event",G_CALLBACK(key_pressed),NULL);
  g_signal_connect(result,"key-release-event",G_CALLBACK(key_released),NULL);
  g_signal_connect(result,"button-release-event",G_CALLBACK(button_released),NULL);
    
  return result;
}
