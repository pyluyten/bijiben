#include <gtk/gtk.h>
#include <libbiji/libbiji.h>

#include "utils/bjb-icons-colors.h"
#include "widgets/bjb-menu-tool.h"

#include "bjb-bijiben.h"
#include "bjb-rename-note.h"
#include "bjb-share.h"
#include "bjb-tracker.h"
#include "bjb-main-view.h"
#include "bjb-note-view.h"
#include "bjb-window-base.h"
#include "bjb-zeitgeist-dataprovider.h"

/* Default color (X11 rgb.txt) - maybe gsettings instead */ 
#define DEFAULT_NOTE_COLOR "LightGoldenrodYellow"

G_DEFINE_TYPE (BjbNoteView, bjb_note_view, GTK_TYPE_FRAME)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BJB_TYPE_NOTE_VIEW, BjbNoteViewPrivate))

struct _BjbNoteViewPrivate {
  GtkWidget *window ;
  GtkTextView *view ;
  BijiNoteObj *current_note ;
  GtkBox *toolbars_box; // delete ?
  GtkTextBuffer *buffer ;
  GtkWidget *bar ; // Infobar "no tag", might be NULL
  GtkWidget *vbox;

  // ?
  gboolean is_main_window ;

  // 2nd toolbar when text is selected ; and button to toggle these
  GtkWidget *edit_bar ;
  gboolean edit_bar_is_sticky ;

  //Manage tags dialog.
  GtkWidget *tags_dialog;
  GtkWidget *entry ; // text entry to input new tags.
  GtkTreeModel *model; // tag model

  // Convenience
  GdkRGBA *color ;
	
  // hack when widget is destroyed.Probably obsolete.
  gboolean to_be_saved ;

  // Signal when window is destroyed. We may need to disconect g_signal
  gulong    destroy ;
  gulong    renamed ;
  gulong    deleted ;
};

static void
bjb_note_view_finalize(GObject *object)
{
  BjbNoteView *view = BJB_NOTE_VIEW(object) ;

  // Don't unref buffer. Biji Does it when we close note.
  g_signal_handler_disconnect(view->priv->current_note,view->priv->renamed);
  g_signal_handler_disconnect(view->priv->window,view->priv->destroy);
  g_signal_handler_disconnect(view->priv->current_note,view->priv->deleted);

  G_OBJECT_CLASS (bjb_note_view_parent_class)->finalize (object);
}

static void
bjb_note_view_class_init (BjbNoteViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
    
  object_class->finalize = bjb_note_view_finalize;
  g_type_class_add_private (klass, sizeof (BjbNoteViewPrivate));
}

static void
bjb_note_view_init (BjbNoteView *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BJB_TYPE_NOTE_VIEW,
	                                        BjbNoteViewPrivate);

  self->priv->window = NULL ;
  self->priv->view = NULL ;
  self->priv->current_note = NULL ;
  self->priv->toolbars_box = NULL ;
  self->priv->buffer = NULL ;
  self->priv->model  = NULL ;
  self->priv->bar = NULL ;
  self->priv->edit_bar_is_sticky = FALSE ;
  self->priv->to_be_saved = TRUE ;
  self->priv->tags_dialog = NULL ;
}

// Handlers for GtkTreeView of tags
enum 
{
  TAG_NAME_COLUMN ,
  TAG_CHECK_COLUMN ,

  VISIBLE_COLUMN,
  SELECT_COLUMN,
  NUM_COLUMNS
} ;

static GtkTreeModel *
create_tags_model (BjbNoteView *view)
{
  GtkTreeStore *model;
  GtkTreeIter iter;
  GList *all_tags ;

  model = gtk_tree_store_new (NUM_COLUMNS,
                              G_TYPE_STRING,
                              G_TYPE_BOOLEAN,
                              G_TYPE_BOOLEAN,
                              G_TYPE_BOOLEAN);

  // Fill in the tree with ALL tags including no note tags.
  // But we might first retrieve all tags,
  // then display active tags for this note,
  // then display other notes tags
  // then display tags active for files
  // then display tags not active at all
  gint i ;
  all_tags = bjb_window_base_get_tags(view->priv->window);
  for ( i=0 ; i<g_list_length (all_tags) ; i++ )
  {
    gchar *tag = g_list_nth_data(all_tags,i);
    gtk_tree_store_append (model, &iter, NULL);
    gtk_tree_store_set (model, &iter,
                        TAG_NAME_COLUMN,tag,
                        TAG_CHECK_COLUMN,
                        biji_note_obj_has_tag(view->priv->current_note,tag),
                        VISIBLE_COLUMN,TRUE,
                        SELECT_COLUMN,TRUE,
                        -1);
  }
  
  return GTK_TREE_MODEL (model);       
}

