/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * bijiben
 * Copyright (C) Pierre-Yves Luyten 2012 <py@luyten.fr>
 * 
bijiben is free software: you can redistribute it and/or modify it
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

#ifndef _BJB_CONTROLLER_H_
#define _BJB_CONTROLLER_H_

#include <glib-object.h>
#include <libbiji/libbiji.h>

#include "widgets/gd-main-view.h"


G_BEGIN_DECLS

#define BJB_TYPE_CONTROLLER             (bjb_controller_get_type ())
#define BJB_CONTROLLER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BJB_TYPE_CONTROLLER, BjbController))
#define BJB_CONTROLLER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BJB_TYPE_CONTROLLER, BjbControllerClass))
#define BJB_IS_CONTROLLER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BJB_TYPE_CONTROLLER))
#define BJB_IS_CONTROLLER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BJB_TYPE_CONTROLLER))
#define BJB_CONTROLLER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BJB_TYPE_CONTROLLER, BjbControllerClass))

typedef struct _BjbControllerClass BjbControllerClass;
typedef struct _BjbController BjbController;
typedef struct _BjbControllerPrivate BjbControllerPrivate;


struct _BjbControllerClass
{
	GObjectClass parent_class;
};

struct _BjbController
{
	GObject parent_instance;

    BjbControllerPrivate *priv; 
};


GType bjb_controller_get_type (void) G_GNUC_CONST;


/* Model Columns */
/* TODO : use GD_MAIN_VIEW columns instead */
enum
{
  COL_URN,
  COL_URI,
  COL_NAME,
  COL_AUTHOR,
  COL_IMAGE,
  COL_MTIME,
  COL_SELECTED,
  NUMBER_COLUMNS
};

BjbController * bjb_controller_new ( BijiNoteBook *book, gchar *needle ) ;

void bjb_controller_set_book (BjbController * self, BijiNoteBook * book ) ;

void bjb_controller_set_needle (BjbController *self, const gchar *needle ) ; 

gchar * bjb_controller_get_needle (BjbController *self ) ;

GtkTreeModel * bjb_controller_get_model  (BjbController *self) ;

GtkTreeModel * bjb_controller_get_completion(BjbController *self);

void bjb_controller_set_main_view (BjbController *self, GdMainView *current);

G_END_DECLS

#endif /* _BJB_CONTROLLER_H_ */
