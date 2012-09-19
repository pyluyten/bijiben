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

#ifndef _BJB_MAIN_TOOLBAR_H_
#define _BJB_MAIN_TOOLBAR_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include <clutter-gtk/clutter-gtk.h>
#include <libgd/gd.h>

#include "bjb-main-view.h"

G_BEGIN_DECLS

#define BJB_TYPE_MAIN_TOOLBAR             (bjb_main_toolbar_get_type ())
#define BJB_MAIN_TOOLBAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BJB_TYPE_MAIN_TOOLBAR, BjbMainToolbar))
#define BJB_MAIN_TOOLBAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BJB_TYPE_MAIN_TOOLBAR, BjbMainToolbarClass))
#define BJB_IS_MAIN_TOOLBAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BJB_TYPE_MAIN_TOOLBAR))
#define BJB_IS_MAIN_TOOLBAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BJB_TYPE_MAIN_TOOLBAR))
#define BJB_MAIN_TOOLBAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BJB_TYPE_MAIN_TOOLBAR, BjbMainToolbarClass))

typedef struct _BjbMainToolbarClass BjbMainToolbarClass;
typedef struct _BjbMainToolbar BjbMainToolbar;
typedef struct _BjbMainToolbarPrivate BjbMainToolbarPrivate;

struct _BjbMainToolbarClass
{
  GObjectClass parent_class;
};

struct _BjbMainToolbar
{
  GObject parent_instance;
  BjbMainToolbarPrivate *priv; 
};

GType bjb_main_toolbar_get_type (void) G_GNUC_CONST;

BjbMainToolbar * bjb_main_toolbar_new (GdMainView *view,BjbMainView *parent);

ClutterActor *bjb_main_toolbar_get_actor (BjbMainToolbar *self);

void bjb_main_toolbar_set_view (BjbMainToolbar *self, GdMainView *view);

G_END_DECLS

#endif /* _BJB_MAIN_TOOLBAR_H_ */
