/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
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
#include "bjb-tracker.h"
#include "bjb-main-view.h"
#include "bjb-note-view.h"
#include "bjb-tags-view.h"
#include "bjb-window-base.h"

static void bijiben_startup (GApplication *application);

// Currently, priv is useless structure.
struct _BijibenPriv
{
  BijiNoteBook *book;
  BjbSettings *settings ;
};

G_DEFINE_TYPE (Bijiben, bijiben, GTK_TYPE_APPLICATION);

static void
bijiben_new_window (GApplication *app,GFile *file)
{
  gchar *bijiben_path;
  GFile *folder;
  BijiNoteBook *book;
  
  /* Sanitize dir, load book . but should be a thread */
  bijiben_path =  g_strdup_printf("%s/bijiben",g_get_user_data_dir());
  folder= g_file_new_for_path (bijiben_path);
  g_file_make_directory(folder, NULL, NULL)
  g_object_unref(folder);
  book = biji_book_new_from_dir (bijiben_path);
  g_free(bijiben_path);
  BIJIBEN_APPLICATION (app)->priv->book = book ;

  /* we should show propose importing or looking with tracker */
  if ( book == NULL )
  {
    g_warning("notebook null");
    return ;
  }

  /* No file : show main window */
  if (file == NULL)
  {
    bjb_window_base_new( GTK_APPLICATION(app));
  }

  /* File : show main window, then switch to note. */
  else
  {
    BijiNoteObj* cur= biji_note_get_new_from_file(g_file_get_path(file));
    GtkWindow *win = bjb_window_base_new(GTK_APPLICATION(app));

    bjb_window_base_set_frame(BJB_WINDOW_BASE(win),
                              bjb_note_view_new(GTK_WIDGET(win),cur));
  
    gtk_window_set_title(GTK_WINDOW(win),biji_note_get_title (cur));
  }
}



// If Bijiben already runs, show the window(s) and that's all folks.
static void
bijiben_activate (GApplication *app)
{
  GList * list = gtk_application_get_windows (GTK_APPLICATION(app));

  // try each window and show main window.
  if ( list )
  {
	g_list_foreach(list,(GFunc) gtk_window_present,NULL) ;	              
  }

  // create the main window
  else 
	bijiben_new_window (app, NULL);  
	
  return ;
}

// this function works ? why ? 
static void
bijiben_open (GApplication  *application,
              GFile        **files,
              gint           n_files,
              const gchar   *hint)
{
  g_message("bijiben open");

  //GList * list = gtk_application_get_windows (GTK_APPLICATION(app));
  // if list ...
	
  gint i;

  for (i = 0; i < n_files; i++)
    bijiben_new_window(application,files[i]);
    /*create_new_window_for_note(application,
                               biji_note_get_new_from_file(
                                                    g_file_get_path(files[i])));*/
}

static void
bijiben_init (Bijiben *object)
{	
  object->priv = 
  G_TYPE_INSTANCE_GET_PRIVATE(object,BIJIBEN_TYPE_APPLICATION,BijibenPriv);

  object->priv->book = NULL ;

  // TODO create a preferences GOjbect with properties to bind.
  g_message("bijiben init : preferences");
  object->priv->settings = initialize_settings();
}

static void
bijiben_finalize (GObject *object)
{
  G_OBJECT_CLASS (bijiben_parent_class)->finalize (object);
}

static void
bijiben_class_init (BijibenClass *klass)
{
  G_APPLICATION_CLASS (klass)->activate = bijiben_activate;
  G_APPLICATION_CLASS (klass)->open = bijiben_open;
  G_APPLICATION_CLASS (klass)->startup = bijiben_startup;

  G_OBJECT_CLASS (klass)->finalize = bijiben_finalize;
	
  g_type_class_add_private (klass, sizeof (BijibenPriv));
}

Bijiben *
bijiben_new (void)
{
  g_type_init ();

  return g_object_new (bijiben_get_type (),
	                   "application-id", "org.gnome.bijiben",
	                   "flags", G_APPLICATION_HANDLES_OPEN,
	                   NULL);
}

void
bijiben_set_book(GtkApplication *app,BijiNoteBook *book)
{
  g_return_if_fail(BIJIBEN_IS_APPLICATION(app)) ;
	
  BIJIBEN_APPLICATION(app)->priv->book = book ;
}

BijiNoteBook *
bijiben_get_book(GtkApplication *app)
{
  g_return_val_if_fail(BIJIBEN_IS_APPLICATION(app),NULL) ;
	
  return BIJIBEN_APPLICATION(app)->priv->book ;
}

const gchar *
bijiben_get_bijiben_dir (void)
{
  static gchar *icons_dir = NULL;
  if (!icons_dir)
  {
    icons_dir = g_strdup_printf ("%s",DATADIR);
  }
  return (const gchar *) icons_dir;
}

/* const gchar * bijiben_get_bijiben_private_dir (void) */

BjbSettings * bjb_app_get_settings(gpointer application)
{
  return BIJIBEN_APPLICATION(application)->priv->settings ;
}

/* Create the window with its controller. Then switches to note view.
 * This should be fixed we rather want an independant window
 * with its controller but without any frame */
void
create_new_window_for_note(gpointer app , BijiNoteObj *note)
{
  GtkWidget     *win;
  BjbWindowBase *base;
  BjbMainView   *main;

  g_return_if_fail (G_IS_APPLICATION (app));

  win = bjb_window_base_new (app);
  base = BJB_WINDOW_BASE (win);

  main = BJB_MAIN_VIEW (bjb_window_base_get_main_view (win));
  switch_to_note_view (main,note);
}

// return pointer to BJB main window
gpointer
create_new_main_window(gpointer app)
{
  return bjb_window_base_new(app);
}

void
bijiben_startup (GApplication *application)
{
  G_APPLICATION_CLASS (bijiben_parent_class)->startup (application);

  if (gtk_clutter_init (NULL, NULL) != CLUTTER_INIT_SUCCESS)
    {
      g_warning ("Unable to initialize Clutter");
      return;
    }

  bjb_app_menu_set(application) ; 
}
