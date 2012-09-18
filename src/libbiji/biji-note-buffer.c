#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include "biji-note-buffer.h"
#include "biji-note-editor.h"
#include "biji-note-obj.h"
#include "biji-note-tag.h"
#include "biji-serialize.c" // only for debug. to be removed after debug.

#ifndef NO_NOTE_TITLE
#define NO_NOTE_TITLE
#endif

// GOBJECT

struct _BijiNoteBufferPrivate
{
  BijiNoteObj *note; // the note to which this buffer belongs.
  GSList *active_tags; // used to tmp list of GtkTextTags
};

#define BIJI_NOTE_BUFFER_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BIJI_TYPE_NOTE_BUFFER, BijiNoteBufferPrivate))

G_DEFINE_TYPE (BijiNoteBuffer, biji_note_buffer, GTK_TYPE_TEXT_BUFFER);

static void
biji_note_buffer_init (BijiNoteBuffer *self)
{
  self->priv = 
  G_TYPE_INSTANCE_GET_PRIVATE(self,BIJI_TYPE_NOTE_BUFFER,BijiNoteBufferPrivate);

  self->priv->active_tags = NULL ;
  self->priv->note = NULL ;
}

static void
biji_note_buffer_finalize (GObject *object)
{
  BijiNoteBuffer *self = BIJI_NOTE_BUFFER(object) ;
    
  if ( self->priv->active_tags != NULL )
        g_slist_free(self->priv->active_tags);
    
  G_OBJECT_CLASS (biji_note_buffer_parent_class)->finalize (object);
}

static void
biji_note_buffer_class_init (BijiNoteBufferClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (BijiNoteBufferPrivate));

  object_class->finalize = biji_note_buffer_finalize;
}

BijiNoteBuffer *
create_note_buffer(gpointer note_obj) // FIXME create_note_buffer(NoteBook *book);
{
  BijiNoteBuffer *ret;
  BijiNoteObj *note = BIJI_NOTE_OBJ(note_obj);
		
  ret = g_object_new(BIJI_TYPE_NOTE_BUFFER,NULL);
  ret->priv->note = note ;
  GtkTextBuffer *buf = GTK_TEXT_BUFFER(ret);

  create_all_note_tags(buf);

#ifndef NO_NOTE_TITLE
  g_message("ifndef ne marche pas");
  gtk_text_buffer_set_text(buf,_biji_note_obj_get_title(note),-1);
#endif
    
  return ret;
}

gpointer
note_buffer_get_obj(BijiNoteBuffer *buf)
{
  return (gpointer)buf->priv->note ;
}

// PRIVATE API

static void
note_buffer_add_active_tag (BijiNoteBuffer *self, GtkTextTag *tag)
{		
  g_return_if_fail(self != NULL);
  g_return_if_fail(tag != NULL);
  g_return_if_fail(BIJI_IS_NOTE_BUFFER(self));
		
  if (g_strcmp0(biji_note_tag_get_name(tag), "depth") == 0)
		return;

  GSList *tags = self->priv->active_tags;
  while (tags != NULL) 
  {
	if (g_strcmp0(biji_note_tag_get_name(tags->data),biji_note_tag_get_name(tag)) == 0) 
    {
	  return;
    }
	tags = tags->next;
  }
  self->priv->active_tags = g_slist_prepend(self->priv->active_tags, tag);
}

static void
note_buffer_remove_active_tag (BijiNoteBuffer *self, GtkTextTag *tag)
{
  GSList *tags = self->priv->active_tags;
  while (tags != NULL) 
  {
    if (g_strcmp0(biji_note_tag_get_name(tags->data),biji_note_tag_get_name(tag)) == 0) 
    {
      self->priv->active_tags = g_slist_remove(self->priv->active_tags, tags->data);
      return;
    }
      
	tags = tags->next;
  }
}

static void
note_buffer_add_active_tag_by_name (BijiNoteBuffer *self, const gchar *tag_name)
{

  GtkTextTagTable *table;

  table = gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(self));
  note_buffer_add_active_tag(self, gtk_text_tag_table_lookup(table, tag_name));
}

static void
note_buffer_remove_active_tag_by_name (BijiNoteBuffer *self, const gchar *tag_name)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(tag_name != NULL);
	g_return_if_fail(BIJI_IS_NOTE_BUFFER(self));

  GtkTextTagTable *table;
  table = gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(self));
  note_buffer_remove_active_tag(self,gtk_text_tag_table_lookup(table, 
	                                                           tag_name));
}

static GSList*
note_buffer_get_active_tags (BijiNoteBuffer *self)
{	
  g_return_val_if_fail(BIJI_IS_NOTE_BUFFER(self), NULL);

  return self->priv->active_tags;
}

static void
note_buffer_clear_active_tags (BijiNoteBuffer *self)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(BIJI_IS_NOTE_BUFFER(self));

	g_slist_free(self->priv->active_tags);
	self->priv->active_tags = NULL;
}