// TODO
static void
tag_toggled (GtkCellRendererToggle *cell,gchar *path_str,BjbNoteView *view)
{
  GtkTreeModel *model = (GtkTreeModel *)view->priv->model;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  GtkTreeIter iter;
  gboolean toggle_item;
  gint *column;
  gchar *tag ;

  column = g_object_get_data (G_OBJECT (cell), "column");
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, column, &toggle_item, -1);
  gtk_tree_model_get (model, &iter,TAG_NAME_COLUMN, &tag,-1);
    
  if ( toggle_item == FALSE )
  {
    // add into libbiji then tracker.
    biji_note_obj_add_tag(view->priv->current_note,tag);
    note_obj_save_note_using_buffer(view->priv->current_note);
    push_existing_tag_to_note(tag,view->priv->current_note);
                          
    // and update the cell
    toggle_item = TRUE ;
  }
  else
  {
    // Remove the tag as in libiji, also remove tracker tag
    biji_note_obj_remove_tag(view->priv->current_note,tag);
    note_obj_save_note_using_buffer(view->priv->current_note);
    remove_tag_from_note(tag,view->priv->current_note);

    // and update the cell
    toggle_item = FALSE ;
  }
  gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,
                      toggle_item, -1);
    
  gtk_tree_path_free (path);
}


static void
add_columns (GtkTreeView *treeview,BjbNoteView *view)
{
  gint col_offset;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  // TAG NAME
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer, "xalign", 0.0, NULL);

  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                            -1, "Tag",
                                                            renderer, "text",
                                                            TAG_NAME_COLUMN,
                                                            NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);
  gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);

  // TAG ACTIVE
  renderer = gtk_cell_renderer_toggle_new ();
  g_object_set (renderer, "xalign", 0.0, NULL);
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)TAG_CHECK_COLUMN);

  
  g_signal_connect (renderer, "toggled", G_CALLBACK (tag_toggled), view);

  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                            -1, "Select",
                                                            renderer,
                                                            "active",
                                                            TAG_CHECK_COLUMN,
                                                            "visible",
                                                            VISIBLE_COLUMN,
                                                            "activatable",
                                                            SELECT_COLUMN, NULL);

  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                   GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

}

static void 
show_tags_dialog(BjbNoteView *view);

static void
add_new_tag(GtkButton *new, BjbNoteView *view)
{
  /* Push the tag to tracker */
  push_tag_to_tracker((gchar*)gtk_entry_get_text(GTK_ENTRY(view->priv->entry)));
  bjb_window_base_set_tags(view->priv->window,get_all_tracker_tags());

  /* Update the view */
  gtk_widget_destroy (view->priv->tags_dialog);
  view->priv->tags_dialog = NULL ;
  show_tags_dialog(view);
}

