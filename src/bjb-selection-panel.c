#include "bjb-selection-panel.h"
#include "utils/bjb-icons-colors.h"



GtkWidget * get_selection_panel(BjbMainView *v)
{
  GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,TRUE,TRUE,2);

  GtkWidget *tr_im ;

  tr_im = get_icon ("user-trash-symbolic"); 
  GtkWidget *delete = gtk_button_new();
  gtk_button_set_image (GTK_BUTTON(delete),tr_im);
  g_signal_connect(delete,"clicked",
                   G_CALLBACK(action_delete_selected_notes),v);
    
  gtk_box_pack_start(GTK_BOX(hbox),delete,FALSE,FALSE,2);

  gtk_widget_show_all(hbox);
    
    
  return GTK_WIDGET(hbox) ;
}

