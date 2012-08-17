 #ifndef _BIJI_NOTE_TAG_H_
#define _BIJI_NOTE_TAG_H_

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BIJI_TYPE_NOTE_TAG             (biji_note_tag_get_type ())
#define BIJI_NOTE_TAG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_NOTE_TAG, BijiNoteTag))
#define BIJI_NOTE_TAG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_NOTE_TAG, BijiNoteTagClass))
#define BIJI_NOTE_IS_TAG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_NOTE_TAG))
#define BIJI_NOTE_IS_TAG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_NOTE_TAG))
#define BIJI_NOTE_TAG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_NOTE_TAG, BijiNoteTagClass))

typedef struct _BijiNoteTagPrivate BijiNoteTagPrivate;
typedef struct _BijiNoteTagClass BijiNoteTagClass;
typedef struct _BijiNoteTag BijiNoteTag;

struct _BijiNoteTagClass
{
  GtkTextTagClass parent_class;
};

struct _BijiNoteTag
{
  GtkTextTag parent_instance;
  BijiNoteTagPrivate *priv;
};

GType biji_note_tag_get_type (void) G_GNUC_CONST;

// all defined tags for note buffers. dynamic tags not handled yet.
enum
{
  //NOTE_TAG_TITLE, // title is not serialized.
  NOTE_TAG_LIST,	// Bullets
  NOTE_TAG_LIST_ITEM, // Bullets. direction?
  NOTE_TAG_WIKI, // Link to another note
  NOTE_TAG_URL, // hyperlink
  // TODO link broken
  // TODO find match
  // TODO centered
  // TODO related to
  NOTE_TAG_BOLD,
  NOTE_TAG_ITALIC,
  NOTE_TAG_UNDERLINE,
  NOTE_TAG_STRIKE,
  NOTE_TAG_HIGHLIGHT,
  NOTE_TAG_MONOSPACE,
  NOTE_TAG_SMALL,
  NOTE_TAG_BIG,
  NOTE_TAG_HUGE,
  NOTE_TAG_NUMBER
};

//GtkTextTagTable * create_all_note_tags(void);
void create_all_note_tags(GtkTextBuffer *buf);

gchar * biji_note_tag_get_name(GtkTextTag *tag); // free tag name after use.

gboolean biji_note_tag_is_depth_tag(GtkTextTag *tag);
gint biji_note_tag_get_depth(GtkTextTag *tag);

G_END_DECLS

#endif /* _BIJI_NOTE_TAG_H_ */