static void
show_tags_dialog(BjbNoteView *view)
{
  GtkWidget *area, *treeview, *sw ;
  GtkWindow *win;

  // First, push the note to tracker if it's new.
  bijiben_push_note_to_tracker(view->priv->current_note);
    
  win = GTK_WINDOW(view->priv->window);
    
  view->priv->tags_dialog = gtk_dialog_new_with_buttons("Notes tag",
	                                     win,
	                                     GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT,
	                                     GTK_STOCK_OK,
	                                     GTK_RESPONSE_OK,
	                                     NULL);
    
  area = gtk_dialog_get_content_area (GTK_DIALOG (view->priv->tags_dialog));
  gtk_container_set_border_width (GTK_CONTAINER (area), 8);

  // Create a new tag-button
  GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
  view->priv->entry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox),view->priv->entry,TRUE,TRUE,0);
  GtkWidget *new = gtk_button_new_with_label("New tag");
  g_signal_connect(new,"clicked",G_CALLBACK(add_new_tag),view);
  gtk_box_pack_start (GTK_BOX(hbox),new,FALSE,FALSE,0);
  gtk_box_pack_start (GTK_BOX (area),hbox, FALSE, FALSE,0);

  // List of tags
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  view->priv->model = create_tags_model (view);
  treeview = gtk_tree_view_new_with_model (view->priv->model);
  g_object_unref (view->priv->model);

  
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
                                   GTK_SELECTION_MULTIPLE);
  
  add_columns (GTK_TREE_VIEW (treeview),view);
  gtk_container_add (GTK_CONTAINER (sw), treeview);
    
  gtk_box_pack_start (GTK_BOX (area), sw, TRUE, TRUE,2);
  gtk_widget_show_all(area);

  // deal with the result ?? no sense.
  gtk_window_set_default_size (GTK_WINDOW(view->priv->tags_dialog), 320, 380 );
  /*result =*/ gtk_dialog_run (GTK_DIALOG (view->priv->tags_dialog));

  if ( GTK_IS_WIDGET(view->priv->tags_dialog))
  {
    gtk_widget_destroy(view->priv->tags_dialog);
    view->priv->tags_dialog = NULL ;
  }
}

/* tracker is not handled by libbiji */
static void
bjb_close_note(gpointer note)
{
  bijiben_push_note_to_tracker(note);
  biji_note_close(note);
}

static gboolean
on_window_closed(GtkWidget *window,gpointer note)
{   
  if ( window == NULL )
  {
    g_message ("on window closed : note deleted, don't close it");
    return TRUE ;
  }
    
  bjb_close_note(note) ;
  return TRUE ;
}

// Callbacks

static void
just_switch_to_main_view(BjbNoteView *view)
{
  GtkWindow *window;
  BijiNoteBook *notes;
  BjbController *controller;
  GtkContainer *to_be;
    
  window = view->priv->window;
  controller = bjb_window_base_get_controller(window);
  
  to_be = GTK_CONTAINER(bjb_main_view_new((gpointer)window,
                                           controller));
            
  bjb_window_base_set_frame((gpointer)view->priv->window,GTK_CONTAINER(to_be));
  prepare_view_for_usage((BjbMainView*)to_be);
}

static void
save_then_switch_to_notes_view(BjbNoteView *view)
{
  bijiben_push_note_to_tracker(view->priv->current_note);
  bjb_close_note(view->priv->current_note);
  just_switch_to_main_view(view);
}

static void
action_switch_to_notes_callback(GtkButton *but,BjbNoteView *view)
{
  save_then_switch_to_notes_view(view);
}

static void
action_view_tags_callback(GtkButton *but,BjbNoteView *view)
{
  // If Infobar we should hide it...
  if (BJB_NOTE_IS_VIEW(view))
  {
    show_tags_dialog(view);
  }
  else
  {
    g_message("callback view tags wrong");
  }
}

static void
action_rename_note_callback(GtkButton *but,BjbNoteView *view)
{
  gchar * title = note_title_dialog(GTK_WINDOW(view->priv->window),
                                 "Rename Note",
                                 biji_note_get_title(view->priv->current_note));
   
  if ( !title)
  {
    g_message("No title set");
    return ;
  }

  else
  {
    g_message("renaming note");
  }

  set_note_title(view->priv->current_note,title);
  gtk_window_set_title (GTK_WINDOW(view->priv->window),
                        title);
}

static void
link_callback(GtkButton *item,BjbNoteView *view)
{
  gchar *link = gtk_text_view_get_selection(view->priv->view);

  // Actually that's critical, button should be toggled.
  if (link == NULL )
  {
    g_message("NO SELECTION, CANNOT LINK");
  }

  g_message("selection is %s",link);
 
  // else, create it
  BijiNoteObj *result ;
  BijiNoteBook *book = bjb_window_base_get_book(view->priv->window);
  gchar *folder = g_strdup_printf("%s/bijiben",g_get_user_data_dir());
  result = biji_note_get_new_from_string(link,folder);
  g_free(folder);
  //g_free(link); FIXME libbiji note get new from string should 
  // allocate it's own strings for notes title...
    
  note_book_append_new_note(book,result);
  create_new_window_for_note(bjb_window_base_get_app(view->priv->window) , result) ;
  
}

