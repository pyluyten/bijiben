#include <glib/gprintf.h>
#include <libbiji/libbiji.h>

#include "utils/bjb-icons-colors.h"
#include "widgets/bjb-menu-tool.h"
#include "widgets/gd-main-toolbar.h"


#include "bjb-app-menu.h"
#include "bjb-bijiben.h"
#include "bjb-tracker.h"
#include "bjb-main-view.h"
#include "bjb-note-view.h"
#include "bjb-tags-view.h"
#include "bjb-window-base.h"
#include "bjb-selection-panel.h"
#include "bjb-rename-note.h"

/************************** Gobject ***************************/

typedef enum {
  ORDER_BY_LAST ,
  ORDER_BY_TITLE
} NoteViewSort ;

// wheter users is selecting notes
typedef enum {
  BJB_SELECTION_NONE,
  BJB_SELECTION_MULTIPLE
} ViewMode ;


struct _BjbMainViewPriv {
  GtkWidget      *window;
  GtkWidget      *icon_view ;
  GtkWidget      *vbox;
  GtkWidget      *standard_toolbar; 

  /* Notes */
  GtkTreeStore   *store ;
  GList          *notes_to_show ;
  NoteViewSort    sort_type ;

  /* Selection */
  ViewMode        mode ;
  GtkWidget      *select_toolbar;
  GtkWidget      *label;
  GtkToolItem    *finish_selection ;
  GtkWidget      *actions ;  // actions bar for selected notes.

  /* Search Entry */
  GtkWidget      *vbox_up ; // FIXME remove this one. 
  GtkWidget      *hbox_entry ;
  GtkWidget      *search_entry ;
  gboolean        has_entry ;

  /* signals */
  gulong notes_changed ;               // I don't remember this one.
  gulong click_signal ;          // Icon view selection changed
  gulong key_pressed ;           // a key pressed shows the entry.
};

G_DEFINE_TYPE (BjbMainView, bjb_main_view, GTK_TYPE_BOX);

static void
bjb_main_view_init (BjbMainView *object)
{
  object->priv = 
  G_TYPE_INSTANCE_GET_PRIVATE(object,BJB_TYPE_MAIN_VIEW,BjbMainViewPriv);

  object->priv->window = NULL;
  object->priv->search_entry = NULL;
  object->priv->sort_type = ORDER_BY_LAST ;
  object->priv->standard_toolbar = NULL ;
  object->priv->select_toolbar = NULL ;
  object->priv->mode = BJB_SELECTION_NONE ;
}

static void
bjb_main_view_finalize (GObject *object)
{
  BjbMainView *view = BJB_MAIN_VIEW(object) ;
  
  if ( view->priv->notes_to_show != NULL )
  {
    g_list_free(view->priv->notes_to_show) ;
  }

  g_signal_handler_disconnect(bijiben_window_get_book(view->priv->window),
                              view->priv->notes_changed);

  G_OBJECT_CLASS (bjb_main_view_parent_class)->finalize (object);
}

static void
bjb_main_view_set_property (GObject      *object,
			                guint         prop_id,
			                const GValue *value,
			                GParamSpec   *pspec)
{
}


static GObject *
biji_main_view_constructor (GType                  gtype,
                            guint                  n_properties,
                            GObjectConstructParam  *properties)
{
  GObject *obj;
  {
    obj = G_OBJECT_CLASS (bjb_main_view_parent_class)->constructor (gtype, n_properties, properties);
  }
  return obj;
}

static void
bjb_main_view_class_init (BjbMainViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = bjb_main_view_finalize;
  object_class->set_property = bjb_main_view_set_property;
  object_class->constructor = biji_main_view_constructor;
    
  g_type_class_add_private (klass, sizeof (BjbMainViewPriv));
}



BijiNoteBook *
bjb_main_view_get_book(BjbMainView *view)
{
  return bijiben_window_get_book(view->priv->window);
}

/*********************** Notes View (icon grid)********************/
enum
{
  COL_TITLE,
  COL_PIXBUF,
  COL_SELECT,
  COL_PATH,
  NUM_COLS
};

