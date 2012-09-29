#include "biji-note-book.h"
#include "biji-note-obj.h"
#include "biji-note-watcher.h"

static void
check_or_fix_title (BijiNoteObj *obj)
{
  // TODO emit some signal afterward?
    
  BijiNoteBook * book = biji_note_obj_get_note_book(obj);
    
  if ( book == NULL )
    return ;

  if ( _biji_note_book_is_title_unique(book,_biji_note_obj_get_title(obj))) 
    return ;
}