static void
on_info_bar_callback(GtkWidget *infobar,gint response, BjbNoteView *view)
{
  gtk_widget_destroy (infobar );
  show_tags_dialog(view);
}

static void
delete_item_callback(GtkMenuItem *item,BjbNoteView *view)
{
  g_return_if_fail(BJB_NOTE_IS_VIEW(view));

  biji_note_delete_from_tracker(view->priv->current_note);

  // Delete the note from collection
  biji_note_book_remove_note(bjb_window_base_get_book(view->priv->window),
	                         view->priv->current_note);

  // The deleted note will emit a signal.
}

// FIXME add a label to buttons to call one signle func
static void
bold_button_callback(GtkWidget *button,GtkTextView *view)
{
  biji_toggle_bold_tag(view);
}

/*static void
highlight_button_callback(GtkWidget *button,GtkTextView *view)
{
  biji_toggle_highlight_tag(view);
}*/

static void
italic_button_callback(GtkWidget *button,GtkTextView *view)
{
  biji_toggle_italic_tag(view);
}

/*static void
underline_button_callback(GtkWidget *button,GtkTextView *view)
{
  biji_toggle_underline_tag(view);
}*/

static void
strike_button_callback(GtkWidget *button,GtkTextView *view)
{
  biji_toggle_strike_tag(view);
}

/*static void
augment_size_button_callback(GtkWidget *button,GtkTextView *view)
{
  biji_augment_font_size(view);
}

static void
decrease_size_button_callback(GtkWidget *button,GtkTextView *view)
{
  biji_decrease_font_size(view);
}

static void
bullet_augment_callback(GtkWidget *button,GtkTextView *view)
{
  biji_augment_depth(view);
}

static void
bullet_decrease_callback(GtkWidget *button,GtkTextView *view)
{
  biji_decrease_depth(view);
}*/

static void
on_focus(GtkTextView *text_view,GdkEvent *event,BjbNoteView *view)
{
  if ( view->priv->bar != NULL )
  { 
    gtk_widget_destroy(view->priv->bar);
    view->priv->bar = NULL ;
  }

  // FIXME we should rather g_signal_handler_disconnect on_focus 
  // than test if on_focus still has sense.
}

static void
action_view_note_in_another_win_callback(GtkAction *action, BjbNoteView *view)
{
  BijiNoteObj *note = view->priv->current_note ;
  GtkWidget *window = view->priv->window ;

  // /* Before rewrite 
  // Switch current window to notes list ?or not ?
  save_then_switch_to_notes_view(view);
  // Pop up a new window 
  create_new_window_for_note(bjb_window_base_get_app(window) ,note ) ; // */

}

static void
debug_note_view(GtkButton *button,BjbNoteView *view)
{
  biji_print_note( view->priv->buffer) ;
}

static void
set_editor_color(BjbNoteView *v, GdkRGBA *to_be)
{
  g_message("set editor color");
  gtk_widget_override_background_color(GTK_WIDGET(v->priv->view),
                                       GTK_STATE_FLAG_NORMAL,to_be);
}

static void
on_color_choosed(  GtkDialog *dialog,
                   gint       response_id,
                   BjbNoteView *view    )
{
  GdkRGBA *color = g_new(GdkRGBA,1) ;

  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (dialog), color);
	
  if (response_id == GTK_RESPONSE_OK)
  {
    set_editor_color(view,color);
	biji_note_obj_set_rgba(view->priv->current_note,color) ;
  }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
on_color_clicked(GtkWidget *but,BjbNoteView *view)
{	
  GtkWidget *dialog;
    
  dialog = gtk_color_chooser_dialog_new ("Choose note background",
                                         GTK_WINDOW(view->priv->window));  

  g_signal_connect (dialog,
                    "response",
                    G_CALLBACK (on_color_choosed),
                    view); 
	
  gtk_widget_show_all(dialog);
}


