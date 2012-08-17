#ifndef __ZEITGEIST_H
#define __ZEITGEIST_H

#include <libbiji/libbiji.h>
#include "bjb-zeitgeist-dataprovider.h"

static ZeitgeistLog	*global_log;

static ZeitgeistLog *
get_log(void)
{
  if ( global_log == NULL )
  {
    global_log = g_object_new (ZEITGEIST_TYPE_LOG, NULL);
    /*
    dsReg = new DataSourceClient();
	handler = new ZeitgeistHandler();    
    
    dsReg.RegisterDataSources(tomboyDataSourceId, 
			                          	tomboyDataSourceName, 
			                          	tomboyDataSourceDesc , 
			                          	new List<Event>(){ev});
    */
  }

  return global_log ;
}

void 
insert_zeitgeist(BijiNoteObj *note,const char *action)
{
  g_message("insert zeitgeist");
  ZeitgeistEvent	 *event;
  ZeitgeistSubject   *subject ;
  ZeitgeistLog       *log = get_log() ;
    
  subject = zeitgeist_subject_new_full ( note_obj_get_path(note),
                                         ZEITGEIST_NFO_DOCUMENT,
			                             ZEITGEIST_NFO_FILE_DATA_OBJECT,
			                             "application/x-note",
			                             note_obj_get_path(note),
			                             biji_note_get_title(note), 
			                             NULL) ;

  event = zeitgeist_event_new_full (action,
                                    ZEITGEIST_ZG_USER_ACTIVITY,
                                    "application://bijiben.desktop", //actor will open
                                    subject,
                                    NULL );

  zeitgeist_log_insert_events_no_reply(log, event, NULL);
}

#endif /* __ZEITGEIST_H */