/* load a note which has tomboy format */

#ifndef _READ_TOMBOY_H
#define _READ_TOMBOY_H

#include <glib.h>

G_BEGIN_DECLS

// XML CHAR to GtkTextBuffer
void note_buffer_set_xml(gpointer note_buffer, const gchar *xml_string);

/* load a given NoteObj.
 * uses NoteObj->id->path to define the source 
 * returns 0 if ok */
int load_tomboy_note (gpointer note) ;

G_END_DECLS

#endif /*_READ_TOMBOY_H*/