#include <clutter-gtk/clutter-gtk.h>

#include "bjb-selection-panel.h"
#include "utils/bjb-icons-colors.h"

/* TODO : implement osd style , maybe re-use stuff */
ClutterActor *
get_selection_panel(BjbMainView *v)
{
  ClutterActor    *result ;
  GtkWidget       *toolbar, *tr_im, *delete ;
  GtkStyleContext *context ;
  GdkRGBA          color = {0.0, 0.0, 0.0, 0.0};

  toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);

  /* Stick note ? */

  /* Throw note for later */

  /* Trash note */
  tr_im = get_icon ("user-trash-symbolic"); 
  delete = gtk_button_new();
  gtk_button_set_image (GTK_BUTTON(delete),tr_im);
  g_signal_connect(delete,"clicked",
                   G_CALLBACK(action_delete_selected_notes),v);
  gtk_box_pack_start(GTK_BOX(toolbar),delete,FALSE,FALSE,2);

  context = gtk_widget_get_style_context (toolbar);
  gtk_style_context_add_class (context, "osd");
  gtk_widget_show_all(toolbar);

  result = gtk_clutter_actor_new_with_contents(toolbar);  
  gtk_widget_override_background_color (toolbar,
                                        GTK_STATE_FLAG_NORMAL,
                                        &color);

  return result ;
}