static void
note_buffer_set_active_tags (BijiNoteBuffer *self, GSList *tags)
{		
	g_return_if_fail(self != NULL);
	g_return_if_fail(tags != NULL);
	g_return_if_fail(BIJI_IS_NOTE_BUFFER(self));

	// Free the list
	g_slist_free(self->priv->active_tags);
	self->priv->active_tags = NULL;

	// Copy elements of given list, but ignore depth tags
	while (tags != NULL) 
    {
		GtkTextTag *tag = GTK_TEXT_TAG(tags->data);
		if (!g_str_has_prefix(biji_note_tag_get_name(tag), "depth")) 
        {
			self->priv->active_tags = g_slist_prepend(self->priv->active_tags, tag);
		}
		tags = tags->next;
	}
}

void
apply_active_tags(GtkTextBuffer *buffer, 
                  GtkTextIter *iter, 
                  const gchar *input, 
                  gpointer user_data)
{
	GtkTextIter start_iter;
	GSList *active_tags = note_buffer_get_active_tags(BIJI_NOTE_BUFFER(buffer));

	// The first line is always the title. Don't apply tags there
	if (gtk_text_iter_get_line(iter) == 0) {
		return;
	}

	// First remove all tags, then apply all active tags
	start_iter = *iter;
	gtk_text_iter_backward_chars(&start_iter, g_utf8_strlen(input, -1));
	gtk_text_buffer_remove_all_tags(buffer, &start_iter, iter);

	while (active_tags != NULL && active_tags->data != NULL) 
    {
        g_message("we have a tag to appy :%s",biji_note_tag_get_name(active_tags->data));
		gtk_text_buffer_apply_tag(buffer, active_tags->data, &start_iter, iter);
		active_tags = active_tags->next;
	}
}

static GtkTextTag*
note_buffer_find_depth_tag(GtkTextIter* iter)
{
	GSList *tags = gtk_text_iter_get_tags(iter);
	while (tags != NULL) 
    {
		if (biji_note_tag_is_depth_tag(tags->data)) 
        {
			gchar *test ;
			g_object_get(G_OBJECT(tags->data),"name",&test,NULL);
			return tags->data;
		}
		tags = tags->next;
	}
	return NULL;
}

static GtkTextTag*
note_buffer_get_depth_tag(GtkTextBuffer *buffer, gint depth)
{
  GtkTextTag *tag;
	gchar depth_str[5] = {0};
	gchar *tag_name;

	if (depth < 1) 
    {
		g_printerr("ERROR: buffer_get_depth_tag(): depth must be at least 1. Not: %i \n", depth);
	}

	g_sprintf(depth_str, "%i", depth);
	tag_name = g_strconcat("depth", ":", depth_str, NULL);

    tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer),
	                                 tag_name);

	if (tag == NULL) 
	{
		BijiNoteTag *new_depth_tag ;
		new_depth_tag = g_object_new(BIJI_TYPE_NOTE_TAG,"name",g_strconcat("depth", ":", depth_str, NULL),
                          "indent",-20,
                          "left-margin",depth*25,
                          NULL);
			
		tag = GTK_TEXT_TAG(new_depth_tag);
		gtk_text_tag_table_add (gtk_text_buffer_get_tag_table(buffer),tag);
	}

	g_free(tag_name);
	return tag;
}

void
note_buffer_update_active_tags (BijiNoteBuffer *self)
{		
	g_return_if_fail(self != NULL);
	g_return_if_fail(BIJI_IS_NOTE_BUFFER(self));

	// Clean the list of active tags
	note_buffer_clear_active_tags(self);

	// Add tags at this location
	GtkTextIter location;
	gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(self), &location, gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(self)));
	GSList *tags = gtk_text_iter_get_tags(&location);
	if (tags != NULL) {
		note_buffer_set_active_tags(self, tags);
	}

	// Go the beginning of line and check if there is a bullet.
	// If yes, add list-item and list tags
	gtk_text_iter_set_line_offset(&location, 0);
	if (note_buffer_find_depth_tag(&location) != NULL) {
		// Add tags
		note_buffer_add_active_tag_by_name(self, "list-item");
		note_buffer_add_active_tag_by_name(self, "list");
	}
}

#define NUM_BULLETS 3
static const gunichar bullets[NUM_BULLETS] =  {0x2023,0x2022,0x2218 };

static gboolean
is_bullet(gunichar string)
{
	gint i;
	GString *tested;
	tested = g_string_new("");
	tested = g_string_append_unichar (tested,string);
	
	for ( i=0 ; i<NUM_BULLETS ; i++ )
	{
			GString *comp;
			comp = g_string_new("");
			comp = g_string_append_unichar (comp,bullets[i]);
			// if it's a bullet, free all and return true
			if ( g_strcmp0 (tested->str,comp->str) == 0 )
			{
				g_string_free(tested,TRUE);
			  g_string_free(comp,TRUE);
			  return TRUE ;
			}
		  // else, just free the comparison string for next round
			g_string_free(comp,TRUE);
	}
	// not any bullet. free the test string and return false
	g_string_free(tested,TRUE);
  return FALSE;
}

static gchar*
get_bullet_by_depth_tag(GtkTextTag *tag) 
{
	return get_bullet_by_depth(biji_note_tag_get_depth(tag));
}

