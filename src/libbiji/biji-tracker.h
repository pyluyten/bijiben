/* biji-tracker.h
 * Copyright (C) Pierre-Yves LUYTEN 2012 <py@luyten.fr>
 * 
 * bijiben is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * bijiben is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


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
