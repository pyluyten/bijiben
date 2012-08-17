#ifndef _SERIALIZE_H
#define _SERIALIZE_H

#include <libxml/xmlwriter.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct {
	xmlTextWriter *writer;
	gint depth;
	gint new_depth;
	gint xml_elements_debug;
	gboolean is_bullet;
	gboolean is_list_active;
} Serializer;


xmlBufferPtr note_obj_serialize(gpointer note, gint test);

void note_buffer_debug_xml(BijiNoteBuffer* note_buffer);

G_END_DECLS

#endif /*_SERIALIZE_H*/