static void
add_bullets(BijiNoteBuffer *buffer, gint start_line, gint end_line, GtkTextTag *depth_tag)
{
  GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
  gint i = 0;
  GtkTextIter start_iter, end_iter;
  gint total_lines = gtk_text_buffer_get_line_count(buf);

  // For each selected line
  for (i = start_line; i <= end_line; i++) 
  {
    gtk_text_buffer_get_iter_at_line(buf, &start_iter, i);
    // If there is already a bullet, skip this line
    if (note_buffer_find_depth_tag(&start_iter)) {
      continue;
    }

    // Insert bullet character
    gtk_text_buffer_insert(buf,&start_iter,
	                       get_bullet_by_depth_tag(depth_tag),-1);

    // Remove existing tags from the bullet and add the <depth> tag
    gtk_text_buffer_get_iter_at_line_offset(buf, &start_iter, i, 0);
    gtk_text_buffer_get_iter_at_line_offset(buf, &end_iter, i, 2);
    gtk_text_buffer_remove_all_tags(buf, &start_iter, &end_iter);

    gtk_text_buffer_apply_tag(buf, depth_tag, &start_iter, &end_iter);

    // Surround line (starting after BULLET) with "list-item" tags
    gtk_text_buffer_get_iter_at_line_offset(buf, &start_iter, i, 2); // Jump bullet
      
    if (i == end_line) 
    {
      gtk_text_buffer_get_iter_at_line(buf, &end_iter, i);
      gtk_text_iter_forward_to_line_end(&end_iter);
    } 

    else 
    {
      gtk_text_buffer_get_iter_at_line(buf, &end_iter, i + 1);
    }
        
    gtk_text_buffer_apply_tag_by_name(buf, "list-item", &start_iter, &end_iter);
  }

  // Surround everything it with "list" tags
  // Check line above and below. If one or both are bullet lines, 
  // include the newline chars at the beginning and the end
  // This is done, so that there are no gaps in the <list> tag and that it really surrounds the whole list.

  // Set start iter
  if (start_line > 0) {
    gtk_text_buffer_get_iter_at_line(buf, &start_iter, start_line - 1);
    if (note_buffer_find_depth_tag(&start_iter) != NULL) {
      gtk_text_iter_forward_to_line_end(&start_iter);
	} 
    else 
    {
	  gtk_text_buffer_get_iter_at_line(buf, &start_iter, start_line);
    }
  } 
    
  else 
  {
   gtk_text_buffer_get_iter_at_line(buf, &start_iter, start_line);
  }

	// Set end iter
  if (end_line < total_lines - 1) 
  {
    gtk_text_buffer_get_iter_at_line(buf, &end_iter, end_line + 1);
    if (note_buffer_find_depth_tag(&end_iter) == NULL) 
    {
      gtk_text_buffer_get_iter_at_line(buf, &end_iter, end_line);
      gtk_text_iter_forward_to_line_end(&end_iter);
    }
  } 
  else 
  {
    gtk_text_buffer_get_iter_at_line(buf, &end_iter, end_line);
    gtk_text_iter_forward_to_line_end(&end_iter);
  }

  gtk_text_buffer_apply_tag_by_name(buf, "list", &start_iter, &end_iter);
}

static void
remove_bullets(BijiNoteBuffer *buffer, GtkTextIter *start_iter, GtkTextIter *end_iter)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	gint i = 0;
	GtkTextIter start = *start_iter;
	GtkTextIter end = *end_iter;
	gint start_line = gtk_text_iter_get_line(&start);
	gint end_line = gtk_text_iter_get_line(&end);
		
	// Remove tags
	gtk_text_iter_set_line_offset(&start, 0);
	gtk_text_iter_set_line_offset(&end, 0);
	gtk_text_iter_forward_to_line_end(&end);

	// Include the newline char before and after this line
	gtk_text_iter_backward_char(&start);
	gtk_text_iter_forward_char(&end);

	gtk_text_buffer_remove_tag_by_name(buf, "list-item", &start, &end);
	gtk_text_buffer_remove_tag_by_name(buf, "list", &start, &end);
		
	buffer_selection_toggle_tag_by_name(GTK_TEXT_BUFFER(buffer),"list");
	buffer_selection_toggle_tag_by_name(GTK_TEXT_BUFFER(buffer),"list-item");
		
	// Remove bullets
	for (i = start_line; i <= end_line; i++) {
		gtk_text_buffer_get_iter_at_line(buf, &start, i);
		gtk_text_buffer_get_iter_at_line(buf, &end, i);
		gtk_text_iter_forward_chars(&end, 2);
		gtk_text_buffer_delete(buf, &start, &end);
	}
}

static void
note_buffer_increase_indent(BijiNoteBuffer *buffer, gint start_line, gint end_line)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start_iter, end_iter;
	GtkTextTag *old_tag;
	GtkTextTag *new_tag;
	gint i;

	for (i = start_line; i <= end_line; i++) {

		gtk_text_buffer_get_iter_at_line(buf, &start_iter, i);
		end_iter = start_iter;

		old_tag = note_buffer_find_depth_tag(&start_iter);

		gint depth = 0;
		// If already bullet list, remove bullet and tags
		if (old_tag != NULL) {
			depth = biji_note_tag_get_depth(old_tag);
			gtk_text_iter_set_line_offset(&end_iter, 2);

			// Remove old tag
			gtk_text_buffer_remove_tag(buf, old_tag, &start_iter, &end_iter);

			// Delete old bullet
			gtk_text_buffer_delete(buf, &start_iter, &end_iter);
		}

		depth++;
		new_tag = note_buffer_get_depth_tag(buf, depth);

		// Add new bullet with new tag
		add_bullets(buffer, i, i, new_tag);
	}

  note_buffer_update_active_tags(buffer);
}

