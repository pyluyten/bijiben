/*
 * bijiben.c
 * Copyright (C) Pierre-Yves LUYTEN 2011 <py@luyten.fr>
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

#include <glib/gi18n.h>
#include <stdlib.h>
#include <clutter-gtk/clutter-gtk.h>

#include <libbiji/libbiji.h>

#include "bjb-app-menu.h"
#include "bjb-bijiben.h"
#include "bjb-settings.h"
#include "bjb-main-view.h"
#include "bjb-note-view.h"
#include "bjb-window-base.h"

struct _BijibenPriv
{
  BijiNoteBook *book;
  BjbSettings *settings ;
};

G_DEFINE_TYPE (Bijiben, bijiben, GTK_TYPE_APPLICATION);

static void
bijiben_new_window_internal (GApplication *app,
                             GFile *file,
                             BijiNoteObj *note_obj)
{
  GtkWindow *win = bjb_window_base_new();
  BijiNoteObj* note = NULL;

  if (file != NULL)
    note = biji_note_get_new_from_file(g_file_get_path(file));
  else if (note_obj != NULL)
    note = note_obj;

  if (note != NULL)
    {
      bjb_window_base_set_frame(BJB_WINDOW_BASE(win),
                                CLUTTER_ACTOR(bjb_note_view_new(GTK_WIDGET(win), note)));

      gtk_window_set_title(win,biji_note_get_title (note));
    }
}

void
bijiben_new_window_for_note (GApplication *app,
                             BijiNoteObj *note)
{
  bijiben_new_window_internal(app, NULL, note);
}

static void
bijiben_activate (GApplication *app)
{
  GList *windows = gtk_application_get_windows (GTK_APPLICATION (app));

  // ensure the last used window is raised
  gtk_window_present (g_list_nth_data (windows, 0));
}

static void
bijiben_open (GApplication  *application,
              GFile        **files,
              gint           n_files,
              const gchar   *hint)
{
  gint i;

  for (i = 0; i < n_files; i++)
    bijiben_new_window_internal(application, files[i],NULL);
}

static void
bijiben_init (Bijiben *object)
{	
  object->priv =
    G_TYPE_INSTANCE_GET_PRIVATE(object,BIJIBEN_TYPE_APPLICATION,BijibenPriv);

  object->priv->settings = initialize_settings();
}

static void
bijiben_startup (GApplication *application)
{
  Bijiben *self = BIJIBEN_APPLICATION (application);
  gchar *storage_path;
  GFile *storage;

  G_APPLICATION_CLASS (bijiben_parent_class)->startup (application);

  if (gtk_clutter_init (NULL, NULL) != CLUTTER_INIT_SUCCESS)
    {
      g_warning ("Unable to initialize Clutter");
      return;
    }

  bjb_app_menu_set(application);

  storage_path = g_build_filename (g_get_user_data_dir (), "bijiben", NULL);
  storage = g_file_new_for_path (storage_path);

  self->priv->book = biji_note_book_new (storage);

  g_free (storage_path);
  g_object_unref (storage);

  // create the first window
  bijiben_new_window_internal (application, NULL, NULL);
}

static void
bijiben_finalize (GObject *object)
{
  G_OBJECT_CLASS (bijiben_parent_class)->finalize (object);
}

static void
bijiben_class_init (BijibenClass *klass)
{
  GApplicationClass *aclass = G_APPLICATION_CLASS (klass);
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  aclass->activate = bijiben_activate;
  aclass->open = bijiben_open;
  aclass->startup = bijiben_startup;

  oclass->finalize = bijiben_finalize;

  g_type_class_add_private (klass, sizeof (BijibenPriv));
}

Bijiben *
bijiben_new (void)
{
  g_type_init ();

  return g_object_new (BIJIBEN_TYPE_APPLICATION,
	                   "application-id", "org.gnome.bijiben",
	                   "flags", G_APPLICATION_HANDLES_OPEN,
	                   NULL);
}

BijiNoteBook *
bijiben_get_book(Bijiben *self)
{
  return self->priv->book ;
}

const gchar *
bijiben_get_bijiben_dir (void)
{
  return DATADIR;
}

BjbSettings * bjb_app_get_settings(gpointer application)
{
  return BIJIBEN_APPLICATION(application)->priv->settings ;
}
