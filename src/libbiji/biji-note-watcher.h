#ifndef _NOTE_WATCHER_H
#define _NOTE_WATCHER_H

#include <gtk/gtk.h>

#include "biji-note-editor.h"

G_BEGIN_DECLS

// when the note is opened
void on_note_opened(GtkTextBuffer *buffer);

// at each note modification
gboolean watch_buffer(GtkTextBuffer *buffer,BijiNoteEditor *editor);

// TODO 
// spellcheck

G_END_DECLS

#endif /*_NOTE_WATCHER_H*/