static void
note_buffer_decrease_indent(BijiNoteBuffer *buffer, gint start_line, gint end_line)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start_iter, end_iter;
	GtkTextTag *old_tag;
	GtkTextTag *new_tag;
	gint i;

	for (i = start_line; i <= end_line; i++) {

		gtk_text_buffer_get_iter_at_line(buf, &start_iter, i);
		end_iter = start_iter;

		old_tag = note_buffer_find_depth_tag(&start_iter);

		if (old_tag != NULL) {
			gint depth = biji_note_tag_get_depth(old_tag);
			gtk_text_iter_set_line_offset(&end_iter, 2);

			if (depth == 1) {
				g_message("remove bullets");
				remove_bullets(buffer, &start_iter, &end_iter);
				continue;
			}

			// Remove old tag
			gtk_text_buffer_remove_tag(buf, old_tag, &start_iter, &end_iter);

			// Delete old bullet
			gtk_text_buffer_delete(buf, &start_iter, &end_iter);

			depth--;
			new_tag = note_buffer_get_depth_tag(buf, depth);

			// Add new bullet with new tag
			add_bullets(buffer, i, i, new_tag);
		}
	}
	note_buffer_update_active_tags(buffer);
}

static gboolean
line_needs_bullet(GtkTextBuffer *buffer, gint line_number)
{
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_line(buffer, &iter, line_number);

	while (!gtk_text_iter_ends_line(&iter)) {
		switch (gtk_text_iter_get_char(&iter))
		{
		case ' ':
			gtk_text_iter_forward_char(&iter);
			break;

		case '*':
		case '-':
			gtk_text_iter_forward_char(&iter);
			return (gtk_text_iter_get_char(&iter) == ' ');
			break;

		default:
			return FALSE;
		}
	}
	return FALSE;
}

static void
augment_selection(BijiNoteBuffer *buffer, GtkTextIter *start, GtkTextIter *end)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);

	GtkTextTag *start_depth = note_buffer_find_depth_tag(start);
	GtkTextTag *end_depth = note_buffer_find_depth_tag(end);

	GtkTextIter inside_end = *end;
	gtk_text_iter_backward_char(&inside_end);

	GtkTextTag *inside_end_depth = note_buffer_find_depth_tag(&inside_end);

	// Start inside bullet region
	if (start_depth != NULL) {
		gtk_text_iter_set_line_offset(start, 2);
		gtk_text_buffer_select_range(buf, start, end);
	}

	// End inside another bullet
	if (inside_end_depth != NULL) {
		gtk_text_iter_set_line_offset(end, 2);
		gtk_text_buffer_select_range(buf, start, end);
	}

	// Check if the end is right before the start of the bullet
	if (end_depth != NULL){
		gtk_text_iter_set_line_offset(end, 2);
		gtk_text_buffer_select_range(buf, start, end);
	}
}

// SEMI PUBLIC API --> libbiji has to map this.

// FIXME : using GString means the gchar* should be freed.
// while some gunichar -> utf8 func could be used instead
gchar *
get_bullet_by_depth( gint depth)
{
	GString *result;

	result = g_string_new("");
	result = g_string_append_unichar (result,bullets[depth%3]);
	result = g_string_append(result," ");
	return (g_string_free(result,FALSE));
}



// TAGS

gboolean 
is_active_tag(BijiNoteBuffer *note_buffer, gchar *tag_name)
{
  GtkTextBuffer *buf = GTK_TEXT_BUFFER(note_buffer);
  GtkTextTagTable *table = gtk_text_buffer_get_tag_table(buf);
  GtkTextTag *tag = gtk_text_tag_table_lookup(table,tag_name);
  GtkTextIter start, select_end;

  if(gtk_text_buffer_get_selection_bounds(buf,&start,&select_end))
  {
    if ( note_buffer_find_depth_tag(&start))
    {
      gtk_text_iter_set_line_offset(&start,2);
    }
    return gtk_text_iter_has_tag(&start,tag) || gtk_text_iter_has_tag(&select_end,tag);
  }
    
  else
  {
    // TODO
    //return (find(m_active_tags.begin(), m_active_tags.end(), tag) != m_active_tags.end());
	return FALSE ;
  }
}


