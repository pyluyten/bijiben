#include "biji-note-buffer.h"
#include "biji-note-obj.h"
#include "biji-note-tag.h"
#include "biji-serialize.h"
#include "libbiji.h"

/* XML Serializing. Almost as in conboy, thus near to tomboy */

static void
write_start_tag(GtkTextTag *t, Serializer *s)
{
  gchar** strings;
  gchar *tag_name;

  tag_name = biji_note_tag_get_name(t);
		
  // ignore tags starting with "_"
  if ( g_str_has_prefix  (tag_name,"_"))
  {
    return;
  }
		
	// Ignore <list> tags.
	if ( g_strcmp0 (tag_name,"list")==0)
	{
		s->is_list_active = TRUE;
		return;
	}

  // ignore depth
	if ( g_str_has_prefix  (tag_name,"depth")) 
	{
		s->is_bullet = TRUE;
		strings = g_strsplit(tag_name, ":", 2);
		s->new_depth = atoi(strings[1]);
		g_strfreev(strings);
		return;
	}
		
	// If a <list-item> tag
	if ( g_str_has_prefix  (tag_name,"list-item")) 
	{
		// <list-item> , case new depth
		if ( s->new_depth < s->depth )
		{
			gint diff = s->depth - s->new_depth;
			xmlTextWriterEndElement(s->writer); // <list-item>
			s->xml_elements_debug --;

			// <list-item> , case new depth
			// For each depth we need to close a <list-item> and a <list> tag.	

			while (diff > 0) 
			{ 
			  xmlTextWriterEndElement(s->writer); // </list>
				s->xml_elements_debug --;
			  xmlTextWriterEndElement(s->writer); // </list-item>
				s->xml_elements_debug --;
			  diff--;
		  }
				
		xmlTextWriterStartElement(s->writer, BAD_CAST "list-item");
		xmlTextWriterWriteAttribute(s->writer, BAD_CAST "dir", BAD_CAST "ltr");
		s->xml_elements_debug ++;
		}

		// <list-item> , case new depth
		if (s->new_depth > s->depth ) 
		{
			gint diff = s->new_depth - s->depth;

			while (diff > 0) 
			{
			xmlTextWriterStartElement(s->writer, BAD_CAST "list");
			s->xml_elements_debug ++;
			xmlTextWriterStartElement(s->writer, BAD_CAST "list-item");
			s->xml_elements_debug ++;
			xmlTextWriterWriteAttribute(s->writer, BAD_CAST "dir", BAD_CAST "ltr");
			diff--;
		  }
		}

		// <list-item> case same depth
		if ( s->new_depth == s->depth) 
		{
		  xmlTextWriterEndElement(s->writer); /* </list-item> */
		  xmlTextWriterStartElement(s->writer, BAD_CAST "list-item");
		  xmlTextWriterWriteAttribute(s->writer, BAD_CAST "dir", BAD_CAST "ltr");
	  }

	  s->depth = s->new_depth;	
	}

	// not a list-item, simply write it
	else
	{
		xmlTextWriterStartElement(s->writer, BAD_CAST tag_name);
		s->xml_elements_debug ++;
		return;
	}
		
	return;		
}

static void
write_end_tag(GtkTextTag *t, Serializer *s)
{
  gchar *tag_name = biji_note_tag_get_name(t);

  // ignore tags with _ as prefix
  if ( g_str_has_prefix  (tag_name,"_"))
  {
    return;
  }

  // ignore depth
  if ( g_str_has_prefix  (tag_name,"depth")) 
  {
    s->is_bullet = FALSE;
    return;
  }

	// List
	if ( g_strcmp0 (tag_name,"list")==0)
  {
		while (s->depth > 0) 
		{
			xmlTextWriterEndElement(s->writer); // </list-item>
			xmlTextWriterEndElement(s->writer); // </list>
			s->xml_elements_debug --;
			s->xml_elements_debug --;
			s->depth--;
		}
		return;
	}

	if (  g_strcmp0 (tag_name,"list-item")==0)
	{
		return;
	}

  // Not a depth, not a list => simply write
	xmlTextWriterEndElement(s->writer);
	s->xml_elements_debug --;
	return;
}