/* Draw the color button */
static gboolean
on_color_draw(GtkWidget *widget, cairo_t *cr, BijiNoteObj *note)
{
   g_message("on color draw");
    
   cairo_rectangle  (cr,1,1,100,100);  

   GdkRGBA *color = biji_note_obj_get_rgba(note) ;

   if ( !color )
   {
     gdk_rgba_parse( color, DEFAULT_NOTE_COLOR ) ;
   }

   gdk_cairo_set_source_rgba (cr, biji_note_obj_get_rgba(note));
   cairo_fill (cr);
   return FALSE ;
}

static GtkWidget *
create_toolbar(BjbNoteView *parent,BijiNoteObj *note)
{
  GtkWidget *ret ;
  GtkToolbar *tool;
  GtkToolItem *space_l, *space_r ;

  ret = gtk_toolbar_new();
  tool = GTK_TOOLBAR(ret);

  if ( parent->priv->is_main_window )
  {
    
    // Go to Notes button
    GtkToolItem *go_to_notes ;
    GtkWidget *notes_button ;
    GtkWidget *but_grid;
    GtkWidget *notes_icon ;
    GtkWidget *notes_label;

    go_to_notes = gtk_tool_item_new();
    but_grid = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
    notes_icon = get_icon("go-previous-symbolic");
    gtk_box_pack_start(GTK_BOX(but_grid),notes_icon,TRUE,TRUE,TRUE);
    notes_label = gtk_label_new("Notes");
    gtk_box_pack_start(GTK_BOX(but_grid),notes_label,TRUE,TRUE,TRUE);
    notes_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(notes_button),but_grid);
    g_signal_connect(notes_button,"clicked",
                     G_CALLBACK(action_switch_to_notes_callback),parent) ;  
    gtk_container_add (GTK_CONTAINER(go_to_notes),notes_button);
    gtk_tool_item_set_expand(go_to_notes,FALSE);
    gtk_toolbar_insert (tool,go_to_notes,-1);

  }

  /* DEBUG
  GtkToolItem *iter = gtk_tool_button_new_from_stock(GTK_STOCK_EDIT);
  g_signal_connect(iter,"clicked",
                 G_CALLBACK(debug_note_view),parent);
  gtk_toolbar_insert (tool,iter,-1);
  // */


  // space
  space_l = gtk_separator_tool_item_new(); 
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(space_l),FALSE);
  gtk_tool_item_set_expand(space_l,TRUE);
  gtk_toolbar_insert (tool,space_l,-1);
    
  // 
  space_r = gtk_separator_tool_item_new(); 
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(space_r),FALSE);
  gtk_tool_item_set_expand(space_r,TRUE);
  gtk_toolbar_insert (tool,space_r,-1);

  /* Color Chooser (Simple Cairo). */
  GtkToolItem *color ;
  GtkWidget *co = gtk_drawing_area_new();
  color = gtk_tool_button_new(co,NULL);
  g_signal_connect(co,"draw",G_CALLBACK(on_color_draw),note);
  g_signal_connect(color,"clicked",
                   G_CALLBACK(on_color_clicked),parent) ;
  //gtk_tool_button_set_relief_style(color,GTK_RELIEF_NORMAL);
  gtk_toolbar_insert (tool,color,-1); // */


  /* Sharing is not yet implemented, only e-mail (bjb-share) */
  GtkToolItem *share_b ;  
  share_b = gtk_tool_button_new(get_icon("send-to-symbolic"),NULL);
  g_signal_connect(share_b,"clicked",
                   G_CALLBACK(on_email_note_callback),note);
  gtk_toolbar_insert(tool,GTK_TOOL_ITEM(share_b),-1);
  
		
  // Action menu 
  GtkToolItem *action = gtk_tool_item_new();
  GtkMenu *action_menu = GTK_MENU(gtk_menu_new());
  GtkWidget *action_image = get_icon("emblem-system-symbolic");

  BijibenMenuButton *action_toggle = bijiben_menu_button_new(action_image);
  bijiben_menu_button_set_menu(action_toggle,GTK_MENU(action_menu));

  gtk_container_add(GTK_CONTAINER(action),GTK_WIDGET(action_toggle));
  gtk_toolbar_insert (tool,action,-1);
  
  // Action menu - Rename
  GtkWidget *rename_it = gtk_menu_item_new_with_label("Rename");
  gtk_menu_shell_append(GTK_MENU_SHELL(action_menu),rename_it);
  g_signal_connect(rename_it,
                   "activate",
	               G_CALLBACK(action_rename_note_callback),
                   parent); 
  gtk_widget_show(rename_it);
  
  // Action menu - Tags
  GtkWidget *tags_item = gtk_menu_item_new_with_label("Tags");
  gtk_menu_shell_append(GTK_MENU_SHELL(action_menu),tags_item);
  g_signal_connect(tags_item,"activate",
	                 G_CALLBACK(action_view_tags_callback),parent);
  gtk_widget_show(tags_item);

  // Action menu - Delete
  GtkWidget *delete_item = gtk_menu_item_new_with_label("Delete this note");
  gtk_menu_shell_append(GTK_MENU_SHELL(action_menu),delete_item);
  g_signal_connect(delete_item,"activate",
	                G_CALLBACK(delete_item_callback),parent);
  gtk_widget_show(delete_item);

  return ret ;
}