// FIXME use "is active tag" instead
void
buffer_selection_toggle_tag_by_name(GtkTextBuffer *buffer,gchar *name)
{
  GtkTextIter start;
  GtkTextIter end;
  GtkTextTag * tag ;

  tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer),name);

  // Some Selection
  if (gtk_text_buffer_get_selection_bounds(buffer,&start,&end))
  {
    // Skip bullet
    if ( note_buffer_find_depth_tag(&start))
	  gtk_text_iter_set_line_offset(&start,2);
			
	if (gtk_text_iter_has_tag(&start,tag) || gtk_text_iter_has_tag(&end,tag)) 
	{
	  gtk_text_buffer_remove_tag(buffer,tag,&start,&end);
    }
	else 
    {
	  gtk_text_buffer_apply_tag(buffer,tag,&start,&end);
	}
  }
    
  // No selection
  else
  {
    note_buffer_add_active_tag(BIJI_NOTE_BUFFER(buffer), tag);
  }

  // note might need to be saved
  _biji_note_obj_mark_as_need_save(BIJI_NOTE_BUFFER(buffer)->priv->note);
}

void
note_buffer_remove_tags(GtkTextBuffer *buffer)
{
  GtkTextIter start;
  GtkTextIter end;

	if (gtk_text_buffer_get_selection_bounds(buffer,&start,&end))
	{
		GtkTextTag *depth = note_buffer_find_depth_tag(&start);
		while (depth)
		{
			note_buffer_disable_bullets(BIJI_NOTE_BUFFER(buffer));
			depth = note_buffer_find_depth_tag(&start);
		}
	  gtk_text_buffer_remove_all_tags (buffer,&start,&end);
	}
}

gboolean
augment_font_size(GtkTextBuffer *gtk_text_buffer)
{
	BijiNoteBuffer *buf = BIJI_NOTE_BUFFER(gtk_text_buffer);
		
  if ( is_active_tag(buf,"size:small") )
	{
		buffer_selection_toggle_tag_by_name(gtk_text_buffer,"size:small");
	  return TRUE;
	}
	else if ( is_active_tag(buf,"size:large") )
	{
		buffer_selection_toggle_tag_by_name(gtk_text_buffer,"size:large");
		buffer_selection_toggle_tag_by_name(gtk_text_buffer,"size:huge");
		return TRUE;
	}
	else if ( is_active_tag(buf,"size:huge") )
	{
		return FALSE ;
	}
	else
	{
	 buffer_selection_toggle_tag_by_name(gtk_text_buffer,"size:large");
	 return TRUE ;
	}
}

gboolean
decrease_font_size(GtkTextBuffer *gtk_text_buffer)
{
	BijiNoteBuffer *buf = BIJI_NOTE_BUFFER(gtk_text_buffer);
		
	if ( is_active_tag(buf,"size:small") )
	{
		return FALSE;
	}
	else if ( is_active_tag(buf,"size:large") )
	{
		buffer_selection_toggle_tag_by_name(gtk_text_buffer,"size:large");		
		return TRUE;
	}
	else if ( is_active_tag(buf,"size:huge") )
	{
		buffer_selection_toggle_tag_by_name(gtk_text_buffer,"size:huge");
		buffer_selection_toggle_tag_by_name(gtk_text_buffer,"size:large");
		return TRUE;
	}
	else
	{
	    buffer_selection_toggle_tag_by_name(gtk_text_buffer,"size:small");
	    return TRUE ;
	}
}


// #ifndef NO_NOTE_TITLE
void
format_note_title(GtkTextBuffer *buffer)
{
  GtkTextIter start,end;

  gtk_text_buffer_get_iter_at_line(buffer,&start,0);
  end = start;
  gtk_text_iter_forward_visible_line(&end);
  GtkTextTagTable *table =  gtk_text_buffer_get_tag_table(buffer);
  gtk_text_buffer_apply_tag(buffer,
	                        gtk_text_tag_table_lookup(table,"_title"),
	                        &start,&end);
}
// #endif

// BULLETS

void
note_buffer_enable_bullets(BijiNoteBuffer *buffer)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start_iter, end_iter;

	if (gtk_text_buffer_get_has_selection(buf)) {
		// Something is selected
		gtk_text_buffer_get_selection_bounds(buf, &start_iter, &end_iter);
		add_bullets(buffer, 
		            gtk_text_iter_get_line(&start_iter), 
		            gtk_text_iter_get_line(&end_iter), 
		            note_buffer_get_depth_tag(buf, 1));

	} else {
		// Nothing is selected
		int line;
		gtk_text_buffer_get_iter_at_mark(buf, &start_iter, gtk_text_buffer_get_insert(buf));
		line = gtk_text_iter_get_line(&start_iter);
		add_bullets(buffer, 
		            line, 
		            line, 
		            note_buffer_get_depth_tag(buf, 1));

		note_buffer_add_active_tag_by_name(buffer, "list-item");
		note_buffer_add_active_tag_by_name(buffer, "list");
	}


}

void
note_buffer_disable_bullets(BijiNoteBuffer *buffer)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start_iter, end_iter;

	if (gtk_text_buffer_get_has_selection(buf)) {
		// Something is selected
		gtk_text_buffer_get_selection_bounds(buf, &start_iter, &end_iter);
		remove_bullets(buffer, &start_iter, &end_iter);

		note_buffer_remove_active_tag_by_name(buffer, "list-item");
		note_buffer_remove_active_tag_by_name(buffer, "list");

	} else {
		// Nothing is selected
		gtk_text_buffer_get_iter_at_mark(buf, &start_iter, gtk_text_buffer_get_insert(buf));
		gtk_text_buffer_get_iter_at_mark(buf, &end_iter, gtk_text_buffer_get_insert(buf));
		remove_bullets(buffer, &start_iter, &end_iter);

		note_buffer_remove_active_tag_by_name(buffer, "list-item");
		note_buffer_remove_active_tag_by_name(buffer, "list");
	}
}