static 
gint sort_tags(GtkTextTag *tag1, GtkTextTag *tag2)
{
  gint prio1, prio2 ;

	prio1 = gtk_text_tag_get_priority(tag1) ;
	prio2 = gtk_text_tag_get_priority(tag2) ;
		
	if (prio1 == prio2)
	{
		g_printerr("ERROR: Two tags cannot have the same priority.\n");
		return 0;
	}

	return (prio2 - prio1);
}

static void
serialize_tags(gpointer *tag, gpointer xml_text_writer)
{
  xmlTextWriterPtr writer = (xmlTextWriterPtr) xml_text_writer ;

  // TODO if note is template it's different...
  xmlTextWriterStartElement(writer, BAD_CAST "tag");
  gchar *to_serialize = g_strdup_printf("system:notebook:%s",(gchar*)tag);
  xmlTextWriterWriteString(writer, BAD_CAST to_serialize);
  xmlTextWriterEndElement(writer);
  //g_free(to_serialize) ; cannot free here. Could push to a GSList to free
    // AFTER the file is written...
}

static xmlBufferPtr
note_obj_serialize_content(gpointer note)
{    
  GtkTextIter iter, old_iter, start,end;
  GSList *start_tags, *end_tags;
  gint debug;
  gchar *text;
  BijiNoteBuffer *note_buffer ;
  GtkTextBuffer *gtk_buf ;

  xmlBufferPtr buf = xmlBufferCreate();
  Serializer serializer ;
  serializer.is_bullet = FALSE; 
  serializer.writer = xmlNewTextWriterMemory(buf, 0);
  serializer.depth = 0 ;
  serializer.new_depth = 0;
  serializer.xml_elements_debug =0;

  // <note-content>
  xmlTextWriterStartElement(serializer.writer, BAD_CAST "note-content");
  xmlTextWriterWriteAttributeNS(serializer.writer, NULL, 
                                BAD_CAST "version",NULL, 
	                            BAD_CAST "0.1");
  xmlTextWriterWriteAttributeNS(serializer.writer, BAD_CAST "xmlns",
                                BAD_CAST "link", NULL, 
                                BAD_CAST "http://beatniksoftware.com/tomboy/link");
  xmlTextWriterWriteAttributeNS(serializer.writer, BAD_CAST "xmlns", BAD_CAST "size", NULL,
                                BAD_CAST "http://beatniksoftware.com/tomboy/size");
  xmlTextWriterWriteAttributeNS(serializer.writer, NULL, BAD_CAST "xmlns", NULL, 
                                BAD_CAST "http://beatniksoftware.com/tomboy");
    
  // begin work
    
  start_tags = NULL;
  end_tags = NULL ;
  note_buffer = BIJI_NOTE_BUFFER(biji_note_get_or_create_buffer(note));
  gtk_buf = GTK_TEXT_BUFFER(note_buffer);
  gtk_text_buffer_get_bounds(gtk_buf, &start, &end);
  iter = start;
  old_iter = iter;
	                              
  while (gtk_text_iter_compare(&iter, &end) < 0)
  {
	debug ++ ;
		
	start_tags = gtk_text_iter_get_toggled_tags(&iter, TRUE); // toggled on
    end_tags   = gtk_text_iter_get_toggled_tags(&iter, FALSE); // not toggled on

	// write tags
	g_slist_foreach(end_tags, (GFunc)write_end_tag, &serializer);
	start_tags = g_slist_sort(start_tags, (GCompareFunc)sort_tags);
	g_slist_foreach(start_tags, (GFunc)write_start_tag, &serializer);

	// write text
	old_iter = iter;
	if (gtk_text_iter_compare(&iter, &end) >= 0) 
    {
	  break;
	}
      
	gtk_text_iter_forward_to_tag_toggle(&iter, NULL);
	text = gtk_text_iter_get_text(&old_iter, &iter);

	if ( !serializer.is_bullet )
	{
	  xmlTextWriterWriteString(serializer.writer, BAD_CAST text);
	}
  }

  // if the text ending the buffer has tag, we'll miss some end tags.
  while ( serializer.xml_elements_debug > 0 )
  {
    xmlTextWriterEndElement(serializer.writer);
    serializer.xml_elements_debug --;
  }

  // </note-content>
  xmlTextWriterEndElement(serializer.writer);

  // we pass the note content to stock it FIXME
  xmlFreeTextWriter(serializer.writer);
  return buf ;
}

