#include <libbiji/libbiji.h>

#include "bjb-bijiben.h"
#include "bjb-main-view.h"
#include "bjb-tags-view.h"
#include "bjb-window-base.h"
#include "bjb-tracker.h"
#include "bjb-note-view.h"

struct _BjbTagsViewPrivate
{
  GtkWidget *window ;
  GtkWidget *toggle_view;
  GtkWidget *tags_view; // GtkTreeView 
  GtkTreeStore *store ;
};

#define BJB_TAGS_VIEW_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BJB_TYPE_TAGS_VIEW, BjbTagsViewPrivate))

G_DEFINE_TYPE (BjbTagsView, bjb_tags_view, GTK_TYPE_BOX);

static void
bjb_tags_view_init (BjbTagsView *object)
{
  object->priv = 
    G_TYPE_INSTANCE_GET_PRIVATE(object,BJB_TYPE_TAGS_VIEW,BjbTagsViewPrivate);
}

static void
bjb_tags_view_finalize (GObject *object)
{
  G_OBJECT_CLASS (bjb_tags_view_parent_class)->finalize (object);
}

static void
bjb_tags_view_class_init (BjbTagsViewClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (BjbTagsViewPrivate));
  object_class->finalize = bjb_tags_view_finalize;
}

// Tags List View Type
enum
{
  COL_TAG,
  NOTES_NUMBER,
  FILES_NUMBER,
  NUM_COLS
};

static GtkTreeStore *
tags_list (void)
{
  GtkTreeStore *store;
  store = gtk_tree_store_new (NUM_COLS,
                              G_TYPE_STRING,
                              G_TYPE_INT,
                              G_TYPE_INT);
  return store;
}

/* Callbacks */
static void
switch_to_notes_view_callback(GtkButton *button,BjbTagsView *view)
{
  /* TODO  */
}


// See no tag notes
static void 
switch_to_untag_notes_view_callback(GtkButton *button,BjbTagsView *view)
{
  /* TODO */
}

// double click on a tag from 3 columns list
static void
switch_to_notes_with_tag(GtkTreeView *tree_view, GtkTreePath *path,
                         GtkTreeViewColumn *column, BjbTagsView *view)
{
  /* TODO */
}

// show files
static void
show_files_with_tag(GtkButton *but,BjbTagsView *view)
{
  // Get the tag.
  GtkTreeIter iter;
  GtkTreeModel *model ;
  GtkTreePath *path = NULL  ;
  GtkTreeView *tree ;
  GtkTreeViewColumn *column = NULL ;
  gchar *tag ;

  tree = (GtkTreeView*)view->priv->tags_view;
  model = gtk_tree_view_get_model(tree);
  gtk_tree_view_get_cursor(tree,&path,&column);

  if ( path == NULL )
  {
    return ;
  }
    
  gtk_tree_model_get_iter (model,&iter, path);
  gtk_tree_model_get (model, &iter,COL_TAG, &tag,-1);
  gtk_tree_path_free(path);

  // Then work with the file
  GError *error = NULL ;
    
  // Solution 1 : show the tag into tracker-needle.
  gchar * execute[3] = { "tracker-needle",tag,NULL } ;

  // S2 : we could also get the list of files
  // and use dbus or something to show them in file manager

  g_spawn_async_with_pipes ( NULL, 
                             execute,
                             NULL, // envp
                             G_SPAWN_SEARCH_PATH,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             &error); 
  if ( error != NULL )
    g_message("error :%s",error->message);
} 

// Add one row to the tags view ( tree store 3 columns list ) 
static void
append_tag_to_store(gchar *tag, BjbTagsView *view)
{
  GtkTreeIter iter;
  GtkTreeStore *store = view->priv->store ;

  // Get data
  BijiNoteBook *book = bjb_window_base_get_book(view->priv->window);
  GList *notes = biji_note_book_get_notes_with_tag(book,tag);
  gint files = tracker_tag_get_number_of_files(tag);

  // Append
  gtk_tree_store_append(store,&iter,NULL);
  gtk_tree_store_set(store,&iter,
                     COL_TAG,tag,
                     NOTES_NUMBER,g_list_length(notes),
                     FILES_NUMBER,files,
                     -1);
}

static void
update_tags (BjbTagsView *view)
{
  GList *tags_list = bjb_window_base_get_tags(view->priv->window);
    
  gtk_tree_store_clear (view->priv->store);
  g_list_foreach(tags_list,(GFunc)append_tag_to_store,view);
}