gboolean
augment_depth(BijiNoteBuffer *note_buf)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER(note_buf);
  GtkTextIter start_iter, end_iter;
  gint start_line, end_line;

  if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter))
  {
    start_line = gtk_text_iter_get_line(&start_iter);
    end_line = gtk_text_iter_get_line(&end_iter);
    note_buffer_increase_indent(note_buf, start_line, end_line);
  }
    
  else 
  {
    gtk_text_buffer_get_iter_at_mark(buffer, 
	                                 &start_iter, 
	                                 gtk_text_buffer_get_insert(buffer));
	start_line = gtk_text_iter_get_line(&start_iter);
	note_buffer_increase_indent(note_buf, start_line, start_line);
  }
		
  gtk_text_buffer_set_modified(buffer, TRUE);
  return FALSE;
}

gboolean
decrease_depth(BijiNoteBuffer *note_buf)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(note_buf);
	GtkTextIter start_iter, end_iter;
	gint start_line, end_line;

	if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter))
	{
		start_line = gtk_text_iter_get_line(&start_iter);
		end_line = gtk_text_iter_get_line(&end_iter);
		note_buffer_decrease_indent(note_buf, start_line, end_line);
	} 
	else 
	{
		gtk_text_buffer_get_iter_at_mark(buffer, 
		                                 &start_iter, 
		                                 gtk_text_buffer_get_insert(buffer));
		start_line = gtk_text_iter_get_line(&start_iter);
		note_buffer_decrease_indent(note_buf, start_line, start_line);
	}
		
	gtk_text_buffer_set_modified(buffer, TRUE);
	return FALSE;
}




// Funct for Signals / Input management

gboolean
note_buffer_add_new_line(GtkTextBuffer *buf)
{
  BijiNoteBuffer *buffer = BIJI_NOTE_BUFFER(buf);
  GtkTextIter iter;
  GtkTextTag *depth_tag;
  gint line;

  gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));

  // If title : add new line, remove tag _title and that's all folks.
  line = gtk_text_iter_get_line(&iter);

#ifndef NO_NOTE_TITLE
  if ( line == 0 )
  {
    gtk_text_buffer_insert(buf, &iter, "\n", -1);
    gtk_text_buffer_insert(buf, &iter, "\n", -1);
    line = gtk_text_iter_get_line(&iter);
    GtkTextIter end = iter ;
    gtk_text_iter_forward_to_line_end(&end);
    gtk_text_buffer_remove_all_tags(buf, &iter, &end);
    return TRUE ;
  }
