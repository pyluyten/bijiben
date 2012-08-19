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

#include "bjb-view-mode-button.h"


struct _BjbViewModeButtonPrivate
{
	BjbViewModeType selection_mode;
};

/* GObject properties */

enum {
    PROP_0,
    PROP_SELECTION_MODE,
    NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

#define BJB_VIEW_MODE_BUTTON_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BJB_TYPE_VIEW_MODE_BUTTON, BjbViewModeButtonPrivate))


G_DEFINE_TYPE (BjbViewModeButton, bjb_view_mode_button, GTK_TYPE_TOOL_BUTTON);

static gboolean
on_bjb_view_mode_button_clicked ( GtkWidget *widget ) 
{
    BjbViewModeButton *self = BJB_VIEW_MODE_BUTTON ( widget) ;
	
	if ( self->priv->selection_mode == BJB_VIEW_MODE_LIST )
	{
		bjb_view_mode_button_set_selection_mode ( self,
		                                         BJB_VIEW_MODE_GRID );
	}

	else
	{
		bjb_view_mode_button_set_selection_mode ( self,
		                                         BJB_VIEW_MODE_LIST );
	}

	return FALSE ;

}

static void
bjb_view_mode_button_init (BjbViewModeButton *self)
{
	BjbViewModeButtonPrivate *priv ;
	
	priv = BJB_VIEW_MODE_BUTTON_GET_PRIVATE(self);
	self->priv = priv ;

	/* Connect */
	g_signal_connect ( self, "clicked",
	                   G_CALLBACK(on_bjb_view_mode_button_clicked) , NULL ) ;
	
}

static void
bjb_view_mode_button_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (bjb_view_mode_button_parent_class)->finalize (object);
}

static void
bjb_view_mode_button_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  BjbViewModeButton *self = BJB_VIEW_MODE_BUTTON (object);

  switch (property_id)
    {
    case PROP_SELECTION_MODE:
      g_value_set_int (value, self->priv->selection_mode);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
bjb_view_mode_button_set_property (GObject    *object,
                                   guint       property_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  BjbViewModeButton *self = BJB_VIEW_MODE_BUTTON (object);

  switch (property_id)
    {
    case PROP_SELECTION_MODE:
	  bjb_view_mode_button_set_selection_mode(self,g_value_get_int(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
bjb_view_mode_button_class_init (BjbViewModeButtonClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GtkToolButtonClass* parent_class = GTK_TOOL_BUTTON_CLASS (klass);

	g_type_class_add_private (klass, sizeof (BjbViewModeButtonPrivate));

	object_class->get_property = bjb_view_mode_button_get_property;
    object_class->set_property = bjb_view_mode_button_set_property;
	object_class->finalize = bjb_view_mode_button_finalize;

	properties[PROP_SELECTION_MODE] =
    g_param_spec_int ("selection-mode",
                      "selection-mode",
                      "Selection Mode",
                      BJB_VIEW_MODE_GRID,
                      BJB_VIEW_MODE_LIST,
                      BJB_VIEW_MODE_LIST,
                      G_PARAM_READWRITE |
                      G_PARAM_CONSTRUCT |
                      G_PARAM_STATIC_STRINGS);

	g_object_class_install_property (object_class, 
	                                 PROP_SELECTION_MODE, 
	                                 properties[PROP_SELECTION_MODE]); 
	
}

static void
udate_widget ( BjbViewModeButton *self )
{
    GtkToolButton *tool = GTK_TOOL_BUTTON ( self ) ;
	
	if ( self->priv->selection_mode == BJB_VIEW_MODE_LIST )
		gtk_tool_button_set_icon_name(tool,"view-list-symbolic");

	else
		gtk_tool_button_set_icon_name(tool,"view-grid-symbolic");
	
}



GtkWidget *
bjb_view_mode_button_new ( BjbViewModeType mode )
{
	return g_object_new (  BJB_TYPE_VIEW_MODE_BUTTON, 
	                       "selection-mode", mode,
	                       NULL ) ;
}

BjbViewModeType
bjb_view_mode_button_get_selection_mode ( BjbViewModeButton *self )
{
	return self->priv->selection_mode ;
}

void
bjb_view_mode_button_set_selection_mode (BjbViewModeButton *self,
                                         BjbViewModeType mode )
{
	self->priv->selection_mode = mode ;
	udate_widget ( self ) ;
}