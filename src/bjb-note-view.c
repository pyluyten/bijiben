#include <gtk/gtk.h>
#include <libbiji/libbiji.h>

#include "utils/bjb-icons-colors.h"
#include "widgets/gd-main-toolbar.h"

#include "bjb-bijiben.h"
#include "bjb-editor-toolbar.h"
#include "bjb-rename-note.h"
#include "bjb-share.h"
#include "bjb-tracker.h"
#include "bjb-main-view.h"
#include "bjb-note-view.h"
#include "bjb-window-base.h"
#include "bjb-zeitgeist-dataprovider.h"

/* Default color (X11 rgb.txt) - maybe gsettings instead */ 
#define DEFAULT_NOTE_COLOR "LightGoldenrodYellow"

enum
{
  PROP_0,
  PROP_WINDOW,
  PROP_NOTE,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

G_DEFINE_TYPE (BjbNoteView, bjb_note_view, CLUTTER_TYPE_ACTOR)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BJB_TYPE_NOTE_VIEW, BjbNoteViewPrivate))

struct _BjbNoteViewPrivate {
  /* Data */
  GtkWidget         *window ;
  BijiNoteObj       *note ;
  GtkTextBuffer     *buffer ;

  /* UI */
  ClutterActor      *embed;
  ClutterActor      *edit_actor;
  GtkBox            *toolbars_box;
  GtkTextView       *view;
  BjbEditorToolbar  *edit_bar;
  ClutterActor      *edit_bar_actor;
  gboolean           edit_bar_is_sticky ;
  ClutterActor      *last_update;

  /* Manage tags dialog */
  GtkWidget         *tags_dialog;
  GtkWidget         *entry ;             // text entry to input new tags
  GtkTreeModel      *model;              //                    tag model

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

  /* Don't unref buffer. Biji Does it when we close note. */
  g_signal_handler_disconnect(view->priv->note,view->priv->renamed);
  g_signal_handler_disconnect(view->priv->window,view->priv->destroy);
  g_signal_handler_disconnect(view->priv->note,view->priv->deleted);

  /* TODO */

  G_OBJECT_CLASS (bjb_note_view_parent_class)->finalize (object);
}

