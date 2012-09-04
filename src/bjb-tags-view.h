/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * bijiben-0.1
 * 
 * bijiben-0.1 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * bijiben-0.1 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TAGS_VIEW_H_
#define _TAGS_VIEW_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define BJB_TYPE_TAGS_VIEW             (bjb_tags_view_get_type ())
#define BJB_TAGS_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BJB_TYPE_TAGS_VIEW, BjbTagsView))
#define BJB_TAGS_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BJB_TYPE_TAGS_VIEW, BjbTagsViewClass))
#define BJB_IS_TAGS_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BJB_TYPE_TAGS_VIEW))
#define BJB_IS_TAGS_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BJB_TYPE_TAGS_VIEW))
#define BJB_TAGS_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BJB_TYPE_TAGS_VIEW, BjbTagsViewClass))

typedef struct _BjbTagsViewClass BjbTagsViewClass;
typedef struct _BjbTagsView BjbTagsView;

typedef struct _BjbTagsViewPrivate BjbTagsViewPrivate;

struct _BjbTagsViewClass
{
  GtkBoxClass parent_class;
};

struct _BjbTagsView
{
  GtkBox parent_instance ;
  BjbTagsViewPrivate *priv;
};

GType bjb_tags_view_get_type (void) G_GNUC_CONST;

ClutterActor * bjb_tags_view_new(GtkWidget *biji_main_window);

G_END_DECLS

#endif /* _TAGS_VIEW_H_ */