static void
append_note_to_iter ( BijiNoteObj *n, BjbMainView *view)
{
  GtkTreeIter iter;
  GtkTreeStore *store = view->priv->store ;
  GdkPixbuf *pixbuf = main_window_get_note_pixbuf(view->priv->window);
    
  if ( biji_note_obj_is_template(n) == FALSE )
	{
      gtk_tree_store_append(store,&iter,NULL);
      gtk_tree_store_set(store,&iter,
                         COL_TITLE,biji_note_get_title(n),
                         COL_PIXBUF,pixbuf,
                         COL_SELECT,pixbuf,
                         COL_PATH,note_obj_get_path(n),
                         -1);
	}
}

static void
main_view_set_notes_to_show(BjbMainView *view, GList *notes)
{
  if ( view->priv->notes_to_show != NULL )
  {
    g_list_free(view->priv->notes_to_show);
  }

  view->priv->notes_to_show = notes ;
}

glong
most_recent_note_first ( BijiNoteObj *a, BijiNoteObj *b)
{
  glong result = biji_note_id_get_last_change_date_sec(note_get_id(b));
  return result - biji_note_id_get_last_change_date_sec(note_get_id(a));
}

static void
sort_notes (BjbMainView *view)
{
  if ( view->priv->sort_type == ORDER_BY_LAST )
  {
    view->priv->notes_to_show = g_list_sort(view->priv->notes_to_show,
                                            (GCompareFunc)most_recent_note_first);
  }
  else 
  {
    g_message("MAIN VIEW ! Critcial ! cannot sort notes with %i sort type",
              view->priv->sort_type );
  }                                          
}

static void
update_notes (BjbMainView *view)
{
  gtk_tree_store_clear (view->priv->store);
  sort_notes(view);
  g_list_foreach (view->priv->notes_to_show,(GFunc)append_note_to_iter,view);
}

static void
on_selection_mode_changed ( GtkWidget *button, BjbMainView *view)
{
  /* Go to SELECTION MODE. Allow Clicking notes to select.
   * Deactivate opening notes. */
	
  if ( view->priv->mode == BJB_SELECTION_NONE )
  {
    view->priv->mode = BJB_SELECTION_MULTIPLE ;
    BIJI_DEBUG("switch to selection mode")

    gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (view->priv->icon_view),
                                      GTK_SELECTION_MULTIPLE);

    gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW(view->priv->icon_view),
	                                 COL_SELECT);

    GtkWidget * actions = get_selection_panel(view);
    view->priv->actions = actions;
    gtk_widget_show_all(actions);


    hide_search_entry(view);
      
    gtk_box_pack_start(GTK_BOX(view->priv->vbox),actions,FALSE,FALSE,0);
    gtk_widget_hide(view->priv->standard_toolbar);
    gtk_widget_show_all(view->priv->select_toolbar);
	
  }

  /* Return to classic mode. No selection.  */
	
  else
  { 
    g_message("switch back to normal mode");
    view->priv->mode = BJB_SELECTION_NONE ;
	  
    gtk_icon_view_unselect_all(GTK_ICON_VIEW(view->priv->icon_view));

	  gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (view->priv->icon_view),
                              GTK_SELECTION_SINGLE);  //      GTK_SELECTION_NONE);

    gtk_widget_destroy (view->priv->actions);

    gtk_widget_show_all(view->priv->standard_toolbar);
    gtk_widget_hide(view->priv->select_toolbar);
	
  }

}

// Callbacks

void
switch_to_note_view(BjbMainView *view,BijiNoteObj *note)
{	
  /* Sanitize */
  g_signal_handler_disconnect(view->priv->window,
                              view->priv->key_pressed) ;
    
  GtkWidget *win = view->priv->window;
  main_window_set_frame((gpointer)win,
                             GTK_CONTAINER(bjb_note_view_new(win,note,TRUE)));
  
  gtk_window_set_title(GTK_WINDOW(win),biji_note_get_title (note));
}



static void
clear_search_entry_callback(GtkEntry *entry,GtkEntryIconPosition icon_pos,
                            GdkEvent *event,BjbMainView *view)
{
  gtk_entry_set_text(entry,"");
  BijiNoteBook *book = bijiben_window_get_book(view->priv->window);
  main_view_set_notes_to_show(view,biji_note_book_get_notes(book));
  update_notes(view);
}

