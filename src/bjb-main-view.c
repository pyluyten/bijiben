#include <glib/gprintf.h>
#include <libbiji/libbiji.h>

#include "utils/bjb-icons-colors.h"
#include "widgets/bjb-menu-tool.h"
#include "widgets/gd-main-toolbar.h"
#include "widgets/gd-main-icon-view.h"
#include "widgets/gd-main-view.h"
#include "widgets/gd-main-view-generic.h"
#include "widgets/bjb-view-mode-button.h"

#include "bjb-app-menu.h"
#include "bjb-bijiben.h"
#include "bjb-controller.h"
#include "bjb-tracker.h"
#include "bjb-main-view.h"
#include "bjb-note-view.h"
#include "bjb-tags-view.h"
#include "bjb-window-base.h"
#include "bjb-selection-panel.h"
#include "bjb-rename-note.h"


#define DEFAULT_VIEW GD_MAIN_VIEW_ICON

enum
{
  PROP_0,
  PROP_BJB_CONTROLLER,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

/************************** Gobject ***************************/

struct _BjbMainViewPriv {
  GtkWidget      *window;
  GtkWidget      *icon_view ;
  GtkWidget      *vbox;
  GtkWidget      *standard_toolbar; 

  /* TODO Properties */ 

  /* Selection TODO : use one clutter widget */
  GtkWidget      *select_toolbar;
  GtkWidget      *label;
  GtkToolItem    *finish_selection ;
  GtkWidget      *actions ;  // actions bar for selected notes.

  /* Search Entry TODO : use clutter instead */
  GtkWidget      *vbox_up ;  
  GtkWidget      *hbox_entry ;
  GtkWidget      *search_entry ;
  gboolean        has_entry ;

  /* Private */ 
  GdMainView     *view ; 
  BjbController  *controller ;

  /* signals */
  gulong notes_changed ;         // 
  
  gulong click_signal ;          // Signal to delete (icon view)
  
