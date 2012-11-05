/*
 * bijiben-shell-search-provider.c - Implementation of a GNOME Shell
 *   search provider
 *
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * Bijiben is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Bijiben is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
*/

/*
 * Adapted from Nautilus shell-search-provider
 * 
 * Authors: 
 * Original code : Cosimo Cecchi <cosimoc@gnome.org>
 * Bijiben : Pierre-Yves Luyten <py@luyten.fr>
 *
 */

#include <config.h>

#include <gio/gio.h>
#include <gtk/gtk.h>

#include <tracker-sparql.h>

/* Strlen for timestamp */
#include <string.h>

/* Time */
#include <gdk/gdkx.h>

#include "bijiben-shell-search-provider-generated.h"

#define SEARCH_PROVIDER_INACTIVITY_TIMEOUT 12000 /* milliseconds */

typedef struct {
  GApplication parent;

  guint name_owner_id;
  GDBusObjectManagerServer *object_manager;
  BijibenShellSearchProvider *skeleton;

   TrackerSparqlConnection *connection;

} BijibenShellSearchProviderApp;

typedef GApplicationClass BijibenShellSearchProviderAppClass;

GType bijiben_shell_search_provider_app_get_type (void);

#define BIJIBEN_TYPE_SHELL_SEARCH_PROVIDER_APP bijiben_shell_search_provider_app_get_type()
#define BIJIBEN_SHELL_SEARCH_PROVIDER_APP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJIBEN_TYPE_SHELL_SEARCH_PROVIDER_APP, BijibenShellSearchProviderApp))

G_DEFINE_TYPE (BijibenShellSearchProviderApp, bijiben_shell_search_provider_app, G_TYPE_APPLICATION)

static TrackerSparqlCursor *
bjb_perform_query (BijibenShellSearchProviderApp *self, gchar * query )
{
  TrackerSparqlCursor * result = NULL;
  GError *error = NULL ;

  result = tracker_sparql_connection_query (self->connection,
                                            query,
                                            NULL,
                                            &error);

  if (error)
    g_warning (error->message);

  return result ;
}

GList *
biji_get_notes_with_strings (BijibenShellSearchProviderApp *self, gchar **needles)
{
  gint parser;
  GString *match = g_string_new ("");
  gchar *query;
  TrackerSparqlCursor *cursor;
  const gchar *uuid;
  GList *result = NULL;

  if (!needles)
    return result;

  /* AND is implicit into tracker */
  for (parser=0; needles[parser] != NULL; parser++)
  {
    match = g_string_append (match, needles[parser]);
    match = g_string_append (match, "* ");
  }

  query = g_strdup_printf ("SELECT nie:url(?f) WHERE { ?f fts:match '%s' }", match->str);
  g_string_free (match, TRUE);

  /* Go to the query */
  cursor = bjb_perform_query (self, query);

  if (!cursor)
    return result;

/* TODO
tracker_sparql_cursor_next_async    (TrackerSparqlCursor *self,
                                     GCancellable *cancellable,
                                     GAsyncReadyCallback _callback_,
                                     gpointer _user_data_);
*/

  while (tracker_sparql_cursor_next (cursor, NULL, NULL))
  {
    uuid = tracker_sparql_cursor_get_string (cursor, 0, 0);

    /* currently no mimetype pushed to tracker by libbiji "push note" method
     * it shall be added instead testing suffix here */
    if (uuid && g_str_has_suffix (uuid, ".note"))
      result = g_list_append (result, g_strdup(uuid));
  }

  g_free (query);
  g_object_unref (cursor);
  return result;
}

static void
add_string (gchar *single_result, GVariantBuilder *meta_result)
{
  g_variant_builder_add (meta_result, "s", single_result);
}

static void
handle_get_initial_result_set (BijibenShellSearchProvider   *skeleton,
                               GDBusMethodInvocation        *invocation,
                               gchar                       **terms,
                               gpointer                      user_data)
{
  BijibenShellSearchProviderApp *self = user_data;

  GVariantBuilder builder;
  GList *result;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));
  result = biji_get_notes_with_strings (self, terms);
  g_list_foreach (result, (GFunc) add_string, &builder);
  g_list_free_full (result, (GDestroyNotify) g_free); // ?? not sure. maybe just free, not full

  g_dbus_method_invocation_return_value (invocation, g_variant_new ("(as)", &builder));
  g_application_release (G_APPLICATION (self));
}

