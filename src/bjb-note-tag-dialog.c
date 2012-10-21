/* bjb-note-tag-dialog.c
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

#include "bjb-note-tag-dialog.h"
#include "bjb-window-base.h"

#define BJB_NOTE_TAG_DIALOG_DEFAULT_WIDTH  460
#define BJB_NOTE_TAG_DIALOG_DEFAULT_HEIGHT 380

/* Model for tree view */
enum {
  COL_SELECTION,
  COL_TAG_NAME,
  N_COLUMNS
};

enum {
  SELECTION_INCONSISTENT = -1,
  SELECTION_FALSE = 0,
  SELECTION_TRUE
};

/* Prop */
enum
{
  PROP_0,
  PROP_WINDOW,
  PROP_NOTES,
  NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

struct _BjbNoteTagDialogPrivate
{
  // parent
  GtkWindow *window;

  // to create new tag
  GtkWidget    * entry;

  // data
  GList *notes;
  GtkListStore *store;
};

#define BJB_NOTE_TAG_DIALOG_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BJB_TYPE_NOTE_TAG_DIALOG, BjbNoteTagDialogPrivate))

G_DEFINE_TYPE (BjbNoteTagDialog, bjb_note_tag_dialog, GTK_TYPE_DIALOG);

static void
append_tag (gchar *tag, BjbNoteTagDialog *self)
{
  BjbNoteTagDialogPrivate *priv = self->priv;

  GtkTreeIter iter;
  gint note_has_tag;
  GList *l;

  gtk_list_store_append (priv->store, &iter);
  note_has_tag = biji_note_obj_has_tag (priv->notes->data, tag);

  /* Check if other notes have the same */  
  for (l = priv->notes; l != NULL; l = l->next)
  {
    if (biji_note_obj_has_tag (l->data, tag) != note_has_tag)
    {
      note_has_tag = SELECTION_INCONSISTENT;
      break;
    }
  }

  gtk_list_store_set (priv->store, &iter,
                      COL_SELECTION, note_has_tag,
                      COL_TAG_NAME ,          tag, -1);
}

/* Current tags selection & sorting might be improved */
static void
update_tags_model (BjbNoteTagDialog *self)
{
  GList *all_tags = NULL;

  all_tags = g_list_copy (bjb_window_base_get_tags (GTK_WIDGET(self->priv->window)));
  all_tags = g_list_sort (all_tags, (GCompareFunc) g_strcmp0);
  g_list_foreach (all_tags, (GFunc) append_tag, self);

  g_list_free (all_tags);
}

/* TODO This two func should rather be direct call to libbiji
 * (simply biji_note_obj_*_tag)
 * It's up to libbiji to handle tracker & save */
static void
note_dialog_add_tag (BijiNoteObj *note, gchar *tag)
{
  g_warning ("note %s:", biji_note_obj_get_title (note));
  g_warning ("adding tag %s:", tag);

  if (biji_note_obj_add_tag (note, tag))
  {
    biji_note_obj_save_note (note);
    push_existing_tag_to_note (tag, note);
  }
}

static void
note_dialog_remove_tag (BijiNoteObj *note, gchar *tag)
{
  g_warning ("note %s:", biji_note_obj_get_title (note));
  g_warning ("removing tag %s:", tag);

  if (biji_note_obj_remove_tag (note, tag))
  {
    biji_note_obj_save_note (note);
    remove_tag_from_note (tag, note);
  }
}

static void
on_tag_toggled (GtkCellRendererToggle *cell,
                gchar *path_str,
                BjbNoteTagDialog *self)
{
  BjbNoteTagDialogPrivate *priv = self->priv;

  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  GtkTreeModel *model = GTK_TREE_MODEL (priv->store);
  GtkTreeIter iter;
  gint toggle_item;
  gint *column;
  gchar *tag;

  column = g_object_get_data (G_OBJECT (cell), "column");
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, column, &toggle_item, -1);
  gtk_tree_model_get (model, &iter,COL_TAG_NAME, &tag,-1);