  gulong key_pressed ;           // a key pressed shows the entry.
                                 // FIXME : delete when search bar done.
};

G_DEFINE_TYPE (BjbMainView, bjb_main_view, GTK_TYPE_BOX);

/* TODO : make this initialization correct
 * instead bjb_main_view_new */
static void
bjb_main_view_init (BjbMainView *object)
{
  object->priv = 
  G_TYPE_INSTANCE_GET_PRIVATE(object,BJB_TYPE_MAIN_VIEW,BjbMainViewPriv);

  object->priv->window = NULL;
  object->priv->search_entry = NULL;
  object->priv->standard_toolbar = NULL ;
  object->priv->select_toolbar = NULL ;
}

static void
bjb_main_view_finalize (GObject *object)
{
  BjbMainView *view = BJB_MAIN_VIEW(object) ;

  g_signal_handler_disconnect(bjb_window_base_get_book(view->priv->window),
                              view->priv->notes_changed);

  G_OBJECT_CLASS (bjb_main_view_parent_class)->finalize (object);
}

static void
bjb_main_view_set_controller ( BjbMainView *self, BjbController *controller)
{
  self->priv->controller = controller ;
}

static void
bjb_main_view_get_property ( GObject      *object,
                             guint        prop_id,
                             GValue       *value,
                             GParamSpec   *pspec)
{
  BjbMainView *self = BJB_MAIN_VIEW (object);

  switch (prop_id)
    {
    case PROP_BJB_CONTROLLER:
      g_value_set_object (value, self->priv->controller);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
bjb_main_view_set_property ( GObject        *object,
			                 guint          prop_id,
			                 const GValue   *value,
			                 GParamSpec     *pspec)
{
  BjbMainView *self = BJB_MAIN_VIEW (object);

  switch (prop_id)
    {
    case PROP_BJB_CONTROLLER:
	  bjb_main_view_set_controller(self,g_value_get_object(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/* TODO static  void bjb_main_view_get_property */

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
  object_class->get_property = bjb_main_view_get_property;
  object_class->set_property = bjb_main_view_set_property;
  object_class->constructor = biji_main_view_constructor;
    
  g_type_class_add_private (klass, sizeof (BjbMainViewPriv));

  properties[PROP_BJB_CONTROLLER] = g_param_spec_object ("controller",
                                                         "Controller",
                                                         "BjbController",
                                                         BJB_TYPE_CONTROLLER,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_BJB_CONTROLLER,properties[PROP_BJB_CONTROLLER]);
}


/* TODO delete this */
BijiNoteBook *
bjb_main_view_get_book(BjbMainView *view)
{
  return bjb_window_base_get_book(view->priv->window);
}

static void
on_selection_mode_changed ( GtkWidget *button, BjbMainView *self)
{
    GtkWidget *actions ;
    
    if ( gd_main_view_get_selection_mode(self->priv->view))
    {
        gd_main_view_set_selection_mode ( self->priv->view , FALSE ) ;
        
        gtk_widget_destroy (self->priv->actions);

        gtk_widget_show_all(self->priv->standard_toolbar);
        gtk_widget_hide(self->priv->select_toolbar);
    }

    else
    {
        gd_main_view_set_selection_mode ( self->priv->view , TRUE ) ;
        actions = get_selection_panel (self) ;
        self->priv->actions = actions;
        gtk_widget_show_all ( actions ) ;

        hide_search_entry(self);
      
        gtk_box_pack_start(GTK_BOX(self->priv->vbox),actions,FALSE,FALSE,0);
        gtk_widget_hide(self->priv->standard_toolbar);
        gtk_widget_show_all(self->priv->select_toolbar);
        
    }

    return ;

}

static gboolean
on_view_mode_changed ( GtkWidget *button, BjbMainView *self)
{
    BjbViewModeButton *bvmb = BJB_VIEW_MODE_BUTTON ( button ) ;
    GdMainView *view = self->priv->view ;
    
    BjbViewModeType current = bjb_view_mode_button_get_selection_mode  ( bvmb ) ;
    
    switch ( current )
    {
        case BJB_VIEW_MODE_GRID :
            gd_main_view_set_view_type ( view ,GD_MAIN_VIEW_LIST );
            break ;
        default : 
            gd_main_view_set_view_type ( view ,GD_MAIN_VIEW_ICON );
            break ;
    }
    
    return FALSE ;
}

// Callbacks

void
switch_to_note_view(BjbMainView *view,BijiNoteObj *note)
{	
  /* Sanitize */
  g_signal_handler_disconnect(view->priv->window,
                              view->priv->key_pressed) ;
  view->priv->key_pressed = 0 ;
    
  GtkWidget *win = view->priv->window;
  bjb_window_base_set_frame((gpointer)win,
                             GTK_CONTAINER(bjb_note_view_new(win,note,TRUE)));
  
  gtk_window_set_title(GTK_WINDOW(win),biji_note_get_title (note));
  gtk_widget_show_all(win);
}



static void
clear_search_entry_callback(GtkEntry *entry,GtkEntryIconPosition icon_pos,
                            GdkEvent *event,BjbMainView *self)
{

    gtk_entry_set_text(entry,"");
    bjb_controller_set_needle(self->priv->controller,"");

}

static void
action_search_entry(GtkEntry *entry,BjbMainView *self)
{
  gchar *search = g_strdup(gtk_entry_get_text(entry));
  bjb_controller_set_needle ( self->priv->controller, search ) ;
    
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

  /* append note to collection */
  book = bjb_window_base_get_book(view->priv->window);
  gchar *folder = g_strdup_printf("%s/bijiben",g_get_user_data_dir());
  result = biji_note_get_new_from_string(title,folder);
  g_free(folder);
  note_book_append_new_note(book,result);

  /* Go to that note */
  switch_to_note_view(view,result);
  note_obj_save_note_using_buffer((gpointer)result);
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
                              (bjb_window_base_get_app(view->priv->window)));
    g_list_foreach (notes,(GFunc)show_window_if_title_same,to_open);
    return ;
  }

  // Otherwise, leave main view to show this note into current window.
  switch_to_note_view(view,to_open);
}

static void
update_selection_label(BjbMainView *bmv)
{
    gint selected = 0 ;
    GdMainView *view = bmv->priv->view ;

    if ( view )
    {
       selected = g_list_length (gd_main_view_get_selection(view)) ;
       
       if ( selected < 1 )
       {
            gtk_label_set_text(GTK_LABEL(bmv->priv->label),
                               "Click on Notes to Select");

            return ;
       }
    }

    /* No note selected or no gd main view yet */
    gchar *text  = g_strdup_printf("%i Notes Selected",selected);
    gtk_label_set_text(GTK_LABEL(bmv->priv->label),text);    
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

  /* switch view button */
  iter = GTK_TOOL_ITEM ( bjb_view_mode_button_new (BJB_VIEW_MODE_LIST) );
  g_signal_connect ( iter, "clicked",
                    G_CALLBACK(on_view_mode_changed),parent);
  gtk_toolbar_insert(tool,iter,-1); 
	
  /* select button */
  GtkWidget *check = get_icon("emblem-default-symbolic") ;
  iter = gtk_tool_button_new(check,NULL);
  g_signal_connect(iter,"clicked",
                   G_CALLBACK(on_selection_mode_changed),parent);
  gtk_toolbar_insert (tool,iter,-1); 
	
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
  update_selection_label(parent);
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
get_search_entry(BjbMainView *self)
{
  BjbMainViewPriv *priv ;
  BjbController *controller ;
  gchar *needle ;
  GtkEntry *entry ;
  GtkEntryCompletion *completion ;
  GtkTreeModel *completion_model ;
  GtkEntryBuffer *  entry_buf ;
  
  priv = self->priv ;
  controller = bjb_window_base_get_controller(BJB_WINDOW_BASE(priv->window));

  needle = bjb_controller_get_needle(controller);
  	
  priv->search_entry = gtk_entry_new ();  
  entry = GTK_ENTRY(priv->search_entry);
  
  g_message("1. NEEDLE IS :%s",needle);
  
  if ( needle != NULL )
  { 
    g_message("2. NEEDLE IS :%s",needle);
    priv->has_entry = TRUE ;
    gtk_entry_set_text(entry,needle);
  } 

  gtk_entry_set_icon_from_stock (entry,
                                 GTK_ENTRY_ICON_SECONDARY,
                                 GTK_STOCK_CLEAR);

  completion = gtk_entry_completion_new ();
  gtk_entry_set_completion (entry, completion);
  g_object_unref (completion);
  completion_model = create_completion_model ();
  gtk_entry_completion_set_model (completion, completion_model);
  g_object_unref (completion_model);
  gtk_entry_completion_set_text_column (completion, 0);
		
  g_signal_connect (priv->search_entry, "icon-press",
	                  G_CALLBACK (clear_search_entry_callback),self);

  entry_buf = gtk_entry_get_buffer(entry);
  g_signal_connect(entry_buf,"inserted-text",
	               G_CALLBACK(action_entry_insert_callback),self);
  g_signal_connect(entry_buf,"deleted-text",
	               G_CALLBACK(action_entry_delete_callback),self);	
		
  return priv->search_entry ;
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

void prepare_view_for_usage(BjbMainView *self)
{
  BjbMainViewPriv *priv ;
  
  priv = self->priv ;
	
  /* First show all, then exceptions */
  gtk_widget_show_all(priv->window);
  
  /* TODO : test if selection mode  */
  gtk_widget_hide(priv->select_toolbar);
  
  /* Handle the search entry showing */
  if ( !self->priv->has_entry )
  {
    gtk_widget_hide( self->priv->search_entry);
  }
  
  else 
  {
    gtk_widget_grab_focus(GTK_WIDGET(self->priv->search_entry));
    gtk_editable_set_position (GTK_EDITABLE(self->priv->search_entry),-1);
  }
}

static gboolean
on_key_pressed(GtkWidget *widget,GdkEvent  *event,gpointer user_data)
{
  BjbMainView *view = BJB_MAIN_VIEW (user_data);

  /* Do not allow search when selecting items */
  if ( gd_main_view_get_selection_mode(view->priv->view)==TRUE )
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
on_book_changed(BijiNoteBook *book, BjbMainView *self )
{
  refresh_notes_model(self->priv->controller);
  return TRUE ;
}

static GList *
get_selected_paths(BjbMainView *self)
{
    return gd_main_view_get_selection ( self->priv->view ) ; 
}

static gchar *
get_note_url_from_tree_path(GtkTreePath *path, BjbMainView *self)
{
  GtkTreeIter iter ;
  gchar *note_path ;
  GtkTreeModel *model ;

  model = bjb_controller_get_model(self->priv->controller);  
  gtk_tree_model_get_iter (model,&iter, path);
  gtk_tree_model_get (model, &iter,GD_MAIN_COLUMN_URI, &note_path,-1);

  return note_path ;
}

void action_delete_selected_notes(GtkWidget *w,BjbMainView *view)
{
  GList *notes = NULL ;
  gint i ; 

  /*  GtkTreePath */
  GList *paths = get_selected_paths(view);
    
  for ( i=0 ;  i < g_list_length (paths) ; i++ )
  {
    gchar *url = get_note_url_from_tree_path(g_list_nth_data(paths,i),
                                             view) ;
      
    notes = g_list_append(notes,
                          note_book_get_note_at_path
                          (bjb_window_base_get_book(view->priv->window),url));
    
  } 

  for (i=0 ; i<g_list_length(notes) ;i++ )
  {
    BijiNoteObj *note = g_list_nth_data(notes,i) ;
      
    biji_note_delete_from_tracker(note);
    biji_note_book_remove_note(bjb_window_base_get_book(view->priv->window),note);
  }
}

static gboolean
on_item_activated(GdMainView        * gd, 
                  const gchar       * id,
                  const GtkTreePath * path,
                  BjbMainView       * view)
{
  BijiNoteBook * book ;
  BijiNoteObj  * to_open ;
  GtkTreeIter    iter ;
  gchar        * note_path ;
  GtkTreeModel * model ;

  /* Get Note Path */
  model = gd_main_view_get_model(gd);
  gtk_tree_model_get_iter (model,&iter, (GtkTreePath*) path);
  gtk_tree_model_get (model, &iter,COL_URI, &note_path,-1);

  /* Switch to that note */
  book = bjb_window_base_get_book(view->priv->window); 
  to_open = note_book_get_note_at_path(book,note_path) ;
  switch_to_note(view,to_open); 

  return FALSE ;
}

BjbMainView*
bjb_main_view_new(GtkWidget *win,
                  BijiNoteBook *book,
                  BjbController *controller)
{
  BjbMainView *self ;
  GtkWidget *vbox; 
    
  self = g_object_new( BJB_TYPE_MAIN_VIEW,
                       "controller",controller,
                       NULL);

  self->priv->window = win ;
    
  self->priv->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  self->priv->vbox_up = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

  vbox = self->priv->vbox;
  gtk_box_pack_start(GTK_BOX(self),GTK_WIDGET(vbox),TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(vbox),self->priv->vbox_up,FALSE,FALSE,0);

  /* FIXME : clutter toolbar. */
  self->priv->standard_toolbar = create_standard_toolbar(self);
  gtk_box_pack_start(GTK_BOX(self->priv->vbox_up),self->priv->standard_toolbar,
                     FALSE,FALSE,0) ;

  /* FIXME : clutter toolbar. */
  self->priv->select_toolbar = create_selection_toolbar(self);
  gtk_box_pack_start(GTK_BOX(self->priv->vbox_up),self->priv->select_toolbar,
                     FALSE,FALSE,0) ;

  // Search entry is inside vbox up for test
  self->priv->has_entry = FALSE ;
  get_search_entry(self);
  self->priv->key_pressed = g_signal_connect(self->priv->window,"key-press-event",
                                            G_CALLBACK(on_key_pressed),self); 
  self->priv->hbox_entry = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL,0);  
    

  gtk_box_pack_start (GTK_BOX(self->priv->hbox_entry),
                      self->priv->search_entry,
                      TRUE,FALSE,0) ;
                      
  gtk_box_pack_start (GTK_BOX(self->priv->vbox_up),
                      self->priv->hbox_entry,
                      FALSE,FALSE,0) ;

    /* Controller to display notes 
    
    controller = bjb_controller_new(book ,
                                    bjb_window_base_get_entry(self->priv->window)) ;
    self->priv->controller = controller ;*/

    self->priv->view = gd_main_view_new(DEFAULT_VIEW);
    gtk_box_pack_start(GTK_BOX(vbox),
                       GTK_WIDGET(self->priv->view),
                       TRUE,
                       TRUE,
                       0);

    gd_main_view_set_selection_mode ( self->priv->view, FALSE);
    gd_main_view_set_model(self->priv->view,
                           bjb_controller_get_model(controller));

    g_signal_connect(self->priv->view,"item-activated",
                     G_CALLBACK(on_item_activated),self);
 
    gtk_window_set_title (GTK_WINDOW (win), BIJIBEN_MAIN_WIN_TITLE);

    self->priv->notes_changed = g_signal_connect(book,"changed",
                                        G_CALLBACK(on_book_changed),self);
                                                                          
    return self;
}

GtkWidget *
bjb_main_view_get_window(BjbMainView *view)
{
    return view->priv->window ;
}
