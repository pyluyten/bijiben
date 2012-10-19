/* bjb-note-view.c
 * Copyright (C) Pierre-Yves LUYTEN 2012 <py@luyten.fr>
 * 
 * bijiben is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * bijiben is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <libbiji/libbiji.h>
#include <libgd/gd.h>

#include "utils/bjb-icons-colors.h"

#include "bjb-bijiben.h"
#include "bjb-editor-toolbar.h"
#include "bjb-rename-note.h"
#include "bjb-share.h"
#include "bjb-main-view.h"
#include "bjb-note-tag-dialog.h"
#include "bjb-note-view.h"
#include "bjb-window-base.h"

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
  GtkWidget         *view;
  BijiNoteObj       *note ;

  /* UI */
  ClutterActor      *embed;
  GtkAccelGroup     *accel;
  ClutterActor      *edit_actor;
  GtkBox            *toolbars_box;
  BjbEditorToolbar  *edit_bar;
  ClutterActor      *edit_bar_actor;
  gboolean           edit_bar_is_sticky ;
  ClutterActor      *last_update;

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
  BjbNoteView *self = BJB_NOTE_VIEW (object) ;
  BjbNoteViewPrivate *priv = self->priv;

  g_signal_handler_disconnect (priv->note, priv->renamed);
  g_signal_handler_disconnect (priv->window, priv->destroy);
  g_signal_handler_disconnect (priv->note, priv->deleted);

  clutter_actor_destroy (priv->embed);
  g_clear_object (&priv->accel);
  g_clear_object (&priv->edit_bar);
  /* TODO : check if the editor has been destroyed */

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
  self->priv->accel = gtk_accel_group_new ();
}

static void
on_window_closed(GtkWidget *window,gpointer note)
{
  if ( window == NULL )
  {
    /* Note is deleted */
  }
}

/* Callbacks */

static void
just_switch_to_main_view(BjbNoteView *self)
{
  GtkWindow     *window;
  BjbController *controller;

  window = GTK_WINDOW(self->priv->window);
  gtk_window_remove_accel_group (window, self->priv->accel);
  controller = bjb_window_base_get_controller(BJB_WINDOW_BASE(window));

  g_clear_object (&self);
  bjb_main_view_new ((gpointer) window,controller);
}

static void
action_view_tags_callback (GtkWidget *item, gpointer user_data)
{
  BjbNoteView *self = BJB_NOTE_VIEW (user_data);
  GtkWindow *window = GTK_WINDOW (self->priv->window);

  GList *note = NULL;
  note = g_list_append (note, self->priv->note);

  bjb_note_tag_dialog_new (window, note);
  g_list_free (note);
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

  biji_note_obj_set_title (priv->note,title);
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
on_color_set(GtkColorButton *button,
             BjbNoteView *view)
{
  GdkRGBA color;

  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (button), &color);
  biji_note_obj_set_rgba (view->priv->note,&color) ;
}

/* just a bridge between note menu & webkit func */
static void
bjb_toggle_bullets (BijiWebkitEditor *editor)
{
  biji_webkit_editor_apply_format (editor, BIJI_BULLET_LIST);
}

static void
bjb_toggle_list (BijiWebkitEditor *editor)
{
  biji_webkit_editor_apply_format (editor, BIJI_ORDER_LIST);
}


GtkWidget *
bjb_note_menu_new (BjbNoteView *self)
{
  BjbNoteViewPrivate *priv = self->priv;
  GtkWidget   *result, *item;
  BijiWebkitEditor *editor;

  result = gtk_menu_new();
  editor = BIJI_WEBKIT_EDITOR (biji_note_obj_get_editor (priv->note));

  /* Undo Redo separator */
  item = gtk_menu_item_new_with_label ("Undo");
  gtk_menu_shell_append (GTK_MENU_SHELL (result), item);
  g_signal_connect_swapped (item, "activate",
                            G_CALLBACK (webkit_web_view_undo), editor);
  gtk_widget_add_accelerator (item, "activate", priv->accel, GDK_KEY_u,
                             GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (item);

  item = gtk_menu_item_new_with_label ("Redo");
  gtk_menu_shell_append (GTK_MENU_SHELL (result), item);
  g_signal_connect_swapped (item, "activate",
                            G_CALLBACK (webkit_web_view_redo), editor);
  gtk_widget_add_accelerator (item, "activate", priv->accel, GDK_KEY_r,
                             GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (item);

  item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (result), item);
  gtk_widget_show (item);

  /* Bullets, ordered list, separator */
  item = gtk_menu_item_new_with_label ("Bullets");
  gtk_menu_shell_append (GTK_MENU_SHELL (result), item);
  g_signal_connect_swapped (item, "activate",
                            G_CALLBACK (bjb_toggle_bullets), editor); 
  gtk_widget_show(item);

  item = gtk_menu_item_new_with_label ("List");
  gtk_menu_shell_append (GTK_MENU_SHELL (result), item);
  g_signal_connect_swapped (item, "activate",
                            G_CALLBACK (bjb_toggle_list), editor); 
  gtk_widget_show(item);

  item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (result), item);
  gtk_widget_show (item);

  /* Rename, view tags, separtor */
  item = gtk_menu_item_new_with_label("Rename");
  gtk_menu_shell_append(GTK_MENU_SHELL(result),item);
  g_signal_connect(item,"activate",
                   G_CALLBACK(action_rename_note_callback),self); 
  gtk_widget_show(item);

  item = gtk_menu_item_new_with_label("Tags");
  gtk_menu_shell_append(GTK_MENU_SHELL(result),item);
  g_signal_connect(item,"activate",
                   G_CALLBACK(action_view_tags_callback),self);
  gtk_widget_show(item);

  item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (result), item);
  gtk_widget_show (item);

  /* Delete Note
   * TODO : note is currently NOT backuped!!! */
  item = gtk_menu_item_new_with_label("Delete this note");
  gtk_menu_shell_append(GTK_MENU_SHELL(result),item);
  g_signal_connect(item,"activate",
                   G_CALLBACK(delete_item_callback),self);
  gtk_widget_show(item);

  return result;
}