void
update_notes_with_tag_search(BjbMainView *view, gchar *needle)
{
  GList *notes_to_show = NULL ;
  gchar *tag  = g_strdup(needle);        // tag=toto

  tag = g_utf8_strreverse(tag,-1);       // otot=gat
  glong size = g_utf8_strlen (tag,-1) ;
  g_utf8_strncpy(tag,tag,size-4);        // otot
  tag = g_utf8_strreverse(tag,-1);       // toto

  BijiNoteBook *book = bijiben_window_get_book(view->priv->window);
  notes_to_show = biji_note_book_get_notes_with_tag_prefix(book,tag);
  g_free(tag);
    
  // 2. update the view
  main_view_set_notes_to_show(view,notes_to_show);
  update_notes(view);
}

void
update_notes_with_string_search(BjbMainView *view, gchar *needle)
{
  gint i ;
  BijiNoteBook *book = bijiben_window_get_book(view->priv->window);
  GList *cur_notes = biji_note_book_get_notes(book);
  GList *notes_to_show = NULL ;

  for ( i=0 ; i < g_list_length (cur_notes) ; i++ )
  {
    BijiNoteObj *cur = g_list_nth_data (cur_notes,i) ;
    if (g_strrstr(
                  g_utf8_casefold(biji_note_get_raw_text(cur),-1),
                  g_utf8_casefold(needle,-1)))
    {
      notes_to_show = g_list_append(notes_to_show,cur);
    }
  }

  // update the view
  main_view_set_notes_to_show(view,notes_to_show);
  update_notes(view);
}

static void
show_all_notes(BjbMainView *view)
{
  BijiNoteBook *book = bijiben_window_get_book(view->priv->window);
  main_view_set_notes_to_show(view,biji_note_book_get_notes(book));
  update_notes(view);                                                       
}

/* Fix this to look for both notes and tags */
static void
update_notes_switch(BjbMainView *view, gchar *needle)
{
  // No needle.
  if ( needle == NULL )
  {
    show_all_notes(view);
    return ;
  }

  // No-tag Notes
  if ( g_strcmp0 (needle,"!tag") == 0 )
  {
     BijiNoteBook *book = bjb_main_view_get_book(view);
     GList *notes_to_show = biji_note_book_get_no_tag_notes(book);
     main_view_set_notes_to_show(view,notes_to_show);
     update_notes(view);
     return ;
  }

  // Search notes with a tag
  if ( g_str_has_prefix (needle,"tag=") == TRUE )
  {
    if ( g_utf8_strlen(needle, -1) > 5 )
    {
      update_notes_with_tag_search(view,needle);
    }
    return ;
  }
		
  // Only search with 4+ char.
  if (g_utf8_strlen(needle, -1) > 3 )
  {
    update_notes_with_string_search(view,needle);
  	return ;
  }
  else
  {
    show_all_notes(view);
    return ;
  }
}

static void
action_search_entry(GtkEntry *entry,BjbMainView *view)
{
  gchar *search = g_strdup(gtk_entry_get_text(entry));
  update_notes_switch(view,search);
  //g_free(search);
}

static void
action_entry_insert_callback(GtkEntryBuffer *buffer,guint position,
                             gchar *chars,guint n_chars,BjbMainView *view)
{
  action_search_entry(GTK_ENTRY(view->priv->search_entry),view);
}

static void
action_entry_delete_callback(GtkEntryBuffer *buffer,guint position,
                             guint n_chars,BjbMainView *view)
{ 
  action_search_entry(GTK_ENTRY(view->priv->search_entry),view);
}


void
action_new_note_callback(GtkMenuItem *item,BjbMainView *view)
{
  gchar *title ;
  BijiNoteObj *result ;
  BijiNoteBook *book ;
    
  title = note_title_dialog(GTK_WINDOW(view->priv->window),
                            "New Note : Title",
                            "New Note");
   
  if ( !title)
  {
    g_message("No title set");
    return ;
  }

  /* append note to collection
   * FIXME this is suppose to sanitize title. */ 
  book = bijiben_window_get_book(view->priv->window);
  gchar *folder = g_strdup_printf("%s/bijiben",g_get_user_data_dir());
  result = biji_note_get_new_from_string(title,folder);
  g_free(folder);
  note_book_append_new_note(book,result);

  /* Go to that note */
  switch_to_note_view(view,result);
  note_obj_save_note_using_buffer((gpointer)result);
}

