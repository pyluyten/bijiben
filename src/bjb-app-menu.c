#include "bjb-app-menu.h"
#include "bjb-bijiben.h"
#include "bjb-main-view.h"
#include "bjb-settings.h"
#include "bjb-window-base.h"

/* Callbacks */

void show_about_dialog(GtkApplication *app)
{
  GList * windows = gtk_application_get_windows(app);
  
  gtk_show_about_dialog(
  g_list_nth_data(windows,0),
	"program-name", "Bijiben",
	"comments", "Simple noteboook for GNOME",
	"license", "GPLv3",
	"version", "0.2.0",
	"copyright", "Pierre-Yves Luyten 2012",
	NULL,NULL,NULL);

}

void 
summary()
{
	GError *error = NULL;
	gtk_show_uri (NULL, "ghelp:bijiben", gtk_get_current_event_time (), &error);
}

static void
new_activated (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       user_data)
{
  bjb_window_base_new();
}

static void
about_activated (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  show_about_dialog(GTK_APPLICATION(user_data));
}

static void
help_activated (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  GError *error = NULL;
  gtk_show_uri (NULL, "ghelp:bijiben", gtk_get_current_event_time (), &error);
}

static void
quit_activated (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  GApplication *app = user_data;

  g_application_quit (app);
}

/* Menu */

static GActionEntry app_entries[] = {
           { "new", new_activated, NULL, NULL, NULL },
           { "help", help_activated, NULL, NULL, NULL },
           { "about", about_activated, NULL, NULL, NULL },
           { "quit", quit_activated, NULL, NULL, NULL },
};


void bjb_app_menu_set(GApplication *application) 
{
  GtkBuilder *builder;

  g_action_map_add_action_entries (G_ACTION_MAP (application),
                                   app_entries,
                                   G_N_ELEMENTS (app_entries),
                                   application);
    
  builder = gtk_builder_new ();
  gtk_builder_add_from_resource (builder, "/org/gnome/bijiben/app-menu.ui", NULL);

  gtk_application_set_app_menu (GTK_APPLICATION (application), 
                                G_MENU_MODEL (gtk_builder_get_object (builder, "app-menu")));
  g_object_unref (builder);
}