/* TODO : start from previous result , simply with GList */
static void
handle_get_subsearch_result_set (BijibenShellSearchProvider   *skeleton,
                                 GDBusMethodInvocation        *invocation,
                                 gchar                       **previous_results,
                                 gchar                       **terms,
                                 gpointer                      user_data)
{
  BijibenShellSearchProviderApp *self = user_data;

  GVariantBuilder builder;
  GList *result;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));
  result = biji_get_notes_with_strings (self, terms);
  g_list_foreach (result, (GFunc) add_string, &builder);
  g_list_free_full (result, (GDestroyNotify) g_free); // ?? not sure. maybe just free, not full

  g_dbus_method_invocation_return_value (invocation, g_variant_new ("(as)", &builder));
  g_application_release (G_APPLICATION (self));
}

static gchar *
get_note_icon (gchar *note__nie_url)
{
  gchar *path = g_build_filename ("usr", "share", "bijiben", "icons", "hicolor", 
                                  "48x48", "actions", "note.png",  NULL);

  GFile *gicon_file = g_file_new_for_path (path);
  g_free (path);

  GIcon *gicon = g_file_icon_new (gicon_file);
  g_object_unref (gicon_file);

  gchar *result = g_icon_to_string (gicon);
  g_object_unref (gicon);
  return result;
}

static void
add_single_note_meta (BijibenShellSearchProviderApp *self,
                      gchar *note__nie_url,
                      GVariantBuilder *results)
{
  GVariantBuilder note;
  gchar *query, *result;
  TrackerSparqlCursor *cursor;

  /* the WHERE nie:url query has a single result : the note which we're providing metadata */
  query = g_strdup_printf ("SELECT nie:url(?f) nie:title(?f) WHERE { ?f nie:url '%s'}",
                           note__nie_url);
  cursor = bjb_perform_query (self, query);
  g_free (query);

  g_variant_builder_init (&note, G_VARIANT_TYPE ("a{sv}"));

  if (tracker_sparql_cursor_next (cursor, NULL, NULL))
  {
    /* NIE:URL (id) */
    result = tracker_sparql_cursor_get_string (cursor, 0, 0);
    g_variant_builder_add (&note, "{sv}", "id", g_variant_new_string (result));

    /* NIE:TITLE (name) is the title pushed by libbiji */
    result = tracker_sparql_cursor_get_string (cursor, 1, 0);
    g_variant_builder_add (&note, "{sv}", "name", g_variant_new_string (result));

   /* ICON is currently generic icon,        *
    * TODO serialize icons in libbiji        *
    *      or deserialize note here )        */
    result = get_note_icon (NULL);
    g_variant_builder_add (&note, "{sv}", "gicon", g_variant_new_string (result));

    g_variant_builder_add_value (results, g_variant_builder_end (&note));
  }
 
  g_object_unref (cursor);
}

static void
handle_get_result_metas (BijibenShellSearchProvider  *skeleton,
                         GDBusMethodInvocation        *invocation,
                         gchar                       **results,
                         gpointer                      user_data)
{
  BijibenShellSearchProviderApp *self = user_data;

  GVariantBuilder retval;
  gint parser;

  g_variant_builder_init (&retval, G_VARIANT_TYPE ("aa{sv}"));

  for (parser=0; results[parser] != NULL; parser++)
    add_single_note_meta (self, results[parser], &retval);

  g_dbus_method_invocation_return_value (invocation, g_variant_new ("(aa{sv})", &retval));
  g_application_release (G_APPLICATION (self));
}

/* taken from Epiphany's ephy-main.c */
static Time
slowly_and_stupidly_obtain_timestamp (Display *xdisplay)
{
  Window xwindow;
  XEvent event;

  {
    XSetWindowAttributes attrs;
    Atom atom_name;
    Atom atom_type;
    char* name;

    attrs.override_redirect = True;
    attrs.event_mask = PropertyChangeMask | StructureNotifyMask;

    xwindow =
      XCreateWindow (xdisplay,
                     RootWindow (xdisplay, 0),
                     -100, -100, 1, 1,
                     0,
                     CopyFromParent,
                     CopyFromParent,
                     CopyFromParent,
                     CWOverrideRedirect | CWEventMask,
                     &attrs);

    atom_name = XInternAtom (xdisplay, "WM_NAME", TRUE);
    g_assert (atom_name != None);
    atom_type = XInternAtom (xdisplay, "STRING", TRUE);
    g_assert (atom_type != None);

    name = "Fake Window";
    XChangeProperty (xdisplay,
                     xwindow, atom_name,
                     atom_type,
                     8, PropModeReplace, (unsigned char *)name, strlen (name));
  }

  XWindowEvent (xdisplay,
                xwindow,
                PropertyChangeMask,
                &event);

  XDestroyWindow(xdisplay, xwindow);

  return event.xproperty.time;
}