static gboolean
on_cut_clicked ( GtkWidget *button, GtkTextView *view)
{
  g_signal_emit_by_name (view,"cut-clipboard");
  return TRUE ;
}

static gboolean
on_copy_clicked ( GtkWidget *button, GtkTextView *view)
{
  g_signal_emit_by_name (view,"copy-clipboard");
  return TRUE ;
}

static gboolean
on_paste_clicked ( GtkWidget *button, GtkTextView *view)
{
  g_signal_emit_by_name (view,"paste-clipboard");
  return TRUE ;
}

static GtkWidget *
create_edit_bar(BjbNoteView *parent)
{
  GtkTextView *view = parent->priv->view ;
    
  GtkToolItem *iter ;
  GtkWidget *result = gtk_toolbar_new();
  GtkToolbar *bar = GTK_TOOLBAR(result) ;

  // First space to center buttons
  iter = gtk_separator_tool_item_new(); 
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(iter),FALSE);
  gtk_tool_item_set_expand(iter,TRUE);
  gtk_toolbar_insert (bar,iter,-1);

  /* Cut */
  iter = gtk_tool_button_new(gtk_label_new("Cut"),NULL);
  g_signal_connect(iter,"clicked",G_CALLBACK(on_cut_clicked),view);
  gtk_toolbar_insert (bar,iter,-1); 

  /* Copy */
  iter = gtk_tool_button_new(gtk_label_new("Copy"),NULL);
  g_signal_connect(iter,"clicked",G_CALLBACK(on_copy_clicked),view);
  gtk_toolbar_insert (bar,iter,-1); 

  /* Paste */
  iter = gtk_tool_button_new(gtk_label_new("Paste"),NULL);
  g_signal_connect(iter,"clicked",G_CALLBACK(on_paste_clicked),view);
  gtk_toolbar_insert (bar,iter,-1); 

	// Bold
  iter = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_BOLD);
  if ( gtk_text_view_selection_has_tag(view,"bold") )
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(iter),TRUE);

  g_signal_connect(iter,"clicked",
                 G_CALLBACK(bold_button_callback),view);
  gtk_toolbar_insert (bar,iter,-1);

  // Italic
  iter = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_ITALIC);
  if ( gtk_text_view_selection_has_tag(view,"italic") )
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(iter),TRUE);
    
  g_signal_connect(iter,"clicked",
                   G_CALLBACK(italic_button_callback),view);
  gtk_toolbar_insert (bar,iter,-1);

  // Strike
  iter = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_STRIKETHROUGH);
  if ( gtk_text_view_selection_has_tag(view,"strikethrough") )
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(iter),TRUE);
  g_signal_connect(iter,"clicked",
                 G_CALLBACK(strike_button_callback),view);
	gtk_toolbar_insert (bar,iter,-1);

  /* Link */
  //iter = gtk_tool_button_new_from_stock(GTK_STOCK_INDEX);
  iter = gtk_tool_button_new(get_icon("go-jump-symbolic"),NULL);
  g_signal_connect(iter,"clicked",
                   G_CALLBACK(link_callback),parent);
  gtk_toolbar_insert (bar,iter,-1);

  // second space
  iter = gtk_separator_tool_item_new(); 
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(iter),FALSE);
  gtk_tool_item_set_expand(iter,TRUE);
  gtk_toolbar_insert (bar,iter,-1);

  return result ;
}

