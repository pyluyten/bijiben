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

#ifndef _BJB_VIEW_MODE_BUTTON_H_
#define _BJB_VIEW_MODE_BUTTON_H_

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BJB_TYPE_VIEW_MODE_BUTTON             (bjb_view_mode_button_get_type ())
#define BJB_VIEW_MODE_BUTTON(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BJB_TYPE_VIEW_MODE_BUTTON, BjbViewModeButton))
#define BJB_VIEW_MODE_BUTTON_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BJB_TYPE_VIEW_MODE_BUTTON, BjbViewModeButtonClass))
#define BJB_IS_VIEW_MODE_BUTTON(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BJB_TYPE_VIEW_MODE_BUTTON))
#define BJB_IS_VIEW_MODE_BUTTON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BJB_TYPE_VIEW_MODE_BUTTON))
#define BJB_VIEW_MODE_BUTTON_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BJB_TYPE_VIEW_MODE_BUTTON, BjbViewModeButtonClass))

typedef struct _BjbViewModeButtonClass BjbViewModeButtonClass;
typedef struct _BjbViewModeButton BjbViewModeButton;
typedef struct _BjbViewModeButtonPrivate BjbViewModeButtonPrivate;

typedef enum {
  BJB_VIEW_MODE_GRID,
  BJB_VIEW_MODE_LIST
} BjbViewModeType ;


struct _BjbViewModeButtonClass
{
	GtkToolButtonClass parent_class;
};

struct _BjbViewModeButton
{
	GtkToolButton parent_instance;

    BjbViewModeButtonPrivate *priv; 
};

GType bjb_view_mode_button_get_type (void) G_GNUC_CONST;

GtkWidget * bjb_view_mode_button_new ( BjbViewModeType mode ) ;

BjbViewModeType bjb_view_mode_button_get_selection_mode ( BjbViewModeButton *self ) ;

void bjb_view_mode_button_set_selection_mode (BjbViewModeButton *self, BjbViewModeType mode ) ;


G_END_DECLS

#endif /* _BJB_VIEW_MODE_BUTTON_H_ */