static void
handle_activate_result (BijibenShellSearchProvider *skeleton,
                        GDBusMethodInvocation       *invocation,
                        gchar                       *result,
                        gpointer                     user_data)
{
  GError *error = NULL;
  guint32 timestamp;

  /* We need a timestamp here to get the correct WM focus.
   * Ideally this would be given to us by the caller, but since it
   * is not, get it ourselves.
   */
  timestamp = slowly_and_stupidly_obtain_timestamp (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()));
  gtk_show_uri (NULL, result, timestamp, &error);

  if (error != NULL) {
    g_warning ("Unable to activate %s: %s", result, error->message);
    g_error_free (error);
  }
}

static void
search_provider_name_acquired_cb (GDBusConnection *connection,
                                  const gchar     *name,
                                  gpointer         user_data)
{
  g_debug ("Search provider name acquired: %s\n", name);
}

static void
search_provider_name_lost_cb (GDBusConnection *connection,
                              const gchar     *name,
                              gpointer         user_data)
{
  g_debug ("Search provider name lost: %s\n", name);
}

static void
search_provider_bus_acquired_cb (GDBusConnection *connection,
                                 const gchar *name,
                                 gpointer user_data)
{
  BijibenShellSearchProviderApp *self = user_data;

  self->object_manager = g_dbus_object_manager_server_new ("/org/gnome/Bijiben/SearchProvider");
  self->skeleton = bijiben_shell_search_provider_skeleton_new ();

  g_signal_connect (self->skeleton, "handle-get-initial-result-set",
                    G_CALLBACK (handle_get_initial_result_set), self);
  g_signal_connect (self->skeleton, "handle-get-subsearch-result-set",
                    G_CALLBACK (handle_get_subsearch_result_set), self);
  g_signal_connect (self->skeleton, "handle-get-result-metas",
                    G_CALLBACK (handle_get_result_metas), self);
  g_signal_connect (self->skeleton, "handle-activate-result",
                    G_CALLBACK (handle_activate_result), self);

  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (self->skeleton),
                                    connection,
                                    "/org/gnome/Bijiben/SearchProvider", NULL);
  g_dbus_object_manager_server_set_connection (self->object_manager, connection);
}

static void
search_provider_app_dispose (GObject *obj)
{
  BijibenShellSearchProviderApp *self = BIJIBEN_SHELL_SEARCH_PROVIDER_APP (obj);

  if (self->name_owner_id != 0) {
    g_bus_unown_name (self->name_owner_id);
    self->name_owner_id = 0;
  }

  if (self->skeleton != NULL) {
    g_dbus_interface_skeleton_unexport (G_DBUS_INTERFACE_SKELETON (self->skeleton));
    g_clear_object (&self->skeleton);
  }

  g_object_unref (self->connection);
  g_clear_object (&self->object_manager);

  G_OBJECT_CLASS (bijiben_shell_search_provider_app_parent_class)->dispose (obj);
}

static void
search_provider_app_startup (GApplication *app)
{
  BijibenShellSearchProviderApp *self = BIJIBEN_SHELL_SEARCH_PROVIDER_APP (app);

  G_APPLICATION_CLASS (bijiben_shell_search_provider_app_parent_class)->startup (app);

  /* hold indefinitely if we're asked to persist*/
  if (g_getenv ("BIJIBEN_SEARCH_PROVIDER_PERSIST") != NULL)
    g_application_hold (app);

  self->name_owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                                        "org.gnome.Bijiben.SearchProvider",
                                        G_BUS_NAME_OWNER_FLAGS_NONE,
                                        search_provider_bus_acquired_cb,
                                        search_provider_name_acquired_cb,
                                        search_provider_name_lost_cb,
                                        app, NULL);
}

static void
bijiben_shell_search_provider_app_init (BijibenShellSearchProviderApp *self)
{
  GApplication *app = G_APPLICATION (self);
  GError *error = NULL;

  g_application_set_inactivity_timeout (app, SEARCH_PROVIDER_INACTIVITY_TIMEOUT);
  g_application_set_application_id (app, "org.gnome.Bijiben.SearchProvider");
  g_application_set_flags (app, G_APPLICATION_IS_SERVICE);

  self->connection = tracker_sparql_connection_get (NULL, &error);

  if (error)
  {
     g_warning (error->message);
     g_application_release (app);
  }
}

static void
bijiben_shell_search_provider_app_class_init (BijibenShellSearchProviderAppClass *klass)
{
  GApplicationClass *aclass = G_APPLICATION_CLASS (klass);
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  aclass->startup = search_provider_app_startup;
  oclass->dispose = search_provider_app_dispose;
}

static GApplication *
bijiben_shell_search_provider_app_new (void)
{
  return g_object_new (bijiben_shell_search_provider_app_get_type (),
                       NULL);
}

int
main (int   argc,
      char *argv[])
{
  GApplication *app;
  gint res;

  gtk_init (&argc, &argv);

  app = bijiben_shell_search_provider_app_new ();
  res = g_application_run (app, argc, argv);
  g_object_unref (app);

  return res;
}