static void
show_edit_bar(BjbNoteView *view, gboolean sticky)
{
  if ( GTK_IS_WIDGET(view->priv->edit_bar ) )
  {
    gtk_widget_destroy(view->priv->edit_bar);
  }
    
  view->priv->edit_bar = create_edit_bar(view);
  gtk_box_pack_start(GTK_BOX(view->priv->vbox),view->priv->edit_bar,
                     FALSE,FALSE,0);
  gtk_widget_show_all(view->priv->edit_bar);
  gtk_text_view_scroll_mark_onscreen(view->priv->view, 
                                     gtk_text_buffer_get_insert(
                                                gtk_text_view_get_buffer(
                                                          view->priv->view)));
}

static void
on_text_selected(GObject *toto,BjbNoteView *view)
{
  show_edit_bar(view,FALSE);
}

static gboolean
on_button_pressed(GtkWidget *widget,GdkEvent  *event,BjbNoteView *view)
{
  /* If anything else than right-click, do not break things. */
  if ( event->button.button != 3 )
  {
    return FALSE ;    
  }

  /* If right click, show toolbar and that's all */
  show_edit_bar(view,TRUE);
  return TRUE ;
  
}

static void
on_text_not_selected(GObject *toto,BjbNoteView *view)
{
  if ( view->priv->edit_bar_is_sticky == FALSE )
  {
    if ( GTK_IS_WIDGET(view->priv->edit_bar) )
    {
      gtk_widget_destroy(view->priv->edit_bar);
    }
    view->priv->edit_bar = NULL ;
  }

  else 
    view->priv->edit_bar_is_sticky = FALSE ;
}

static gboolean
on_note_renamed(BijiNoteObj *note, GtkWidget *win)
{
  gtk_window_set_title(GTK_WINDOW(win),biji_note_get_title(note));
  return TRUE ;
}

static gboolean
on_note_deleted(BijiNoteObj *note, BjbNoteView *view)
{
  // if this is the main window, show main view
  if ( view->priv->is_main_window )
  {
    just_switch_to_main_view(view);
    return TRUE ;
  }

  // else we close the window, but first remove connection to "close note"...
  // FIXME maybe, but why ?
  // g_signal_handler_disconnect(view->priv->window,view->priv->destroy);
  gtk_widget_destroy(view->priv->window);
  return TRUE ;
}