static gchar*
tags_view_get_selected_tag(BjbTagsView *view)
{
  GtkTreeIter iter;
  GtkTreeModel *model ;
  GtkTreePath *path = NULL  ;
  GtkTreeView *tree ;
  GtkTreeViewColumn *column = NULL ;
  gchar *tag ;

  tree = (GtkTreeView*)view->priv->tags_view;
  model = gtk_tree_view_get_model(tree);
  gtk_tree_view_get_cursor(tree,&path,&column);

  if ( path == NULL )
  {
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_DEBUG,"No tag is selected!");
    return NULL ;
  }
    
  gtk_tree_model_get_iter (model,&iter, path);
  gtk_tree_model_get (model, &iter,COL_TAG, &tag,-1);
  gtk_tree_path_free(path);
  return tag ;
}


static void 
delete_tag_callback(GtkButton *button,BjbTagsView *view)
{ 
  gchar *tag = tags_view_get_selected_tag(view) ;
  if ( tag == NULL )
    return ;

  // Remove it from both tracker and noteBook
  remove_tag_from_tracker(tag);
  biji_note_book_remove_tag(bjb_window_base_get_book(view->priv->window),tag);

  // Update window
  bjb_window_base_set_tags(view->priv->window,get_all_tracker_tags());
  update_tags(view);
}

static GtkWidget *
create_tags_view(BjbTagsView *self,GList *tags,BijiNoteBook *book)
{
  GtkWidget * sw;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
    
  self->priv->store = tags_list() ;
  update_tags (self);
  self->priv->tags_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(self->priv->store));

  g_signal_connect(self->priv->tags_view,"row-activated",
                   G_CALLBACK(switch_to_notes_with_tag),self);

  // Tag Name Column 
    
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Tag",
                                                     renderer,
                                                     "text",
                                                     COL_TAG,
                                                     NULL);
  gtk_tree_view_column_set_expand(column,TRUE);
  gtk_tree_view_append_column (GTK_TREE_VIEW(self->priv->tags_view), column);
  gtk_tree_view_column_set_sort_column_id (column, COL_TAG);

  // Notes Number
    
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Notes",
                                                     renderer,
                                                     "text",
                                                     NOTES_NUMBER,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW(self->priv->tags_view), column);
  gtk_tree_view_column_set_sort_column_id (column, NOTES_NUMBER);

  // Files Number

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Files",
                                                     renderer,
                                                     "text",
                                                     FILES_NUMBER,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW(self->priv->tags_view), column);
  gtk_tree_view_column_set_sort_column_id (column, FILES_NUMBER);
   
  gtk_container_add (GTK_CONTAINER (sw), self->priv->tags_view);
  return sw;
}


static void
action_new_tag_callback(GtkButton * but, BjbTagsView *view)
{
  GtkWindow *win;
  GtkWidget *dialog;
  GtkWidget *area;
  GtkWidget *entry;
  gint result ;

  win = GTK_WINDOW(view->priv->window) ;
	
  dialog = gtk_dialog_new_with_buttons("New tag",
	                                     win,
	                                     GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT,
										                   GTK_STOCK_CANCEL,
	                                     GTK_RESPONSE_CANCEL,
	                                     GTK_STOCK_OK,
	                                     GTK_RESPONSE_OK,
	                                     NULL);
  area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  entry = gtk_entry_new ();
  gtk_container_set_border_width (GTK_CONTAINER (area), 8);
  gtk_box_pack_start (GTK_BOX (area), entry, TRUE, TRUE,2);
  gtk_widget_show_all(area);

  result = gtk_dialog_run (GTK_DIALOG (dialog));
  if ( result == GTK_RESPONSE_OK)
  {
    // Push Tag
    push_tag_to_tracker((gchar*)gtk_entry_get_text(GTK_ENTRY(entry)));

    // Update window
    bjb_window_base_set_tags(view->priv->window,get_all_tracker_tags());
    update_tags(view);
  }
    
  gtk_widget_destroy (dialog);
}

/*
static void
on_new_tag_note(GtkWidget *button,BjbTagsView *view)
{
  GtkWidget *win = view->priv->window;
  BijiNoteBook *book =  bjb_window_base_get_book(win);

  // Get the tag
  gchar *tag = tags_view_get_selected_tag(view);
  if ( tag == NULL )
    return ;

  // Create the note
  BijiNoteObj *result ;
  gchar *title = "yo" ;  
  gchar *folder = g_strdup_printf("%s/bijiben",g_get_user_data_dir());
  result = biji_note_get_new_from_string(title,folder);
  g_free(folder);
  biji_note_obj_add_tag(result,tag);

  // Append the note, save it, switch to it
  note_book_append_new_note(book,result);
  switch_from_tag_to_note_view(view,result);
  note_obj_save_note_using_buffer((gpointer)result);
} */

