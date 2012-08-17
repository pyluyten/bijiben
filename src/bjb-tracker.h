#ifndef _BIJI_TRACKER_H
#define _BIJI_TRACKER_H

#include <gtk/gtk.h>
#include <tracker-sparql.h>

#include <libbiji/libbiji.h>

//////////////////////////////////////////////////// TAGS

GList * tracker_tag_get_files(gchar *tag); // Free the result
gint tracker_tag_get_number_of_files(gchar *tag);
GList *get_all_tracker_tags(); // Free the result
void push_tag_to_tracker(gchar *tag);
void remove_tag_from_tracker(gchar *tag); // Regardless files
void push_existing_tag_to_note(gchar *tag,BijiNoteObj *note) ;
void remove_tag_from_note (gchar *tag, BijiNoteObj *note) ;

//////////////////////////////////////////////////// NOTES

void bijiben_push_note_to_tracker(BijiNoteObj *note); // Create or update
void biji_note_delete_from_tracker(BijiNoteObj *note);

#endif /*_BIJI_TRACKER_H*/