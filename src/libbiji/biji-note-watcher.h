#ifndef _NOTE_WATCHER_H
#define _NOTE_WATCHER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

// when the note is opened
void on_note_opened(GtkTextBuffer *buffer);

G_END_DECLS

#endif /*_NOTE_WATCHER_H*/