static void
bjb_note_view_get_property (GObject      *object,
                            guint        prop_id,
                            GValue       *value,
                            GParamSpec   *pspec)
{
  BjbNoteView *self = BJB_NOTE_VIEW (object);

  switch (prop_id)
  {
    case PROP_WINDOW:
      g_value_set_object (value, self->priv->window);
      break;
    case PROP_NOTE:
      g_value_set_object (value, self->priv->note);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
bjb_note_view_set_property ( GObject        *object,
                             guint          prop_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
  BjbNoteView *self = BJB_NOTE_VIEW (object);

  switch (prop_id)
  {
    case PROP_WINDOW:
      self->priv->window = g_value_get_object(value);
      break;
    case PROP_NOTE:
      self->priv->note = g_value_get_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
bjb_note_view_init (BjbNoteView *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, BJB_TYPE_NOTE_VIEW,
                                            BjbNoteViewPrivate);

  self->priv->embed = clutter_actor_new ();
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
                        biji_note_obj_has_tag(view->priv->note,tag),
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
    biji_note_obj_add_tag(view->priv->note,tag);
    note_obj_save_note_using_buffer(view->priv->note);
    push_existing_tag_to_note(tag,view->priv->note);
                          
    // and update the cell
    toggle_item = TRUE ;
  }
  else
  {
    // Remove the tag as in libiji, also remove tracker tag
    biji_note_obj_remove_tag (view->priv->note,tag);
    note_obj_save_note_using_buffer (view->priv->note);
    remove_tag_from_note (tag,view->priv->note);

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
  bijiben_push_note_to_tracker(view->priv->note);
    
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

/* Callbacks */

static void
just_switch_to_main_view(BjbNoteView *self)
{
  GtkWindow     *window;
  BjbController *controller;

  clutter_actor_destroy(self->priv->embed);

  window = GTK_WINDOW(self->priv->window);
  controller = bjb_window_base_get_controller(BJB_WINDOW_BASE(window));

  bjb_main_view_new((gpointer)window,controller);
}

static void
save_then_switch_to_notes_view(BjbNoteView *view)
{
  bijiben_push_note_to_tracker(view->priv->note);
  bjb_close_note(view->priv->note);
  just_switch_to_main_view(view);
}

static void
action_switch_to_notes_callback(GtkButton *but,BjbNoteView *view)
{
  save_then_switch_to_notes_view(view);
}

static void
action_view_tags_callback (GtkWidget *item, gpointer user_data)
{
  show_tags_dialog(BJB_NOTE_VIEW(user_data));
}

static void
action_rename_note_callback (GtkWidget *item, gpointer user_data)
{
  BjbNoteView        *view;
  BjbNoteViewPrivate *priv;
  gchar              *title;
  
  view = BJB_NOTE_VIEW (user_data);
  priv = view->priv;

  title = note_title_dialog(GTK_WINDOW(priv->window),"Rename Note",
                            biji_note_get_title(priv->note));

  if (!title)
    return ;

  set_note_title (priv->note,title);
  gtk_window_set_title (GTK_WINDOW(priv->window),title);
}

static void
delete_item_callback (GtkWidget *item, gpointer user_data)
{
  BjbNoteView *view = BJB_NOTE_VIEW (user_data);

  biji_note_delete_from_tracker(view->priv->note);

  /* Delete the note from collection
   * The deleted note will emit a signal. */
  biji_note_book_remove_note(bjb_window_base_get_book(view->priv->window),
                             view->priv->note);
}

static void
set_editor_color(BjbNoteView *v, GdkRGBA *to_be)
{
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
    set_editor_color (view,color);
    biji_note_obj_set_rgba (view->priv->note,color) ;
  }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
on_color_clicked(GtkWidget *but,BjbNoteView *view)
{
  GtkWidget *dialog;
    
  dialog = gtk_color_chooser_dialog_new ("Choose note background",
                                         GTK_WINDOW(view->priv->window));  

  g_signal_connect (dialog,"response",
                    G_CALLBACK (on_color_choosed),view); 

  gtk_widget_show_all(dialog);
}

static gboolean
color_key_pressed_event (GtkWidget *widget,
                         GdkEvent  *event,
                         gpointer   user_data)
{
  on_color_clicked (widget, user_data);

  return TRUE;
}

/* Note View Toolbar */

/* Draw the color button */
static gboolean
on_color_draw(GtkWidget *widget, cairo_t *cr, BijiNoteObj *note)
{
  cairo_rectangle  (cr,1,1,100,100);  

  GdkRGBA *color = biji_note_obj_get_rgba(note) ;

  if ( !color )
    gdk_rgba_parse (color, DEFAULT_NOTE_COLOR );

  gdk_cairo_set_source_rgba (cr, biji_note_obj_get_rgba(note));
  cairo_fill (cr);
  return TRUE;
}

GtkWidget *
bjb_note_menu_new (BjbNoteView *self)
{
  GtkWidget   *result, *item;
  
  result = gtk_menu_new();

  /* Rename */
  item = gtk_menu_item_new_with_label("Rename");
  gtk_menu_shell_append(GTK_MENU_SHELL(result),item);
  g_signal_connect(item,"activate",
                   G_CALLBACK(action_rename_note_callback),self); 
  gtk_widget_show(item);
  
  /* View Tags */
  item = gtk_menu_item_new_with_label("Tags");
  gtk_menu_shell_append(GTK_MENU_SHELL(result),item);
  g_signal_connect(item,"activate",
                   G_CALLBACK(action_view_tags_callback),self);
  gtk_widget_show(item);

  /* Delete Note */
  item = gtk_menu_item_new_with_label("Delete this note");
  gtk_menu_shell_append(GTK_MENU_SHELL(result),item);
  g_signal_connect(item,"activate",
                   G_CALLBACK(delete_item_callback),self);
  gtk_widget_show(item);

  return result;
}

static ClutterActor *
bjb_note_main_toolbar_new (BjbNoteView *self,
                           ClutterActor *parent,
                           BijiNoteObj *note)
{
  GdMainToolbar    *gd;
  ClutterActor     *result;
  GtkWidget        *w,*button;
  
  GtkWidget        *grid,*notes_label,*notes_icon;

  GtkWidget        *color_square,*color;
  GtkSizeGroup     *size;

  w = gd_main_toolbar_new();
  gd = GD_MAIN_TOOLBAR(w);

  result = gtk_clutter_actor_new_with_contents(w);
  clutter_actor_add_child(parent,result);
  gtk_widget_show_all(w);
  clutter_actor_set_x_expand(result,TRUE);

  /* Go to main view */
  grid = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
  notes_icon = get_icon("go-previous-symbolic");
  gtk_box_pack_start(GTK_BOX(grid),notes_icon,TRUE,TRUE,TRUE);
  notes_label = gtk_label_new("Notes");
  gtk_box_pack_start(GTK_BOX(grid),notes_label,TRUE,TRUE,TRUE);

  button = gd_main_toolbar_add_button (gd, NULL, NULL, TRUE);

  gtk_container_add(GTK_CONTAINER(button),grid);
  gtk_widget_show_all(button);
  gtk_widget_set_vexpand (button, TRUE);
  g_signal_connect (button,"clicked",G_CALLBACK(action_switch_to_notes_callback),self);

  /* Note title */
  gd_main_toolbar_set_labels (gd,biji_note_get_title(note),NULL);

  /* Note Color *
   * the size requisition is done later on
   * because it needs another widget */
  color_square = gd_main_toolbar_add_button (gd, NULL, NULL, FALSE);

  color = gtk_drawing_area_new();
  gtk_widget_set_hexpand (color, TRUE);
  gtk_widget_set_vexpand (color, TRUE);
  gtk_container_add (GTK_CONTAINER(color_square), GTK_WIDGET (color));

  gtk_widget_set_vexpand (color_square, TRUE);
  gtk_widget_set_hexpand (color_square, TRUE);
  gtk_widget_show_all (color_square);

  g_signal_connect (color,"draw",G_CALLBACK(on_color_draw),note);

  /* GdkDrawingArea does not have "clicked" signal */
  gtk_widget_add_events (color, GDK_BUTTON_PRESS_MASK);
  g_signal_connect (color,"button-press-event",G_CALLBACK(color_key_pressed_event),self);

  /* Sharing */
  button = gd_main_toolbar_add_button (gd, "send-to-symbolic",
                                       NULL, FALSE);

  g_signal_connect(button,"clicked",
                   G_CALLBACK(on_email_note_callback),note);

  /* Menu */
  button = gd_main_toolbar_add_menu(gd,"emblem-system-symbolic",NULL,FALSE);

  gtk_menu_button_set_menu (GTK_MENU_BUTTON (button),
                            bjb_note_menu_new (self));

  /* Fix the h size for color square since gdk drawing area */
  size = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  gtk_size_group_add_widget (size, button);
  gtk_size_group_add_widget (size, color_square);
  g_object_unref (size);

  return result;
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
  just_switch_to_main_view (view);
  return TRUE;
}

/* Number of days since last updated
 * Instead we might want to play with a func to have a date
 * Also this might be integrated in text view */
ClutterActor *
bjb_note_view_last_updated_actor_new (BjbNoteView *self)
{
  ClutterActor *result, *last, *value;
  ClutterLayoutManager *layout;
  ClutterColor last_up_col = {122,122,122,255};
  gchar *last_updated_str;

  result = clutter_actor_new ();
  layout = clutter_box_layout_new ();
  clutter_actor_set_layout_manager (result, layout);

  last = clutter_text_new ();
  clutter_text_set_text (CLUTTER_TEXT (last), "Last updated ");
  clutter_text_set_font_name (CLUTTER_TEXT (last), "Arial 9px");
  clutter_text_set_color (CLUTTER_TEXT (last), &last_up_col );
  clutter_actor_add_child (result, last);

  value = clutter_text_new ();
  last_updated_str = biji_note_obj_get_last_change_date_string (
                                                      self->priv->note);
  clutter_text_set_text (CLUTTER_TEXT (value), last_updated_str);
  clutter_text_set_font_name (CLUTTER_TEXT (value), "Arial 9px");
  clutter_actor_add_child (result, value);

  clutter_actor_show (result);
  return result ;
}

static void
bjb_note_view_constructed (GObject *obj)
{
  BjbNoteView            *self = BJB_NOTE_VIEW (obj);
  BjbNoteViewPrivate     *priv = self->priv;
  BijiNoteEditor         *editor ;
  BjbSettings            *settings;
  GtkWidget              *scroll;
  ClutterActor           *stage, *vbox;
  ClutterConstraint      *constraint;
  ClutterLayoutManager   *full, *box, *bin;
  gchar                  *font;

  /* view new from note deserializes the note-content. */
  priv->view = biji_text_view_new_from_note (priv->note);
  priv->buffer = gtk_text_view_get_buffer (priv->view);

  settings = bjb_window_base_get_settings(priv->window);
    
  editor = BIJI_NOTE_EDITOR (priv->view);

  priv->renamed = g_signal_connect(priv->note,"renamed",
                                   G_CALLBACK(on_note_renamed),
                                   priv->window);
    
  priv->deleted = g_signal_connect(priv->note,"deleted",
                                   G_CALLBACK(on_note_deleted),self);

  priv->destroy = g_signal_connect(priv->window,"destroy",
                                   G_CALLBACK(on_window_closed),
                                   priv->note);

  /* Start packing ui */
  stage = bjb_window_base_get_stage (BJB_WINDOW_BASE (priv->window));

  full = clutter_bin_layout_new (CLUTTER_BIN_ALIGNMENT_CENTER,
                                 CLUTTER_BIN_ALIGNMENT_CENTER);

  clutter_actor_set_layout_manager (priv->embed,full);
  clutter_actor_add_child (stage,priv->embed);

  constraint = clutter_bind_constraint_new (stage, CLUTTER_BIND_SIZE, 0.0);
  clutter_actor_add_constraint (priv->embed, constraint);

  vbox = clutter_actor_new();
  box = clutter_box_layout_new();
  clutter_box_layout_set_orientation(CLUTTER_BOX_LAYOUT(box),
                                     CLUTTER_ORIENTATION_VERTICAL);

  clutter_actor_set_layout_manager (vbox,box);
  clutter_actor_add_child (priv->embed,vbox);

  /* Main Toolbar  */
  bjb_note_main_toolbar_new (self, vbox, priv->note);

  /* Overlay contains Text and EditToolbar */
  ClutterActor *overlay = clutter_actor_new ();
  bin = clutter_bin_layout_new (CLUTTER_BIN_ALIGNMENT_CENTER,
                                CLUTTER_BIN_ALIGNMENT_CENTER);

  clutter_actor_set_layout_manager (overlay,bin);
  clutter_actor_add_child (vbox,overlay);
  clutter_actor_set_x_expand (overlay,TRUE);
  clutter_actor_set_y_expand (overlay,TRUE);

  /* GtkTextView */
  scroll = gtk_scrolled_window_new (NULL,NULL);
  gtk_widget_show (scroll);

  gtk_widget_set_hexpand (scroll, TRUE);
  gtk_widget_set_vexpand (scroll, TRUE);
  
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll),
                                       GTK_SHADOW_IN);
                                       
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                  GTK_POLICY_NEVER,
                                  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (scroll), GTK_WIDGET(priv->view));
  gtk_widget_show (GTK_WIDGET (priv->view));

  ClutterActor *text_actor = gtk_clutter_actor_new_with_contents (scroll);
  clutter_actor_add_child(overlay,text_actor);

  clutter_actor_set_x_expand(text_actor,TRUE);
  clutter_actor_set_y_expand(text_actor,TRUE);

  /* Apply the selected font */ 
  g_object_get (G_OBJECT(settings),"font",&font,NULL);
  gtk_widget_modify_font(GTK_WIDGET(priv->view),
                         pango_font_description_from_string(font));

  /* Padding */
  gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW(priv->view),
                                        8);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(priv->view),16);

  /* User defined color */
  GdkRGBA *color = NULL ;
  color = biji_note_obj_get_rgba(priv->note) ;
    
  if ( !color )
  {
    gdk_rgba_parse (color, DEFAULT_NOTE_COLOR);
    biji_note_obj_set_rgba (priv->note,color);
  }

  self->priv->color = color ;
  set_editor_color (self,priv->color);

  /* Edition Toolbar */
  priv->edit_bar = bjb_editor_toolbar_new (overlay, self, editor);
  priv->edit_bar_actor = bjb_editor_toolbar_get_actor (priv->edit_bar);
  clutter_actor_add_child (priv->embed, priv->edit_bar_actor);

  /* Last updated row */
  priv->last_update = bjb_note_view_last_updated_actor_new (self);
  clutter_actor_add_child (priv->embed,priv->last_update);
                           
  constraint = clutter_align_constraint_new (priv->embed,CLUTTER_ALIGN_X_AXIS,0.05);
  clutter_actor_add_constraint (priv->last_update, constraint);

  constraint = clutter_align_constraint_new (priv->embed,CLUTTER_ALIGN_Y_AXIS,0.95);
  clutter_actor_add_constraint (priv->last_update, constraint);

  /* Show & let's go */
  gtk_widget_show_all (priv->window);

  // Zeitgeist.
  insert_zeitgeist (priv->note,ZEITGEIST_ZG_ACCESS_EVENT) ;
}