// should be in libbiji ?
BijiNoteObj *
create_template (BjbTagsView *view, gchar *tag)
{    
  gchar *folder = g_strdup_printf("%s/bijiben",g_get_user_data_dir());
  BijiNoteObj *ret = biji_note_get_new_from_string(tag,folder);
  g_free(folder);

  // First, append the note to book
  // So when we add the tag , the note knows.
  note_obj_set_is_template(ret,TRUE);
  note_book_append_new_note(bjb_window_base_get_book(view->priv->window),ret) ;  
  biji_note_obj_add_tag(ret,tag);
  return ret ;  
}

BijiNoteObj *
get_or_create_template(BjbTagsView *view, gchar *tag)
{
  BijiNoteObj *result ;
    
  /* Get template if exists */
  result = biji_note_book_get_tag_template(
                                   bjb_window_base_get_book(view->priv->window),
                                   tag);

  /* Other wise create */
  if ( result == NULL )
  { 
    g_log(G_LOG_DOMAIN,G_LOG_LEVEL_DEBUG,"Template does not exist yet.");
    result = create_template(view,tag) ;
  }

  return result ;
}

static void
on_edit_template(GtkWidget *button,BjbTagsView *view)
{
  g_message("on edit template");
  g_return_if_fail(BJB_IS_TAGS_VIEW(view));
    
  BijiNoteObj *result =  get_or_create_template(view,
                                                tags_view_get_selected_tag(view));
        
  create_new_window_for_note(g_application_get_default(),result);
}

static void
open_tag (GtkAction *action, BjbTagsView *view)
{
  /* TODO */
}

static void
do_popup_menu (GtkWidget *my_widget, GdkEventButton *event,BjbTagsView *view)
{
  g_message("do popup menu");
  GtkWidget *menu, *item ;
  int button, event_time;

  menu = gtk_menu_new ();

  // View notes
  item = gtk_menu_item_new_with_label("View these notes");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
  g_signal_connect(item,"activate",G_CALLBACK(open_tag),view);
  gtk_widget_show(item);

  // New note with that tag - SEGFAULT
  item = gtk_menu_item_new_with_label("New note");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
  /*g_signal_connect(item,"activate",G_CALLBACK(on_new_tag_note),view); deprecated*/
  gtk_widget_show(item);

  // View files OK
  item = gtk_menu_item_new_with_label("View the files");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
  g_signal_connect(item,"activate",G_CALLBACK(show_files_with_tag),view);
  gtk_widget_show(item);

  // Edit template - SEGFAULT
  item = gtk_menu_item_new_with_label("Edit template");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
  g_signal_connect(item,"activate",G_CALLBACK(on_edit_template),view);
  gtk_widget_show(item);

  // Delete tag - OK mais ça peut échoue sur un tag avec ( , ) ...
  item = gtk_menu_item_new_with_label("Delete tag");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
  g_signal_connect(item,"activate",G_CALLBACK(delete_tag_callback),view);
  gtk_widget_show(item);

  if (event)
  {
    button = event->button;
    event_time = event->time;
  }
  else
  {
    button = 0;
    event_time = gtk_get_current_event_time ();
  }

  gtk_menu_attach_to_widget (GTK_MENU (menu), my_widget, NULL);
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                  button, event_time);
} 

static gboolean
on_tag_list_button_press (GtkWidget *widget, GdkEventButton *event,BjbTagsView *view)
{
  g_message("on tag list but");
    
  // Ignore double-clicks and triple-clicks
  if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
  {
    do_popup_menu (widget, event,view);
    return TRUE;
  }

  return FALSE;
}

static gboolean
on_tags_view_popup_menu(GtkWidget *widget, BjbTagsView *view)
{
  g_message("on tag view pop");
    
  do_popup_menu (widget, NULL,view);
  return TRUE;
}

