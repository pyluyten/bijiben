/* libiji.h
 * Copyright (C) Pierre-Yves LUYTEN 2011 <py@luyten.fr>
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

#ifndef _LIB_BIJI_H
#define _LIB_BIJI_H

#define _LIBBIJI_INSIDE_H

#include <gtk/gtk.h>

#include "biji-date-time.h"
#include "biji-note-book.h"
#include "biji-note-buffer.h"
#include "biji-note-editor.h"
#include "biji-note-obj.h"

G_BEGIN_DECLS

#define BIJI_DEBUG(msg)   g_log(G_LOG_DOMAIN,G_LOG_LEVEL_DEBUG,msg);
#define BIJI_WARNING(msg) g_log(G_LOG_DOMAIN,G_LOG_LEVEL_WARNING,msg);

/* Get a collection of notes from tomboy format files */
BijiNoteBook *biji_book_new_from_dir(gchar *tomboy_format_folder);

// Create a Note from a tomboy.note file
BijiNoteObj* biji_note_get_new_from_file (gchar* path);

/* Get a blanck new note FIXME set path, dates, content */
BijiNoteObj* biji_note_get_new_from_string (gchar* title,gchar *folder);

/* Get all the tomboy notes */
BijiNoteBook *note_book_new_from_tomboy_dir();

G_END_DECLS

#undef _LIBBIJI_INSIDE_H

#endif /*_LIB_BIJI_H*/