BjbNoteView*
bjb_note_view_new (GtkWidget *win,BijiNoteObj* note, gboolean is_main_window)
{
  BjbNoteView *ret ;
  BjbSettings *settings ;
  GtkWidget *vbox,*scrolled_editor;

  // view new from note deserializes the note-content.
  ret = g_object_new (BJB_TYPE_NOTE_VIEW, NULL);
  ret->priv->window = win ;
  ret->priv->current_note = note ;
  ret->priv->is_main_window = is_main_window ;
  ret->priv->view = biji_text_view_new_from_note(note);
  ret->priv->buffer = gtk_text_view_get_buffer(ret->priv->view);

  settings = bjb_window_base_get_settings(ret->priv->window);
    
  BijiNoteEditor *editor = BIJI_NOTE_EDITOR(ret->priv->view);

  ret->priv->renamed = g_signal_connect(note,"renamed",
                                        G_CALLBACK(on_note_renamed),
                                        ret->priv->window);
    
  ret->priv->deleted = g_signal_connect(note,"deleted",
                                        G_CALLBACK(on_note_deleted),ret);

  ret->priv->destroy = g_signal_connect(win,"destroy",
                                        G_CALLBACK(on_window_closed),
                                        ret->priv->current_note);

  /* Apply the selected font */ 
  gchar *font ;
  g_object_get (G_OBJECT(settings),"font",&font,NULL);
  gtk_widget_modify_font(GTK_WIDGET(ret->priv->view),
                         pango_font_description_from_string(font));
    
  /* User defined color */
  GdkRGBA *color = NULL ;
  color = biji_note_obj_get_rgba(ret->priv->current_note) ;
    
  if ( !color )
  {
	gdk_rgba_parse(color, DEFAULT_NOTE_COLOR);
    biji_note_obj_set_rgba(ret->priv->current_note,color);
  }

  ret->priv->color = color ;
  set_editor_color(ret,ret->priv->color);

  /* Padding */
  gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW(ret->priv->view),
                                        8);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(ret->priv->view),16);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  gtk_container_add(GTK_CONTAINER(ret),GTK_WIDGET(vbox));
  ret->priv->toolbars_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,0));
  gtk_container_add(GTK_CONTAINER(vbox),GTK_WIDGET(ret->priv->toolbars_box));		

  // Main Toolbar 
  gtk_box_pack_start(ret->priv->toolbars_box,create_toolbar(ret,note),TRUE,TRUE,0);

  /* If some text is selected we do show the proper bar.
   * Same for right click */ 
  g_signal_connect(editor,"selection",G_CALLBACK(on_text_selected),ret);
  g_signal_connect(editor,"button-press-event",G_CALLBACK(on_button_pressed),ret);
  g_signal_connect(editor,"button-release-event",G_CALLBACK(on_button_pressed),ret);

  // If text is no more selected we hide the bar.
  g_signal_connect(editor,"no-more-selection",
                   G_CALLBACK(on_text_not_selected),ret);

  // Infobar for template notes
  if ( note_obj_is_template(note) )
  {
    GtkWidget *template = gtk_info_bar_new();
    gchar *message = g_strdup_printf("This note is a template. All notes with \
    tag %s will contain that text.",_biji_note_template_get_tag(note));
    GtkWidget *text = gtk_label_new(message);
    gtk_box_pack_start (GTK_BOX(gtk_info_bar_get_content_area(GTK_INFO_BAR(template))),
                        text,FALSE,FALSE,0);
    gtk_box_pack_start (GTK_BOX(vbox),template,FALSE,FALSE,0);
  }
    
  /* // Infobar : No tags note. DEPRECATED.
  else if ( biji_note_obj_get_tags(note) == NULL )
  {
    GtkWidget *text;
    ret->priv->bar = gtk_info_bar_new_with_buttons(GTK_STOCK_ADD,0,NULL) ;
    gtk_info_bar_set_message_type(GTK_INFO_BAR(ret->priv->bar),
                                  GTK_MESSAGE_QUESTION);
    g_signal_connect (ret->priv->bar, "response", 
                      G_CALLBACK (on_info_bar_callback),
                      ret);
    text = gtk_label_new("This note has no tag yet.") ;
    gtk_box_pack_start (GTK_BOX (gtk_info_bar_get_content_area (GTK_INFO_BAR (ret->priv->bar))), 
                        text, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox),ret->priv->bar,FALSE,FALSE,0);
    g_signal_connect(ret->priv->view,"focus-in-event",G_CALLBACK(on_focus),ret);
  } // */

  // GtkTextView
  scrolled_editor = gtk_scrolled_window_new(NULL,NULL);
  gtk_container_add(GTK_CONTAINER(scrolled_editor),GTK_WIDGET(ret->priv->view));
  gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(scrolled_editor),TRUE,TRUE,0);

  // Edition Toolbar (only when text is selected)
  ret->priv->edit_bar = create_edit_bar(ret) ;
  gtk_box_pack_start(GTK_BOX(vbox),ret->priv->edit_bar,FALSE,FALSE,0);

  ret->priv->vbox = vbox;

  // Zeitgeist.
  insert_zeitgeist(note,ZEITGEIST_ZG_ACCESS_EVENT) ;
  

    
  return ret ;
}