static gchar *
get_selected_note_path(BjbMainView *view)
{
  // Get the tag.
  GtkTreeIter iter;
  GtkTreeModel *model ;
  GtkTreePath *path = NULL  ;
  GtkIconView *tree ;
  GtkCellRenderer *column = NULL ;
  gchar *note_path ;

  tree = (GtkIconView*)view->priv->icon_view;
  model = gtk_icon_view_get_model(tree);
  gtk_icon_view_get_cursor(tree,&path,&column);

  if ( path == NULL )
  {
    return NULL ;
  }
    
  gtk_tree_model_get_iter (model,&iter, path);
  gtk_tree_model_get (model, &iter,COL_PATH, &note_path,-1);
  gtk_tree_path_free(path);

  return note_path;
}

static BijiNoteObj *
get_selected_note(BjbMainView *view)
{
  return note_book_get_note_at_path(bijiben_window_get_book(view->priv->window),
                                    get_selected_note_path(view));
}

void
action_new_window_callback(GtkAction *action, gpointer bjb_main_view)
{
  BjbMainView *view = BJB_MAIN_VIEW(bjb_main_view);
  BijiNoteObj *note = get_selected_note(view) ;

  if ( note == NULL )
    return ;
    
  GtkWidget *window = bjb_main_view_get_window(view) ; 
  gpointer app = main_window_get_app(window);
  create_new_window_for_note(app , note) ;
}


static GtkTreeStore *
notes_grid (void)
{
  GtkTreeStore *store;
  store = gtk_tree_store_new (NUM_COLS,
                              G_TYPE_STRING,
                              GDK_TYPE_PIXBUF,
                              GDK_TYPE_PIXBUF,
                              G_TYPE_STRING);
  return store;
}

static void
show_window_if_title_same(GtkWindow *window, BijiNoteObj *to_open)
{
  if ( g_strcmp0 (gtk_window_get_title (window),
                  biji_note_get_title(to_open)) == 0 )
  {
    gtk_window_present(window);
  }
}

static void
switch_to_note(BjbMainView *view, BijiNoteObj *to_open)
{
  // If the note is already opened in another window, just show it.
  if ( biji_note_obj_is_opened(to_open) )
  {
    GList *notes ;

    notes = gtk_application_get_windows 
                    (GTK_APPLICATION
                              (main_window_get_app(view->priv->window)));
    g_list_foreach (notes,(GFunc)show_window_if_title_same,to_open);
    return ;
  }

  // Otherwise, leave main view to show this note into current window.
  switch_to_note_view(view,to_open);
}

/* Not used directly  */
static void
action_switch_to_note_callback(GtkIconView *icon,
                               GtkTreePath *path,
                               BjbMainView *view)
{
  g_return_if_fail(GTK_IS_ICON_VIEW(icon));
  g_return_if_fail(BJB_IS_MAIN_VIEW(view));

  /* GET THE NOTE INDEX FIXME this func is written elsewhre...*/
  gchar *note_path;
  GtkTreeIter iter;
  GtkTreeModel *model = GTK_TREE_MODEL (view->priv->store);
  gtk_tree_model_get_iter (model,&iter, path);
  gtk_tree_model_get (model, &iter,COL_PATH, &note_path,-1);

  /* GET THAT NOTE AND JUMP TO IT. */ 
  BijiNoteBook *book = bijiben_window_get_book(view->priv->window); 
  BijiNoteObj *to_open = note_book_get_note_at_path(book,note_path) ;

  switch_to_note(view,to_open);
}


/* shows number of selected notes or so */
static void
update_selection_label(BjbMainView *bmv,gint notes)
{
  g_message("update label, %i",notes);

  if ( notes == 0 )
  {
    gtk_label_set_text(GTK_LABEL(bmv->priv->label),
                       "Click on Notes to Select");
  }

  else 
  {
    gchar *text  = g_strdup_printf("%i Notes Selected",notes);
    gtk_label_set_text(GTK_LABEL(bmv->priv->label),text);
  }

        
}

/* either correct this (because of scrollbar, x y does not indicate note 
 * or delete this one to keep below "on selection changed" */