  if (toggle_item == SELECTION_INCONSISTENT || toggle_item == SELECTION_FALSE)
  {
    g_list_foreach (priv->notes, (GFunc) note_dialog_add_tag, tag);
    toggle_item = SELECTION_TRUE;
  }
  else
  {
    g_list_foreach (priv->notes, (GFunc) note_dialog_remove_tag, tag);
    toggle_item = SELECTION_FALSE;
  }

  gtk_list_store_set (priv->store, &iter, column, toggle_item, -1);
  gtk_tree_path_free (path);
}

static void
add_new_tag (BjbNoteTagDialog *self)
{
  /* Push the tag to tracker */
  push_tag_to_tracker ((gchar*) gtk_entry_get_text(GTK_ENTRY(self->priv->entry)));
  bjb_window_base_set_tags (GTK_WIDGET (self->priv->window), get_all_tracker_tags());

  /* Update the view */
  gtk_list_store_clear (self->priv->store);
  update_tags_model (self);
}

static void
bjb_note_tag_toggle_cell_data_func (GtkTreeViewColumn *column,
                                    GtkCellRenderer   *cell_renderer,
                                    GtkTreeModel      *tree_model,
                                    GtkTreeIter       *iter,
                                    gpointer           user_data)
{
  GValue inconsistent = { 0 };
  gint selection;

  gtk_tree_model_get (tree_model, iter, COL_SELECTION, &selection, -1);
  gtk_cell_renderer_toggle_set_active (GTK_CELL_RENDERER_TOGGLE (cell_renderer),
                                       SELECTION_TRUE == selection);

  g_value_init (&inconsistent, G_TYPE_BOOLEAN);
  g_value_set_boolean (&inconsistent, SELECTION_INCONSISTENT == selection);
  g_object_set_property (G_OBJECT (cell_renderer), "inconsistent", &inconsistent);
}

static void
add_columns (GtkTreeView *view, BjbNoteTagDialog *self)
{
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell_renderer;

  /* List column: toggle */
  column = gtk_tree_view_column_new ();
  gtk_tree_view_append_column (view, column);

  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (column, 50);

  cell_renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (cell_renderer, "toggled",
                    G_CALLBACK (on_tag_toggled), self);

  gtk_tree_view_column_pack_start (column, cell_renderer, TRUE);
  gtk_tree_view_column_set_cell_data_func (column,
                                           cell_renderer,
                                           bjb_note_tag_toggle_cell_data_func,
                                           NULL,
                                           NULL);

  /* List column: tag */
  column = gtk_tree_view_column_new ();
  gtk_tree_view_append_column (view, column);

  cell_renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_set_expand (column, TRUE);
  gtk_tree_view_column_pack_start (column, cell_renderer, TRUE);
  gtk_tree_view_column_add_attribute (column, cell_renderer, "text", COL_TAG_NAME);

}

static void
bjb_note_tag_dialog_init (BjbNoteTagDialog *self)
{
  BjbNoteTagDialogPrivate *priv = BJB_NOTE_TAG_DIALOG_GET_PRIVATE(self);

  self->priv = priv;
  priv->notes = NULL;
  priv->window = NULL;

  gtk_window_set_default_size (GTK_WINDOW (self),
                               BJB_NOTE_TAG_DIALOG_DEFAULT_WIDTH,
                               BJB_NOTE_TAG_DIALOG_DEFAULT_HEIGHT);

  g_signal_connect_swapped (self, "response",
                            G_CALLBACK (gtk_widget_destroy), self);


  priv->store = gtk_list_store_new (N_COLUMNS,
                                    G_TYPE_INT,      // tag active
                                    G_TYPE_STRING);  // tag 
}

static void
on_closed_clicked (BjbNoteTagDialog *self)
{
  gtk_dialog_response (GTK_DIALOG (self), 0);
}