BjbNoteView *
bjb_note_view_new (GtkWidget *win, BijiNoteObj* note)
{
  return g_object_new (BJB_TYPE_NOTE_VIEW,
                       "window",win,
                       "note",note,
                       NULL);
}

static void
bjb_note_view_class_init (BjbNoteViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
    
  object_class->finalize = bjb_note_view_finalize;
  object_class->constructed = bjb_note_view_constructed;
  object_class->get_property = bjb_note_view_get_property;
  object_class->set_property = bjb_note_view_set_property;

  g_type_class_add_private (klass, sizeof (BjbNoteViewPrivate));

  properties[PROP_WINDOW] = g_param_spec_object ("window",
                                                 "Window",
                                                 "Parent Window",
                                                 GTK_TYPE_WIDGET,
                                                 G_PARAM_READWRITE |
                                                 G_PARAM_CONSTRUCT |
                                                 G_PARAM_STATIC_STRINGS);
                                                 
  g_object_class_install_property (object_class,PROP_WINDOW,properties[PROP_WINDOW]);

  properties[PROP_NOTE] = g_param_spec_object ("note",
                                               "Note",
                                               "Note",
                                               BIJI_TYPE_NOTE_OBJ,
                                               G_PARAM_READWRITE |
                                               G_PARAM_CONSTRUCT |
                                               G_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class,PROP_NOTE,properties[PROP_NOTE]);
}

ClutterActor *
bjb_note_view_get_actor ( BjbNoteView *v)
{
  return v->priv->embed ;
}

GtkWidget *
bjb_note_view_get_base_window (BjbNoteView *v)
{
  return v->priv->window;
}