xmlBufferPtr
note_obj_serialize(gpointer note, gint test)
{
  BijiNoteObj *note_obj = BIJI_NOTE_OBJ(note);
  xmlBufferPtr buf = xmlBufferCreate();
  xmlTextWriterPtr writer = xmlNewTextWriterMemory(buf, 0);
	
  // "<?xml version="1.0" encoding="utf-8"?">
  xmlTextWriterStartDocument (writer,"1.0","utf-8",NULL); 

  // < note >
  xmlTextWriterStartElement(writer, BAD_CAST "note");
  xmlTextWriterWriteAttributeNS(writer, NULL, 
                                BAD_CAST "version",NULL, 
	                            BAD_CAST "0.3");
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns",
                                BAD_CAST "link", NULL, 
                                BAD_CAST "http://beatniksoftware.com/tomboy/link");
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "size", NULL,
                                BAD_CAST "http://beatniksoftware.com/tomboy/size");
  xmlTextWriterWriteAttributeNS(writer, NULL, BAD_CAST "xmlns", NULL, 
                                BAD_CAST "http://beatniksoftware.com/tomboy");

  // <title>title</title>
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "title");
  xmlTextWriterWriteString(writer,
                          BAD_CAST biji_note_get_title (note_obj) );
  xmlTextWriterEndElement(writer);

  // <text>
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "text");
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xml",
                                BAD_CAST "space", NULL, 
                                BAD_CAST "preserve");


  // this func serializes the note content 
  // but we still have to WRITE it... erg...
  xmlBufferPtr content_buf = note_obj_serialize_content(note) ; 
  gchar *note_content = (gchar*) content_buf->content ;
  xmlTextWriterWriteRaw(writer, BAD_CAST (note_content) ) ;
  biji_note_set_content(note_obj,note_content);
  /*xmlBufferFree(content_buf);*/



  // </text> 
  xmlTextWriterEndElement(writer);

  BijiNoteID *id = note_get_id(note_obj);
                               
  // <last-change-date>
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "last-change-date");
  xmlTextWriterWriteString(writer,
                           BAD_CAST biji_note_id_get_last_change_date(id));  
  xmlTextWriterEndElement(writer);                    

  //<last-metadata-change-date>
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "last-metadata-change-date");
  xmlTextWriterWriteString(writer,
                           BAD_CAST biji_note_id_get_last_metadata_change_date(id));
  xmlTextWriterEndElement(writer);
                                
  //<create-date>
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "create-date");
  xmlTextWriterWriteString(writer,
                           BAD_CAST biji_note_id_get_create_date(id));
  xmlTextWriterEndElement(writer);
                                
  //<cursor-position>
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "cursor-position");
  xmlTextWriterWriteString(writer,BAD_CAST "0");
  xmlTextWriterEndElement(writer);

  //<selection-bound-position>
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "selection-bound-position");
  xmlTextWriterWriteString(writer,BAD_CAST "0");
  xmlTextWriterEndElement(writer);

  //<width> is always 0 on this soft.
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "width");
  xmlTextWriterWriteString(writer,BAD_CAST "0");
  xmlTextWriterEndElement(writer);
                                
  //<height> is always 0
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "height");
  xmlTextWriterWriteString(writer,BAD_CAST "0");
  xmlTextWriterEndElement(writer);
                                
  //<x> is always 0
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "x");
  xmlTextWriterWriteString(writer,BAD_CAST "0");
  xmlTextWriterEndElement(writer);
                                
  //<y> is always 0
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "y");
  xmlTextWriterWriteString(writer,BAD_CAST "0");
  xmlTextWriterEndElement(writer);

  //<color>
  if ( note_obj_get_rgba(note_obj) )
  {
    xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
    xmlTextWriterStartElement(writer, BAD_CAST "color");
    xmlTextWriterWriteString(writer,BAD_CAST (note_obj_get_rgba(note_obj)));
    xmlTextWriterEndElement(writer);
  }

  //<tags>
  xmlTextWriterStartElement(writer, BAD_CAST "tags");
  GList *tags = (GList*) _biji_note_obj_get_tags(note_obj);
  g_list_foreach (tags,(GFunc)serialize_tags,(gpointer)writer);
  xmlTextWriterEndElement(writer);
  g_list_free(tags);
                                
  //
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n  ");
  xmlTextWriterStartElement(writer, BAD_CAST "open-on-startup");
  xmlTextWriterWriteString(writer,BAD_CAST "False");
  xmlTextWriterEndElement(writer);
              
  // <note>
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n ");
  xmlTextWriterEndElement(writer);
	                              
  xmlFreeTextWriter(writer);
  return buf ;
}