static void
bjb_note_tag_dialog_constructed (GObject *obj)
{
  BjbNoteTagDialog *self = BJB_NOTE_TAG_DIALOG (obj);
  BjbNoteTagDialogPrivate *priv = self->priv;
  GtkWidget *hbox, *label, *new, *area, *sw, *close;
  GtkTreeView *treeview;

  gtk_window_set_transient_for (GTK_WINDOW (self), priv->window);

  area = gtk_dialog_get_content_area (GTK_DIALOG (self));
  gtk_container_set_border_width (GTK_CONTAINER (area), 8);

  label = gtk_label_new ("Enter a name to create a tag");
  gtk_box_pack_start (GTK_BOX (area), label, FALSE, FALSE, 2);

  /* New Tag */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_box_pack_start (GTK_BOX (area), hbox, FALSE, FALSE, 2);

  self->priv->entry = gtk_entry_new();
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->entry, TRUE, TRUE, 0);

  new = gtk_button_new_with_label ("New tag");
  g_signal_connect_swapped (new, "clicked", G_CALLBACK (add_new_tag), self);

  gtk_box_pack_start (GTK_BOX (hbox), new, FALSE, FALSE, 2);

  /* List of tags */
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  update_tags_model (self);
  treeview = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (priv->store)));
  g_object_unref (self->priv->store);

  gtk_tree_view_set_rules_hint (treeview, TRUE);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (treeview),
                               GTK_SELECTION_MULTIPLE);
  
  add_columns (treeview, self);
  gtk_container_add (GTK_CONTAINER (sw), GTK_WIDGET (treeview));

  gtk_box_pack_start (GTK_BOX (area), sw, TRUE, TRUE,2);

  /* Response */
  close = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_box_pack_start (GTK_BOX (area), close, FALSE, FALSE,2);
  g_signal_connect_swapped (close, "clicked",
                            G_CALLBACK (on_closed_clicked), self);

  gtk_widget_show_all (area);
}

static void
bjb_note_tag_dialog_finalize (GObject *object)
{
  G_OBJECT_CLASS (bjb_note_tag_dialog_parent_class)->finalize (object);
}

static void
bjb_note_tag_dialog_get_property (GObject      *object,
                                  guint        prop_id,
                                  GValue       *value,
                                  GParamSpec   *pspec)
{
  BjbNoteTagDialog *self = BJB_NOTE_TAG_DIALOG (object);

  switch (prop_id)
  {
    case PROP_WINDOW:
      g_value_set_object (value, self->priv->window);
      break;
    case PROP_NOTES:
      g_value_set_pointer (value, self->priv->notes);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
bjb_note_tag_dialog_set_property (GObject        *object,
                                  guint          prop_id,
                                  const GValue   *value,
                                  GParamSpec     *pspec)
{
  BjbNoteTagDialog *self = BJB_NOTE_TAG_DIALOG (object);

  switch (prop_id)
  {
    case PROP_WINDOW:
      self->priv->window = g_value_get_object(value);
      break;
    case PROP_NOTES:
      self->priv->notes = g_value_get_pointer (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
bjb_note_tag_dialog_class_init (BjbNoteTagDialogClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (BjbNoteTagDialogPrivate));

  object_class->constructed = bjb_note_tag_dialog_constructed;
  object_class->finalize = bjb_note_tag_dialog_finalize;
  object_class->get_property = bjb_note_tag_dialog_get_property;
  object_class->set_property = bjb_note_tag_dialog_set_property;

  properties[PROP_WINDOW] = g_param_spec_object ("window",
                                                 "Window",
                                                 "Parent Window",
                                                 GTK_TYPE_WIDGET,
                                                 G_PARAM_READWRITE |
                                                 G_PARAM_CONSTRUCT |
                                                 G_PARAM_STATIC_STRINGS);

  properties[PROP_NOTES] = g_param_spec_pointer ("notes",
                                                 "Biji Notes",
                                                 "The Notes to tag",
                                                 G_PARAM_READWRITE |
                                                 G_PARAM_CONSTRUCT_ONLY |
                                                 G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, NUM_PROPERTIES, properties);
}

void
bjb_note_tag_dialog_new (GtkWindow *parent,
                         GList     *biji_note_obj)
{
  BjbNoteTagDialog *self = g_object_new (BJB_TYPE_NOTE_TAG_DIALOG,
                                         "window", parent,
                                         "notes", biji_note_obj,
                                         NULL);

  gtk_dialog_run (GTK_DIALOG (self));
}
