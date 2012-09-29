#ifndef _BIJI_TRACKER_H
#define _BIJI_TRACKER_H

#include <gtk/gtk.h>
#include <tracker-sparql.h>

#include <libbiji/libbiji.h>

/* Free the result */
GList * tracker_tag_get_files(gchar *tag);

gint tracker_tag_get_number_of_files(gchar *tag);

/* Free the result */
GList *get_all_tracker_tags();

void push_tag_to_tracker(gchar *tag);

void remove_tag_from_tracker(gchar *tag);

void push_existing_tag_to_note(gchar *tag,BijiNoteObj *note);

void remove_tag_from_note (gchar *tag, BijiNoteObj *note) ;

/* Insert or update */
void bijiben_push_note_to_tracker(BijiNoteObj *note);

void biji_note_delete_from_tracker(BijiNoteObj *note);

#endif /*_BIJI_TRACKER_H*/