static GtkWidget *
create_toolbar(BjbTagsView *parent)
{
  GtkToolItem *iter ;
  GtkWidget *ret = gtk_toolbar_new();
  GtkToolbar *tool = GTK_TOOLBAR(ret);

  // Go-to-notes button
  GtkWidget *notes_button, *but_grid, *notes_icon, *notes_label ;
  GIcon     *gicon;

  iter = gtk_tool_item_new();
  but_grid = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
  notes_label = gtk_label_new("All notes");
  gtk_box_pack_start(GTK_BOX(but_grid),notes_label,TRUE,TRUE,TRUE);
  gicon = g_themed_icon_new_with_default_fallbacks ("go-next-symbolic");
  notes_icon = gtk_image_new_from_gicon (gicon,GTK_ICON_SIZE_BUTTON);	
  gtk_box_pack_start(GTK_BOX(but_grid),notes_icon,TRUE,TRUE,TRUE);
  notes_button = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(notes_button),but_grid);
  gtk_button_set_relief(GTK_BUTTON(notes_button),GTK_RELIEF_NONE);
  g_signal_connect(notes_button,"clicked",
                   G_CALLBACK(switch_to_notes_view_callback),parent) ;
  gtk_container_add (GTK_CONTAINER(iter),notes_button);
  gtk_toolbar_insert (tool,iter,-1);

  // Go to no tag
  GtkWidget *no_tag_button , *but_grid2, *no_tag_icon, *no_tag_label ;

  iter = gtk_tool_item_new();
  but_grid2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
  no_tag_label = gtk_label_new("Not tagged");
  gtk_box_pack_start(GTK_BOX(but_grid2),no_tag_label,TRUE,TRUE,TRUE);
  no_tag_icon = gtk_image_new_from_gicon (gicon,GTK_ICON_SIZE_BUTTON);	
  gtk_box_pack_start(GTK_BOX(but_grid2),no_tag_icon,TRUE,TRUE,TRUE);
  no_tag_button = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(no_tag_button),but_grid2);
  gtk_button_set_relief(GTK_BUTTON(no_tag_button),GTK_RELIEF_NONE);
  g_signal_connect(no_tag_button,"clicked",
                   G_CALLBACK(switch_to_untag_notes_view_callback),parent);
  gtk_container_add (GTK_CONTAINER(iter),no_tag_button);
  gtk_toolbar_insert (tool,iter,-1);

  //
  iter = gtk_separator_tool_item_new(); 
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(iter),FALSE);
  gtk_tool_item_set_expand(iter,TRUE);
  gtk_toolbar_insert(tool,iter,-1);

  // Search Entry ?
    
  /*

  // TODO New Note with tag on_new_tag_note
  iter = gtk_tool_button_new_from_stock(GTK_STOCK_EDIT);
  g_signal_connect(iter,"clicked",G_CALLBACK(on_new_tag_note),parent);
  gtk_toolbar_insert(tool,iter,-1);

  // Edit Template GTK_STOCK_EDIT on_edit_template
  iter = gtk_tool_button_new_from_stock(GTK_STOCK_PROPERTIES);
  g_signal_connect(iter,"clicked",G_CALLBACK(on_edit_template),parent);
  gtk_toolbar_insert(tool,iter,-1);  

  // View Files GTK_STOCK_DIRECTORY
  iter = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
  g_signal_connect(iter,"clicked",
	                 G_CALLBACK(show_files_with_tag),parent) ;
  gtk_toolbar_insert(tool,iter,-1);  
    
  // Delete Tag GTK_STOCK_CLOSE 
  iter = gtk_tool_button_new_from_stock(GTK_STOCK_CLOSE);
  g_signal_connect(iter,"clicked",
	                 G_CALLBACK(delete_tag_callback),parent) ;
  gtk_toolbar_insert(tool,iter,-1);  

  // */

  // New-tag button
  iter = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
  g_signal_connect(iter,"clicked",
                   G_CALLBACK(action_new_tag_callback),parent);
  gtk_toolbar_insert (tool,iter,-1);

  return ret ;
}

ClutterActor *
bjb_tags_view_new(GtkWidget *biji_main_window)
{
  BjbTagsView * ret = g_object_new(BJB_TYPE_TAGS_VIEW,NULL);
  ret->priv->window = biji_main_window ;
  GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  gtk_container_add(GTK_CONTAINER(ret),GTK_WIDGET(vbox));

  /** Toolbar */ 
  gtk_box_pack_start(GTK_BOX(vbox),create_toolbar(ret),
                    FALSE,FALSE,0);

  /* Scrolled */  
  GList *tags ;
  GtkWidget *view ;
  GtkWindow *win = GTK_WINDOW(biji_main_window);
  tags = bjb_window_base_get_tags(biji_main_window);
  view = create_tags_view(ret,tags,bjb_window_base_get_book(biji_main_window));
  gtk_box_pack_start(GTK_BOX(vbox),view,TRUE,TRUE,0);

  g_signal_connect(ret->priv->tags_view,"popup-menu",
                   G_CALLBACK(on_tags_view_popup_menu),ret);
  g_signal_connect(ret->priv->tags_view,"button-press-event",
                   G_CALLBACK(on_tag_list_button_press),ret);
    
  gtk_window_set_title (win, "Biji - View tags");

  ClutterActor *dummy = NULL ;
  return dummy ;
}
