/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * bijiben.h
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

#ifndef _BIJIBEN_
#define _BIJIBEN_

#include <gtk/gtk.h>
#include <libbiji/libbiji.h>

#include "bjb-settings.h"

G_BEGIN_DECLS

#define BIJIBEN_TYPE_APPLICATION             (bijiben_get_type ())
#define BIJIBEN_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJIBEN_TYPE_APPLICATION, Bijiben))
#define BIJIBEN_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJIBEN_TYPE_APPLICATION, BijibenClass))
#define BIJIBEN_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJIBEN_TYPE_APPLICATION))
#define BIJIBEN_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJIBEN_TYPE_APPLICATION))
#define BIJIBEN_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJIBEN_TYPE_APPLICATION, BijibenClass))

typedef struct _BijibenClass BijibenClass;
typedef struct _Bijiben Bijiben;

typedef struct _BijibenPriv BijibenPriv;

struct _BijibenClass
{
  GtkApplicationClass parent_class;
};

struct _Bijiben
{
  GtkApplication parent_instance;
  BijibenPriv *priv;
};

GType bijiben_get_type (void) G_GNUC_CONST;
Bijiben *bijiben_new (void);

#define BIJIBEN_MAIN_WIN_TITLE "Bijiben"

// Accessors

const gchar *bijiben_get_bijiben_dir (void);
void bijiben_set_book(GtkApplication *app,BijiNoteBook *book);
BijiNoteBook *bijiben_get_book(GtkApplication *app) ;
BjbSettings * bjb_app_get_settings(gpointer application);

// Actions or callbacks 

void create_new_window_for_note(gpointer application , BijiNoteObj *note) ;
gpointer create_new_main_window(gpointer app) ; //return pointer to window

G_END_DECLS

#endif /* _APPLICATION_H_ */
