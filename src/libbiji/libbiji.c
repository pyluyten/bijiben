/* This file to be deleted soon */

#include "libbiji.h"
#include "biji-note-buffer.h"
#include "biji-note-editor.h"
#include "biji-note-obj.h"
#include "biji-note-tag.h"
#include "biji-read-tomboy.h"

/* Todo : check file type */
BijiNoteObj*
biji_note_get_new_from_file (gchar* path)
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

BijiNoteBook *
biji_book_new_from_dir(gchar *tomboy_format_folder)
{
  if (tomboy_format_folder != NULL)
  {
    return _biji_note_book_get_new_from_dir(tomboy_format_folder);
  }
  return NULL ;
}

BijiNoteBook *
note_book_new_from_tomboy_dir()
{
    gchar *tomboy_dir ;
    tomboy_dir = g_strdup_printf("%s/tomboy",g_get_user_data_dir());
    return _biji_note_book_get_new_from_dir(tomboy_dir);
}

/*
int note_book_get_notes_number(BijiNoteBook *book)
{
  if (BIJI_IS_NOTE_BOOK(book))
  {
    return _biji_book_get_number(book);
  }
  else return 0 ;
}
*/
