#include <glib/gprintf.h>

#include "biji-note-tag.h"

// currently a useless struct.
struct _BijiNoteTagPrivate
{
	int dummy;
};

#define BIJI_NOTE_TAG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BIJI_TYPE_NOTE_TAG, BijiNoteTagPrivate))

G_DEFINE_TYPE (BijiNoteTag, biji_note_tag, GTK_TYPE_TEXT_TAG);

static void
biji_note_tag_init (BijiNoteTag *self)
{
  self->priv = BIJI_NOTE_TAG_PRIVATE(self);
}

static void
biji_note_tag_finalize (GObject *object)
{
  G_OBJECT_CLASS (biji_note_tag_parent_class)->finalize (object);
}

static void
biji_note_tag_class_init (BijiNoteTagClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
		
  g_type_class_add_private (klass, sizeof (BijiNoteTagPrivate));
  object_class->finalize = biji_note_tag_finalize;
}

static void 
create_biji_note_tag(GtkTextBuffer *buffer,gint tag_id)
{
  BijiNoteTag *result ;

  switch (tag_id)
  {

    /*
    case NOTE_TAG_TITLE:
	    result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","_title",
	                        "scale", PANGO_SCALE_XX_LARGE,
	                        "foreground", "blue",
                          "underline", PANGO_UNDERLINE_SINGLE,NULL);
    break;
    */
			
  case NOTE_TAG_BOLD:
	result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","bold",
	                      "weight",PANGO_WEIGHT_BOLD,NULL);
    break;

  case NOTE_TAG_ITALIC:
	result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","italic",
	                      "style",PANGO_STYLE_ITALIC,NULL);
	break;

  case NOTE_TAG_UNDERLINE:
  result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","underline",
	                      "underline",PANGO_UNDERLINE_SINGLE,NULL);
  break ; 

  case NOTE_TAG_STRIKE:
    result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","strikethrough",
                          "strikethrough",TRUE,NULL);
	break;

  case NOTE_TAG_HIGHLIGHT:
    result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","highlight",
                          "background","yellow",NULL);
	break;

  case NOTE_TAG_MONOSPACE:
    result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","monospace",
                          "family","monospace",NULL);
    break;

  case NOTE_TAG_SMALL:
	  result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","size:small",
	                      "scale", PANGO_SCALE_SMALL,NULL);
	  break;

  case NOTE_TAG_BIG:
	  result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","size:large",
	                      "scale", PANGO_SCALE_X_LARGE,NULL);
	  break;

	case NOTE_TAG_HUGE:
	  result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","size:huge",
	                      "scale", PANGO_SCALE_XX_LARGE,NULL);
  	break;

  case NOTE_TAG_WIKI:
    result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","link:internal",
                                    "foreground", "blue",
                                    "underline", PANGO_UNDERLINE_SINGLE,
                                    NULL);
	  break;

  case NOTE_TAG_URL:
    result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","link:url",
                                    "foreground", "blue",
                                    "underline", PANGO_UNDERLINE_SINGLE,
                                    NULL);
  	break;


  case NOTE_TAG_LIST_ITEM:
    result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","list-item",
                          NULL);
    break;

  case NOTE_TAG_LIST:
	  result = g_object_new(BIJI_TYPE_NOTE_TAG,"name","list",
                          NULL);
    break;
 
		  
  default : 
    g_message("TAG NOT DEFINED. BIJI WILL NOT WORK PROPERLY.");
  }	
  gtk_text_tag_table_add(gtk_text_buffer_get_tag_table(buffer),
		                       GTK_TEXT_TAG(result));
}

// create all tags as defined above
void create_all_note_tags(GtkTextBuffer *buffer)
{
    int i;

    for ( i=0 ; i< NOTE_TAG_NUMBER ; i++ )
    {
        create_biji_note_tag(buffer,i);
    }
}

gchar *
biji_note_tag_get_name(GtkTextTag *tag)
{ 
    gchar *result;
    g_object_get(G_OBJECT(tag),"name",&result,NULL);
    return result;
}

gboolean
biji_note_tag_is_depth_tag(GtkTextTag *tag)
{
    if (tag == NULL)
        return FALSE;

    gchar* name = biji_note_tag_get_name(tag);
    gboolean result =g_str_has_prefix(name, "depth");
    g_free(name);
    return (result);
}

gint
biji_note_tag_get_depth(GtkTextTag *tag)
{
    if (biji_note_tag_is_depth_tag(tag)) 
    {
        gchar* name = biji_note_tag_get_name(tag);
        gchar **strings = g_strsplit(name, ":", 2);
        gint64 depth = g_ascii_strtoll (strings[1],NULL,0);
        g_strfreev(strings);
        g_free(name);
        return depth;
    }
	
    return 0;
}
