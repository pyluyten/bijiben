#include "biji-note-book.h"
#include "biji-note-buffer.h"
#include "biji-note-editor.h"
#include "biji-note-obj.h"
#include "biji-note-watcher.h"

#ifndef NO_NOTE_TITLE
#define NO_NOTE_TITLE
#endif


static void
check_or_fix_title(BijiNoteObj *obj)
{
  // TODO une fois l'opération finie, on doit émettre un signal titre changé.
    
  BijiNoteBook * book = biji_note_obj_get_note_book(obj);
    
  if ( book == NULL )
  {
    return ;
  }

  if ( _biji_note_book_is_title_unique(book,_biji_note_obj_get_title(obj))) 
                                      //get_note_id_title(note_get_id(obj))))
  {
    return ;
  }
}



void
on_note_opened(GtkTextBuffer *buffer)
{
	format_note_title(buffer);
	//TODO Check links
}


gboolean
watch_buffer(GtkTextBuffer *buffer, BijiNoteEditor *editor)
{
	BijiNoteObj *obj;
	GtkTextIter start,end;
		
	obj = BIJI_NOTE_OBJ(note_buffer_get_obj(BIJI_NOTE_BUFFER(buffer)));
    
	gtk_text_buffer_get_selection_bounds(buffer, &start, &end);

#ifndef NO_NOTE_TITLE
	// Check title
  if (  gtk_text_iter_get_line(&start) == 0 )
	{
		gtk_text_buffer_get_iter_at_line(buffer,&start,0);
		gtk_text_iter_forward_to_line_end(&end);
		gchar *title = gtk_text_iter_get_text(&start,&end);

    // Title is different.
    if ( g_strcmp0 (title, _biji_note_obj_get_title(obj)) != 0 )
    {
      format_note_title(buffer);
      _biji_note_obj_set_title(obj,title);
      check_or_fix_title(obj);
    }
	}
#endif
		
	// If any other line, check for link at current line,
  // or for the whole note ?

  // The buffer has changed -> Save the note.
  //note_obj_save_note_using_buffer(obj);
  _biji_note_obj_propose_saving(obj);
    
	return FALSE;
}