static gboolean
on_button_press_event (GtkWidget *view,
                       GdkEventButton *event,
                       BjbMainView *bmv)
{
  GtkIconView *giv = GTK_ICON_VIEW(bmv->priv->icon_view);
    
  /* Get the note */
  /* gtk_icon_view_get_dest_item_at_pos does NOT work because scrollbar */
  GtkTreePath *note = NULL;
  note = gtk_icon_view_get_path_at_pos(giv,(gint) event->x, (gint) event->y ) ;
    
  if ( !note )
    return TRUE ; 
  
  /* NORMAL_MODE => Switch to this note */
  if ( bmv->priv->mode == BJB_SELECTION_NONE )
  {
    action_switch_to_note_callback(giv,note,bmv);
    return TRUE ;
  }

  /* SELECTION_MODE => Add or Remove note to selection */
  if ( gtk_icon_view_path_is_selected(giv,note) )
  {
    gtk_icon_view_unselect_path(giv,note);
  }
  else
  {
    gtk_icon_view_select_path(giv,note);
  }

  gint number = g_list_length(gtk_icon_view_get_selected_items (giv));
  update_selection_label(bmv,number);
    
  return TRUE ;
}

static GtkWidget *
create_icon_view(BjbMainView *self)
{
  GtkWidget * sw;
  gchar *needle = biji_win_get_entry(self->priv->window);
    
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  self->priv->store = notes_grid() ;

  // Show the proper notes according to the string search
  update_notes_switch(self, needle);
    
  self->priv->icon_view = gtk_icon_view_new_with_model(GTK_TREE_MODEL(self->priv->store));
  gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (self->priv->icon_view),
                                     GTK_SELECTION_SINGLE);
  g_object_unref (self->priv->store); // 

  gtk_icon_view_set_text_column (GTK_ICON_VIEW (self->priv->icon_view), 
                                 COL_TITLE);

  gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (self->priv->icon_view), 
                                   COL_PIXBUF);

  self->priv->click_signal = g_signal_connect(self->priv->icon_view,
                                              "button-press-event",
                                              G_CALLBACK(on_button_press_event),
                                              self); 

  /* self->priv->click_signal = g_signal_connect(self->priv->icon_view,
                                              "selection_changed",
                                              G_CALLBACK(on_selection_changed),
                                              self); */

  gtk_container_add (GTK_CONTAINER (sw), self->priv->icon_view);
  return sw;
}

/* Toolbar */

GtkTreeModel *
create_completion_model (void)
{
  GtkListStore *store;
  GtkTreeIter iter;

  store = gtk_list_store_new (1, G_TYPE_STRING);

  // Search Tag.
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, "tag=", -1);

  return GTK_TREE_MODEL (store);
}



static GtkWidget *
create_standard_toolbar(BjbMainView *parent)
{
  GtkWidget *ret ;
  GtkToolbar *tool;
  GtkToolItem *space_l, *space_r ;

  ret = gtk_toolbar_new();
  tool = GTK_TOOLBAR(ret);
  space_l = gtk_separator_tool_item_new(); 
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(space_l),FALSE);

  /* New Note button */
  GtkToolItem *new_note ;
  GtkWidget *label = gtk_button_new_with_label ("New");
  new_note = gtk_tool_button_new (label,NULL);
  g_signal_connect(new_note,"clicked",
                   G_CALLBACK(action_new_note_callback),parent);
  gtk_toolbar_insert (tool,new_note,-1); 

  /* Space, Label, space */
  gtk_tool_item_set_expand(space_l,TRUE);
  gtk_toolbar_insert(tool,space_l,-1);

  GtkWidget *la = gtk_label_new("New and Recent");
  GtkToolItem *iter = gtk_tool_button_new(la,NULL);
  gtk_toolbar_insert(tool,iter,-1);

  
  space_r = gtk_separator_tool_item_new(); 
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(space_r),FALSE);
  gtk_tool_item_set_expand(space_r,TRUE);
  gtk_toolbar_insert(tool,space_r,-1); 
	
  /* select button */
  GtkWidget *check = get_icon("emblem-default-symbolic") ;
  GtkToolItem *select ;
  select = gtk_tool_button_new(check,NULL);
  g_signal_connect(select,"clicked",
                   G_CALLBACK(on_selection_mode_changed),parent);
  gtk_toolbar_insert (tool,select,-1); 
	
  return ret ;	
}