#define COLOR_SIZE 24

static void
on_note_renamed (BijiNoteObj *note, GdMainToolbar *bar)
{
  gd_main_toolbar_set_labels (bar, biji_note_get_title (note), NULL);
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

  GtkWidget        *color_button;
  GdkRGBA           color;

  w = gd_main_toolbar_new();
  gd = GD_MAIN_TOOLBAR(w);

  result = gtk_clutter_actor_new_with_contents(w);
  clutter_actor_add_child(parent,result);
  gtk_widget_show_all(w);
  clutter_actor_set_x_expand(result,TRUE);

  /* Go to main view basically means closing note */
  grid = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
  notes_icon = get_icon("go-previous-symbolic");
  gtk_box_pack_start(GTK_BOX(grid),notes_icon,TRUE,TRUE,TRUE);
  notes_label = gtk_label_new("Notes");
  gtk_box_pack_start(GTK_BOX(grid),notes_label,TRUE,TRUE,TRUE);

  button = gd_main_toolbar_add_button (gd, NULL, NULL, TRUE);

  gtk_container_add(GTK_CONTAINER(button),grid);
  gtk_widget_show_all(button);
  gtk_widget_set_vexpand (button, TRUE);
  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK(just_switch_to_main_view),self);
  gtk_widget_add_accelerator (button, "activate", self->priv->accel,
                              GDK_KEY_w, GDK_CONTROL_MASK, GTK_ACCEL_MASK);

  /* Note title */
  gd_main_toolbar_set_labels (gd,biji_note_get_title(note),NULL);

  self->priv->renamed = g_signal_connect(note,"renamed",
                                        G_CALLBACK(on_note_renamed), w);

  /* Note Color */
  if (!biji_note_obj_get_rgba (note, &color))
    gdk_rgba_parse (&color, DEFAULT_NOTE_COLOR );

  color_button = gtk_color_button_new ();
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (color_button), &color);

  gd_main_toolbar_add_widget (gd, color_button, FALSE);
  gtk_widget_set_size_request (gtk_bin_get_child (GTK_BIN (color_button)),
                               COLOR_SIZE, COLOR_SIZE);
  gtk_widget_show (color_button);

  g_signal_connect (color_button,"color-set",
                    G_CALLBACK(on_color_set),self);

  /* Sharing */
  button = gd_main_toolbar_add_button (gd, "send-to-symbolic",
                                       NULL, FALSE);

  g_signal_connect(button,"clicked",
                   G_CALLBACK(on_email_note_callback),note);

  /* Menu */
  button = gd_main_toolbar_add_menu(gd,"emblem-system-symbolic",NULL,FALSE);

  gtk_menu_button_set_popup (GTK_MENU_BUTTON (button),
                             bjb_note_menu_new (self));

  return result;
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
  BjbSettings            *settings;
  GtkWidget              *scroll;
  ClutterActor           *stage, *vbox;
  ClutterConstraint      *constraint;
  ClutterLayoutManager   *full, *box, *bin;
  gchar                  *font;

  /* view new from note deserializes the note-content. */
  priv->view = biji_note_obj_open (priv->note);

  settings = bjb_app_get_settings(g_application_get_default());
  gtk_window_add_accel_group (GTK_WINDOW (priv->window), priv->accel);
    
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

  /* Text Editor (WebKitMainView) */
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

  /* User defined color */
  GdkRGBA color ;
  if (!biji_note_obj_get_rgba(priv->note, &color))
    gdk_rgba_parse (&color, DEFAULT_NOTE_COLOR);

  biji_note_obj_set_rgba (priv->note, &color);

  /* Edition Toolbar */
  priv->edit_bar = bjb_editor_toolbar_new (overlay, self, priv->note);
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