#endif

  // If line starts with a bullet
  gtk_text_iter_set_line_offset(&iter, 0);
  depth_tag = note_buffer_find_depth_tag(&iter);
  
  if (depth_tag != NULL) 
  {
    gtk_text_iter_forward_to_line_end(&iter);

    // If line is not empty, add new bullet.
    if (gtk_text_iter_get_line_offset(&iter) > 2) 
    {
      GSList *tmp;

      // Remove all tags but <list> from active tags
      tmp = g_slist_copy(note_buffer_get_active_tags(BIJI_NOTE_BUFFER(buffer)));
      note_buffer_clear_active_tags(buffer);
      note_buffer_add_active_tag_by_name(buffer, "list");

      // Insert newline and bullet
      gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));
      if (gtk_text_iter_get_line_offset(&iter) < 2) 
      {
        gtk_text_iter_set_line_offset(&iter, 2);
      }
      gtk_text_buffer_insert(buf, &iter, "\n", -1);
      //gtk_text_view_scroll_mark_onscreen(view, gtk_text_buffer_get_insert(buf));
      line = gtk_text_iter_get_line(&iter);
      add_bullets(buffer, line, line, depth_tag);

      // Add all tags back to active tags
      note_buffer_set_active_tags(buffer, tmp);
      // */

      return TRUE;
    } 
      
    else // Else remove bullet. il y a depth tag mais ligne vide. marche ?
    {        
      // Remove bullet and insert newline
      GtkTextIter start = iter;
      GtkTextIter end = iter;

      // Disable list and list-item tags
      note_buffer_remove_active_tag_by_name(buffer, "list-item");
      note_buffer_remove_active_tag_by_name(buffer, "list");

      // Delete the bullet and the last newline
      gtk_text_iter_set_line_offset(&start, 0);
      gtk_text_iter_set_line_offset(&end, 0);
      gtk_text_iter_set_line_offset(&iter, 0);
      gtk_text_iter_forward_to_line_end(&end);
      gtk_text_iter_forward_to_line_end(&iter);
      gtk_text_iter_forward_char(&iter);
      gtk_text_buffer_remove_all_tags(buf, &start, &iter);
      gtk_text_buffer_delete(buf, &start, &end);

      gtk_text_buffer_insert(buf, &end, "\n", -1);

      //gtk_text_view_scroll_mark_onscreen(ui->view, gtk_text_buffer_get_insert(buf));

      note_buffer_update_active_tags(buffer);
      // TODO => ui may have to be updated
      return TRUE;
    }
  }

  // There is no depth tag. 
  else 
  {
    g_message("NOTE BUFFER ADD NEW LINE : else") ;
      
	// If line start with a "- " or "- *" */
    if (line_needs_bullet(buf, gtk_text_iter_get_line(&iter))) 
    {
      GSList *tmp;
      GtkTextIter end_iter;

      // Create a bullet line for the current line.

      gtk_text_iter_set_line_offset(&iter, 0);
      end_iter = iter;
      line = gtk_text_iter_get_line(&iter);

      // Skip trailing spaces
      while (gtk_text_iter_get_char(&end_iter) == ' ') 
      {
        gtk_text_iter_forward_char(&end_iter);
      }
          
      // Skip "* " or "- " 
      gtk_text_iter_forward_chars(&end_iter, 2);

      // Delete this part
      gtk_text_buffer_delete(buf, &iter, &end_iter);

      // ruse du 24 avril 2012 =
      //note_buffer_increase_indent(buffer,line,line);

      ///* ci dessous démarre le code avant modif du 2012 avril 24.

      // Add bullet for this line
      add_bullets(buffer, line, line, 
                  note_buffer_get_depth_tag(GTK_TEXT_BUFFER(buffer), 1));


      // TODO: Copied from above

      // Remove all tags but <list> from active tags
      tmp = g_slist_copy(note_buffer_get_active_tags(buffer));
      note_buffer_clear_active_tags(buffer);
      note_buffer_add_active_tag_by_name(buffer, "list");

      // Insert newline and bullet
      gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));
      gtk_text_buffer_insert(buf, &iter, "\n", -1);
      line = gtk_text_iter_get_line(&iter);
      add_bullets(buffer, line, line, 
			      note_buffer_get_depth_tag(GTK_TEXT_BUFFER(buffer), 1));
      //gtk_text_view_scroll_mark_onscreen(view, gtk_text_buffer_get_insert(buf));

      // Add all tags back to active tags
      if (tmp != NULL) {
        note_buffer_set_active_tags(buffer, tmp);
      }

      // Turn on the list-item tag from here on
      note_buffer_add_active_tag_by_name(buffer, "list-item");

      // */

      //TODO : update UI.
      return TRUE;
    }
  }
  // If line does not need bullet, let gtk handle the job...
  return FALSE;
}

gboolean
note_buffer_backspace_handler(BijiNoteBuffer *buffer)
{
  GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
  GtkTextIter start;
  GtkTextIter end;
  gboolean selection = gtk_text_buffer_get_selection_bounds(buf, &start, &end);
  GtkTextTag *depth = note_buffer_find_depth_tag(&start);

  if (selection) 
  {
    augment_selection(buffer, &start, &end);
    gtk_text_buffer_delete(buf, &start, &end);
    return TRUE;
  }

  /* No selection */
  else 
  {
    GtkTextIter prev = start;
    if (gtk_text_iter_get_line_offset(&prev) != 0) 
    {
      gtk_text_iter_backward_char(&prev);
    }
      
    GtkTextTag *prev_depth = note_buffer_find_depth_tag(&prev);
    if (depth != NULL || prev_depth != NULL) 
    {
      int lineNr = gtk_text_iter_get_line(&start);
      note_buffer_decrease_indent(buffer, lineNr, lineNr);
      return TRUE;
    } 
    else 
    {
	    /* Check if cursor is on the right side of a soft 
	     line break. If yes, remove it
	    Below code is probably wrong */
	    prev = start;
	    gtk_text_iter_backward_chars(&prev, 3);
	    if ( is_bullet(gtk_text_iter_get_char(&prev)) )
	    {
	      GtkTextIter end_break = prev;
		    gtk_text_iter_forward_char(&end_break);
		    gtk_text_buffer_delete(buf, &prev, &end_break);
	    }
	  }
  }
    
  return FALSE;
}

gboolean
note_buffer_add_tab(BijiNoteBuffer *buffer)
{
  GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
  GtkTextMark *insert_mark = gtk_text_buffer_get_insert(buf);
  GtkTextIter iter;

  gtk_text_buffer_get_iter_at_mark(buf,&iter,insert_mark);
  gtk_text_iter_set_line_offset(&iter,0);

  GtkTextTag *depth = note_buffer_find_depth_tag(&iter);
  if ( depth ) 
  {
    augment_depth(buffer);
    return TRUE;
  }
    
  return FALSE;
}