static GtkWidget *
create_selection_toolbar(BjbMainView *parent)
{
  GtkWidget *ret ;
  GtkToolbar *tool;
  GtkToolItem *space_l, *space_r ;
	
  ret = gtk_toolbar_new();
  tool = GTK_TOOLBAR(ret);	

  /* Label for selected notes */

  space_l = gtk_separator_tool_item_new(); 
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(space_l),FALSE);
  gtk_tool_item_set_expand(space_l,TRUE);
  gtk_toolbar_insert(tool,space_l,-1);

  parent->priv->label = gtk_label_new("");
  update_selection_label(parent,0);
  GtkToolItem *item = gtk_tool_button_new(parent->priv->label,NULL);
  gtk_toolbar_insert(tool,item,-1);

  space_r = gtk_separator_tool_item_new(); 
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(space_r),FALSE);
  gtk_tool_item_set_expand(space_r,TRUE);
  gtk_toolbar_insert(tool,space_r,-1);

  /*  Button to close selection */
  GtkToolItem *finish_selection ;
  GtkWidget *ok = gtk_label_new("Finished");
  finish_selection = gtk_tool_button_new(ok,NULL);
  gtk_toolbar_insert (tool,finish_selection,-1); 
  parent->priv->finish_selection = finish_selection ; 

  g_signal_connect(finish_selection,"clicked",
                   G_CALLBACK(on_selection_mode_changed),parent);

  gtk_widget_override_background_color(GTK_WIDGET(tool),
                                       GTK_STATE_FLAG_NORMAL,
                                       get_color("DarkSeaGreen4"));
	
  return ret ;
}

GtkWidget *
get_search_entry(BjbMainView *parent)
{
  parent->priv->search_entry = gtk_entry_new ();
  GtkEntry *entry = GTK_ENTRY(parent->priv->search_entry);
	
  gchar *needle = biji_win_get_entry(parent->priv->window);
  if ( needle != NULL )
  {
    gtk_entry_set_text(entry,needle);
  }

  gtk_entry_set_icon_from_stock (entry,
                                 GTK_ENTRY_ICON_SECONDARY,
                                 GTK_STOCK_CLEAR);

  GtkEntryCompletion *completion = gtk_entry_completion_new ();
  gtk_entry_set_completion (entry, completion);
  g_object_unref (completion);
  GtkTreeModel *completion_model = create_completion_model ();
  gtk_entry_completion_set_model (completion, completion_model);
  g_object_unref (completion_model);
  gtk_entry_completion_set_text_column (completion, 0);
		
  g_signal_connect (parent->priv->search_entry, "icon-press",
	                  G_CALLBACK (clear_search_entry_callback),parent);

  GtkEntryBuffer *  entry_buf = gtk_entry_get_buffer(entry);
  g_signal_connect(entry_buf,"inserted-text",
	               G_CALLBACK(action_entry_insert_callback),parent);
  g_signal_connect(entry_buf,"deleted-text",
	               G_CALLBACK(action_entry_delete_callback),parent);	
		
  return parent->priv->search_entry ;
}

static void
show_search_entry(BjbMainView *view,GdkEvent  *event)
{
  view->priv->has_entry = TRUE ;
  gtk_widget_show_all(view->priv->hbox_entry);
    
  gtk_widget_grab_focus(GTK_WIDGET(view->priv->search_entry));
}

void  // ? FIXME static void ?
hide_search_entry(BjbMainView *view)
{
  view->priv->has_entry = FALSE ;

  gtk_entry_set_text(GTK_ENTRY(view->priv->search_entry),"");
  gtk_widget_hide (view->priv->search_entry);
  gtk_widget_grab_focus(GTK_WIDGET(view->priv->search_entry));
}

void prepare_view_for_usage(BjbMainView *view)
{
  gtk_widget_show_all(view->priv->window);
  hide_search_entry(view);
  gtk_widget_hide(view->priv->select_toolbar);
}

static gboolean
on_key_pressed(GtkWidget *widget,GdkEvent  *event,gpointer user_data)
{
  g_message("on key pressed");
  BjbMainView *view = BJB_MAIN_VIEW (user_data);

  /* Do not allow search when selecting items */
  if ( view->priv->mode == BJB_SELECTION_MULTIPLE )
  {
    return TRUE ;
  }

  /* Reveal the entry is text is input. TODO add more keys not input*/
  if ( view->priv->has_entry == FALSE )
  {
	  switch (event->key.keyval)
    {
      case GDK_KEY_Control_L :
      case GDK_KEY_Control_R :
      case GDK_KEY_Shift_L :
      case GDK_KEY_Shift_R :
      case GDK_KEY_Alt_L :
      case GDK_KEY_Alt_R :
      case GDK_KEY_Escape :
        return FALSE ; // or return TRUE ? why escape would matter? maybe ?

      default:
        show_search_entry (view,event);    
    }
  }

  /* If there is already an entry and escape pressed, hide entry */
  else
  {
	  g_message("true, has entry");
	  if ( event->key.keyval == GDK_KEY_Escape )
	  {
	    hide_search_entry(view);
      return TRUE ;
	  }
  }

  /* Let the input drop into entry */
  return FALSE ;
}