//
//
//
//
//
// These debug func just print "text<tag>formated text</tag>final text"
// for ALL tags.

static void
debug_start_tag(GtkTextTag *t, Serializer *s)
{
  gchar *tag_name = biji_note_tag_get_name(t);
  xmlTextWriterStartElement(s->writer, BAD_CAST tag_name);		
}

static void
debug_end_tag(GtkTextTag *t, Serializer *s)
{
  xmlTextWriterEndElement(s->writer);
  return;
}


void note_buffer_debug_xml(BijiNoteBuffer* note_buffer) 
{
  GtkTextBuffer *gtk_buf; 
  GtkTextIter iter, old_iter, start, end;
  xmlBufferPtr buf;
  gchar *text;
  Serializer serializer; // the pamareter structure for "gslist for each"
  xmlTextWriterPtr writer;
  GSList *start_tags, *end_tags;

  gtk_buf = GTK_TEXT_BUFFER(note_buffer);
  gtk_text_buffer_get_bounds(gtk_buf, &start, &end);
  iter = start;
  old_iter = iter;
  buf = xmlBufferCreate();
  writer = xmlNewTextWriterMemory(buf, 0);
  serializer.is_bullet = FALSE; 
  serializer.writer = writer ;
  serializer.depth = 0 ;
  serializer.new_depth = 0;
  start_tags = NULL;
  end_tags = NULL ;
	
  xmlTextWriterStartElement(writer, BAD_CAST "note-content");
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "link", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/link");
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "size", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/size");
  xmlTextWriterWriteAttributeNS(writer, NULL, BAD_CAST "xmlns", NULL, BAD_CAST "http://beatniksoftware.com/tomboy");

  //gint debug;
	                              
  while (gtk_text_iter_compare(&iter, &end) < 0)
  {
    // toggled on = TRUE
    start_tags = gtk_text_iter_get_toggled_tags(&iter, TRUE);

    // toggled on = FALSE
    end_tags   = gtk_text_iter_get_toggled_tags(&iter, FALSE);

    // write tags 
    g_slist_foreach(end_tags, (GFunc)debug_end_tag, &serializer);
    start_tags = g_slist_sort(start_tags, (GCompareFunc)sort_tags);
    g_slist_foreach(start_tags, (GFunc)debug_start_tag, &serializer);

    // write text 
    old_iter = iter;
    if (gtk_text_iter_compare(&iter, &end) >= 0) {
      break;
    }
      
    gtk_text_iter_forward_to_tag_toggle(&iter, NULL);
    text = gtk_text_iter_get_text(&old_iter, &iter);

    // pour les besoins du debuggage on écrit même les bullet !!
  	xmlTextWriterWriteString(serializer.writer, BAD_CAST text);
	}
	                              //get_to

  xmlTextWriterEndElement(writer);
  xmlTextWriterWriteRaw(writer, BAD_CAST "\n ");
  xmlFreeTextWriter(writer);

  // this func just prints the result.that's all!                           
  g_message("result\n %s",(gchar*)buf->content);	
}