gboolean
note_buffer_remove_tab(BijiNoteBuffer *buffer)
{
  GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
  GtkTextMark *insert_mark = gtk_text_buffer_get_insert(buf);
  GtkTextIter iter;
		
  gtk_text_buffer_get_iter_at_mark(buf,&iter,insert_mark);
  gtk_text_iter_set_line_offset(&iter,0);
		
  GtkTextTag *depth = note_buffer_find_depth_tag(&iter);

	// If the cursor is at a line with depth and a tab has been
  // inserted, then we decrease the depth of that line.
  if (depth) 
  {
    decrease_depth(buffer);
    return TRUE;
  }

	// seems this case does not work in GtkTextView for classic indent.
	// we have to manually decrease indentation
	// gint line = gtk_text_iter_get_line(&iter);
  GtkTextIter end;
  gboolean selection = gtk_text_buffer_get_selection_bounds(buf, &iter, &end);
  if (!selection) // If we have a selection, do nothing.
  {
    gtk_text_buffer_get_iter_at_mark(buf,&iter,insert_mark);
	//TODO . the issue is to check
	// maybe, a. we are at the beginning of the line?
	// sure, b. previous char are a tabulation
  }
    
  return FALSE; // untill above is written, return FALSE but that useless.
}

void
note_buffer_check_selection(BijiNoteBuffer *buffer,gpointer biji_note_editor)
{
  GtkTextIter start, end;
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer); 

	if (gtk_text_buffer_get_selection_bounds(buf, &start, &end)) 
  {
    augment_selection(buffer, &start, &end);
    if ( g_utf8_strlen(gtk_text_buffer_get_text(buf,&start,&end,FALSE),-1) > 1 )
    {
      _biji_note_editor_set_text_is_selected(biji_note_editor,TRUE);
    }
    else
    {
      _biji_note_editor_set_text_is_selected(biji_note_editor,FALSE);
    }
	} 
  else 
  {
    _biji_note_editor_set_text_is_selected(biji_note_editor,FALSE);
		if (gtk_text_iter_get_line_offset(&start) < 2 && note_buffer_find_depth_tag(&start) != NULL) 
    {
			gtk_text_iter_set_line_offset(&start, 2);
			gtk_text_buffer_select_range(buf, &start, &start); // Needed?
		}
	}
}

void gtk_note_buffer_debug_xml(GtkTextBuffer *buffer)
{
  note_buffer_debug_xml(BIJI_NOTE_BUFFER(buffer));
}

/* Note Editor */
GtkTextView *
biji_text_view_new_from_note(BijiNoteObj *note)
{
  g_return_val_if_fail(BIJI_IS_NOTE_OBJ(note),NULL);
  return biji_gtk_view_new((gpointer)note);
}

gboolean 
biji_note_obj_is_opened(BijiNoteObj *note)
{
  return _biji_note_obj_is_opened(note) ;
}

void
biji_note_close(BijiNoteObj *note)
{
  _biji_note_obj_close_note(note);
}

gchar *
gtk_text_view_get_selection(GtkTextView *editor)
{
  GtkTextIter start;
  GtkTextIter end;

  GtkTextBuffer *buffer = gtk_text_view_get_buffer(editor);
  g_return_val_if_fail(GTK_IS_TEXT_BUFFER(buffer),NULL);

  if (gtk_text_buffer_get_selection_bounds(buffer, &start, &end))
    return gtk_text_buffer_get_text(buffer,&start,&end,FALSE); 

  return NULL ;
}


// SOIT ON TROUVE LE TAG PAR SON NOM
// SOIT ON RETRIEVE TOUS LES TAG, CHERCHE LES NOMS ET COMPARE...
gboolean
gtk_text_view_selection_has_tag(GtkTextView *editor, gchar *name)
{
  GtkTextIter start,end;
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(editor);
  GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer) ,
                                              name) ;
    
  if ( gtk_text_buffer_get_selection_bounds(buffer,&start,&end) )
  {
    if ( gtk_text_iter_has_tag(&start,tag) || gtk_text_iter_has_tag(&start,tag))
    {
      return TRUE ;
    }
    return FALSE ;
  }
  return FALSE ;
}

void
biji_toggle_bold_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"bold");
}

void
biji_toggle_italic_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"italic");
}

void
biji_toggle_underline_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"underline");
}

void
biji_toggle_strike_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"strikethrough");
}

void
biji_toggle_monospace_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"monospace");
}

void
biji_toggle_highlight_tag(GtkTextView *editor)
{
  buffer_selection_toggle_tag_by_name (gtk_text_view_get_buffer(editor),"highlight");
}

void biji_toggle_tag_by_name(GtkTextView *editor,gchar *name)
{
  buffer_selection_toggle_tag_by_name(gtk_text_view_get_buffer(editor),
                                      name);
}

void 
biji_remove_all_format(GtkTextView *editor)
{
  note_buffer_remove_tags(gtk_text_view_get_buffer(editor));
}

gboolean
biji_augment_font_size(GtkTextView *editor)
{
  return augment_font_size(gtk_text_view_get_buffer(editor));
}

gboolean 
biji_decrease_font_size(GtkTextView *editor)
{
  return decrease_font_size(gtk_text_view_get_buffer(editor));
}

void biji_augment_depth(GtkTextView *editor)
{
  GtkTextBuffer *buf = gtk_text_view_get_buffer(editor);
  augment_depth(BIJI_NOTE_BUFFER(buf));
}

void biji_decrease_depth(GtkTextView *editor)
{
  GtkTextBuffer *buf = gtk_text_view_get_buffer(editor);
  decrease_depth(BIJI_NOTE_BUFFER(buf));
}
