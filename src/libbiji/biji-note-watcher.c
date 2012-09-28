#include "biji-note-book.h"
#include "biji-note-obj.h"
#include "biji-note-watcher.h"

#ifndef NO_NOTE_TITLE
#define NO_NOTE_TITLE
#endif


static void
check_or_fix_title (BijiNoteObj *obj)
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