static gboolean
on_book_changed(BijiNoteBook *book, BjbMainView *view )
{
  update_notes_switch(view,
                      (gchar*) gtk_entry_get_text(
                                           GTK_ENTRY(
                                                 view->priv->search_entry)));
  return TRUE ;
}

static GList *
get_selected_paths(BjbMainView *v)
{
  return gtk_icon_view_get_selected_items (GTK_ICON_VIEW(v->priv->icon_view));
}

static gchar *
get_note_url_from_tree_path(GtkTreePath *path, BjbMainView *view)
{
  GtkTreeIter iter ;
  gchar *note_path ;
  GtkTreeModel *model ;

  model = gtk_icon_view_get_model(GTK_ICON_VIEW(view->priv->icon_view));

    
  gtk_tree_model_get_iter (model,&iter, path);
  gtk_tree_model_get (model, &iter,COL_PATH, &note_path,-1);
  //gtk_tree_path_free(path);

  return note_path ;
}

void action_delete_selected_notes(GtkWidget *w,BjbMainView *view)
{
  GList *notes = NULL ;
  gint i ; 

  GList *paths = get_selected_paths(view);
  for ( i=0 ;  i < g_list_length (paths) ; i++ )
  {
    gchar *url = get_note_url_from_tree_path(g_list_nth_data(paths,i),
                                             view) ;
      
    notes = g_list_append(notes,
                          note_book_get_note_at_path
                          (bijiben_window_get_book(view->priv->window),url));
    
  } 

  for (i=0 ; i<g_list_length(notes) ;i++ )
  {
    BijiNoteObj *note = g_list_nth_data(notes,i) ;
      
    biji_note_delete_from_tracker(note);
    biji_note_book_remove_note(bijiben_window_get_book(view->priv->window),note);
  }
}

BjbMainView*
bjb_main_view_new(GtkWidget *win,BijiNoteBook *book)
{
  BjbMainView *ret ;
  GtkWidget *vbox, *icon_view; 
    
  ret = g_object_new(BJB_TYPE_MAIN_VIEW,NULL);
  ret->priv->window = win ;
    
  ret->priv->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  ret->priv->vbox_up = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

  vbox = ret->priv->vbox;
  gtk_box_pack_start(GTK_BOX(ret),GTK_WIDGET(vbox),TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(vbox),ret->priv->vbox_up,FALSE,FALSE,0);

  /* Create both toolbars */
  ret->priv->standard_toolbar = create_standard_toolbar(ret);
  gtk_box_pack_start(GTK_BOX(ret->priv->vbox_up),ret->priv->standard_toolbar,
                     FALSE,FALSE,0) ;

  ret->priv->select_toolbar = create_selection_toolbar(ret);
  gtk_box_pack_start(GTK_BOX(ret->priv->vbox_up),ret->priv->select_toolbar,
                     FALSE,FALSE,0) ;

  /* Test on clutter toolbar */


  // Search entry is inside vbox up for test
  ret->priv->has_entry = FALSE ;
  ret->priv->key_pressed = g_signal_connect(ret->priv->window,"key-press-event",
                                            G_CALLBACK(on_key_pressed),ret); 
  ret->priv->hbox_entry = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL,0);
    
  gtk_box_pack_start (GTK_BOX(ret->priv->hbox_entry),
                      get_search_entry(ret),
                      TRUE,FALSE,0) ;

  gtk_box_pack_start (GTK_BOX(ret->priv->vbox_up),
                      ret->priv->hbox_entry,
                      FALSE,FALSE,0) ;

  // Icon view (TODO or list view...)
  icon_view = create_icon_view(ret) ;
  gtk_box_pack_start(GTK_BOX(vbox),icon_view,TRUE,TRUE,0);
 
  gtk_window_set_title (GTK_WINDOW (win), BIJIBEN_MAIN_WIN_TITLE);

  ret->priv->notes_changed = g_signal_connect(book,"changed",
                                        G_CALLBACK(on_book_changed),ret);
  return ret;
}

GtkWidget *
bjb_main_view_get_window(BjbMainView *view)
{
  return view->priv->window ;
}
