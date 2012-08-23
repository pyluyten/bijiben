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

#include "bjb-controller.h"
#include "bjb-main-view.h"
#include "bjb-window-base.h"
#include "widgets/gd-main-view.h"

/* Pango rendering for Notes pixbuf */
#define ICON_WIDTH 200
#define ICON_HEIGHT 260
#define PANGO_WIDTH 180000
#define ICON_FONT "Purusa 10"


/* Gobject */

struct _BjbControllerPrivate
{
	BijiNoteBook  * book ;
	gchar         * needle ;
    GtkTreeModel  * model ;
	
	/*  Private  */
	
	GList         * notes_to_show ;
};



enum {
    PROP_0,
    PROP_BOOK ,
	PROP_NEEDLE ,
    PROP_MODEL ,
    NUM_PROPERTIES
};

static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };

#define BJB_CONTROLLER_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BJB_TYPE_CONTROLLER, BjbControllerPrivate))

G_DEFINE_TYPE (BjbController, bjb_controller, G_TYPE_OBJECT);


/* GObject */

static void
bjb_controller_init (BjbController *self)
{
	BjbControllerPrivate *priv  ;
	GtkListStore         *store ;

	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
	                                          BJB_TYPE_CONTROLLER, 
	                                          BjbControllerPrivate);
	priv = self->priv ;

	/* Create the columns */
    store = gtk_list_store_new (NUMBER_COLUMNS,
                                G_TYPE_STRING,     // urn
                                G_TYPE_STRING,     // uri
                                G_TYPE_STRING,     // name
                                G_TYPE_STRING,     // author
                                GDK_TYPE_PIXBUF,   // icon then note
                                G_TYPE_INT64,      // mtime
                                G_TYPE_BOOLEAN);   // state

	priv->model = GTK_TREE_MODEL(store) ;
	
}

static void
bjb_controller_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

    G_OBJECT_CLASS (bjb_controller_parent_class)->finalize (object);
}

static void
bjb_controller_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  BjbController *self = BJB_CONTROLLER (object);

  switch (property_id)
    {
    case PROP_BOOK:
      g_value_set_object (value, self->priv->book);
      break;
	case PROP_NEEDLE:
	  g_value_set_string(value, self->priv->needle);
	  break;
	case PROP_MODEL:
	  g_value_set_object(value, self->priv->model);
	  break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
bjb_controller_set_property (GObject    *object,
                             guint       property_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
  BjbController *self = BJB_CONTROLLER (object);

  switch (property_id)
    {
    case PROP_BOOK:
	  bjb_controller_set_book(self,g_value_get_object(value));
      break;
    case PROP_NEEDLE:
	  bjb_controller_set_needle(self,g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
bjb_controller_class_init (BjbControllerClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	/*GObjectClass* parent_class = G_OBJECT_CLASS (klass);*/

	g_type_class_add_private (klass, sizeof (BjbControllerPrivate));

	object_class->get_property = bjb_controller_get_property;
    object_class->set_property = bjb_controller_set_property;
	object_class->finalize = bjb_controller_finalize;

	properties[PROP_BOOK] =
    g_param_spec_object ("book",
                         "Book",
                         "The BijiNoteBook",
                         BIJI_TYPE_NOTE_BOOK,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT |
                         G_PARAM_STATIC_STRINGS);

	g_object_class_install_property (object_class, 
	                                 PROP_BOOK, 
	                                 properties[PROP_BOOK]); 


	properties[PROP_NEEDLE] =
    g_param_spec_string ("needle",
                         "Needle",
                         "String to search notes",
                         NULL,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT |
                         G_PARAM_STATIC_STRINGS);

	g_object_class_install_property (object_class, 
	                                 PROP_NEEDLE, 
	                                 properties[PROP_NEEDLE]);


	properties[PROP_MODEL] =
    g_param_spec_object ("model",
                         "Model",
                         "The GtkTreeModel",
                         GTK_TYPE_TREE_MODEL,
                         G_PARAM_READABLE  |
                         G_PARAM_STATIC_STRINGS);

	g_object_class_install_property (object_class, 
	                                 PROP_MODEL, 
	                                 properties[PROP_MODEL]); 

}

/* Implement model */

GdkPixbuf *
get_pixbuf_for_note ( BijiNoteObj *note )
{
	GdkPixbuf *ret = NULL ;
	cairo_surface_t *surface = NULL ;
	cairo_t *c;
	PangoLayout *layout;
	PangoFontDescription *desc;
	GdkRGBA *note_color;

	gchar *text = biji_note_get_raw_text(note) ;

	/* Create & Draw surface */ 
	surface = cairo_image_surface_create ( CAIRO_FORMAT_ARGB32 , 
	                                       ICON_WIDTH,
	                                       ICON_HEIGHT) ;
	c=cairo_create(surface);

	/* Background */
	cairo_rectangle(c, 0.5, 0.5, ICON_WIDTH, ICON_HEIGHT);

	note_color = biji_note_obj_get_rgba(note) ;

	// libbiji makes sure there IS a color
	if ( note_color )
	    gdk_cairo_set_source_rgba (c,note_color);
		
	cairo_fill_preserve(c);
    cairo_set_line_width (c,0.8);
	cairo_set_source_rgb(c, 0.5, 0.5, 0.5);
	cairo_stroke(c);


	/* Pango draws */
	cairo_translate(c, 10, 10);
	layout = pango_cairo_create_layout(c);

	pango_layout_set_width(layout, 180000 );
    pango_layout_set_wrap(layout,PANGO_WRAP_WORD_CHAR);
    pango_layout_set_height ( layout, 180000 ) ;
	
	pango_layout_set_text(layout,text, -1);
	desc = pango_font_description_from_string(ICON_FONT);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
	pango_cairo_update_layout(c, layout);
	pango_cairo_show_layout(c, layout);

	g_object_unref(layout);

	ret = gdk_pixbuf_get_from_surface (surface,
                                       0,0,
                                       ICON_WIDTH,ICON_HEIGHT);

	return ret ;
} 

static void
bjb_controller_add_note ( BijiNoteObj *note, BjbController *self )
{
    GtkListStore *store ;
	GtkTreeIter  iter ;

	store = GTK_LIST_STORE(self->priv->model) ;

	
    if ( biji_note_obj_is_template(note) == FALSE )
    {
        gtk_list_store_append(store,&iter);
        gtk_list_store_set(store, 
                           &iter,
                           COL_URN,      note_obj_get_path(note),
                           COL_URI,      note_obj_get_path(note),
                           COL_NAME,     biji_note_get_title(note),
                           COL_AUTHOR,   NULL,
                           COL_IMAGE,    get_pixbuf_for_note(note),
                           COL_MTIME,    biji_note_obj_get_last_change_date_sec(note),
                           COL_SELECTED, FALSE,
                           -1);
    }
}


static void
update_view (BjbController *self)
{
    GList *notes ;

	notes = self->priv->notes_to_show ;
	
    gtk_list_store_clear (GTK_LIST_STORE(self->priv->model));
	g_list_foreach (notes,(GFunc)bjb_controller_add_note,self) ;	
}

static glong
most_recent_note_first ( BijiNoteObj *a, BijiNoteObj *b)
{
    glong result = biji_note_id_get_last_change_date_sec(note_get_id(b));
    return result - biji_note_id_get_last_change_date_sec(note_get_id(a));
}

static void
sort_notes( BjbController *self)
{
    GList *notes ;

	notes = self->priv->notes_to_show ;
	notes = g_list_sort(notes,(GCompareFunc)most_recent_note_first);
	self->priv->notes_to_show = notes ;
}

static void
add_note_if_searched ( BijiNoteObj *obj , BjbController *self )
{
	gboolean add_note = FALSE ;
	BjbControllerPrivate *priv = self->priv ;

	/* Title match ? */
	if (g_strrstr(g_utf8_casefold(biji_note_get_title(obj),-1),
                  g_utf8_casefold(priv->needle,-1)))
		add_note = TRUE ;

	/* Tag match ? */
	if ( _biji_note_obj_has_tag_prefix(obj,priv->needle))
	    add_note = TRUE ;

	/* Content match? */
	if (g_strrstr(g_utf8_casefold(biji_note_get_raw_text(obj),-1),
                  g_utf8_casefold(priv->needle,-1)))
		add_note = TRUE ;


	if ( add_note )
	{
		priv->notes_to_show = g_list_append ( priv->notes_to_show, obj ) ;
	}	
}

static void
bjb_controller_apply_needle ( BjbController *self )
{
	gchar *needle = self->priv->needle ;
	
	g_list_free(self->priv->notes_to_show);
	self->priv->notes_to_show = NULL ;

	/* Show all notes */
	if ( needle == NULL || g_strcmp0 (needle,"") == 0)
	{
		self->priv->notes_to_show = biji_note_book_get_notes(self->priv->book);
	} 

	/* Test which note to show */
	else
	{
		GList *all_notes ;
		
		all_notes = biji_note_book_get_notes(self->priv->book) ;

        g_list_foreach (all_notes,
                        (GFunc) add_note_if_searched,
                        self) ;

		g_list_free(all_notes);

	}

	sort_notes (self) ;
	update_view(self);
}

static void
on_book_changed ( BjbController *self )
{
	/* refresh_notes_model (self); */
	bjb_controller_apply_needle (self);
}

static void
on_needle_changed ( BjbController *self )
{
	/*refresh_notes_model (self);*/
	bjb_controller_apply_needle (self);
}

BjbController *
bjb_controller_new ( BijiNoteBook *book , 
                     gchar *needle        )
{
    return g_object_new ( BJB_TYPE_CONTROLLER,
                          "book", book,
                          "needle", needle,
                          NULL); 
}

void
refresh_notes_model (BjbController *self)
{
	bjb_controller_apply_needle(self);
}


void
bjb_controller_set_book (BjbController *self, BijiNoteBook  *book )
{
    self->priv->book = book ;
}

void
bjb_controller_set_needle (BjbController *self, const gchar *needle )
{
	self->priv->needle = needle ;

	g_message("bjb controller set needle, %s",needle);

	on_needle_changed(self);
}

GtkTreeModel *
bjb_controller_get_model  (BjbController *self)
{
    return self->priv->model ;
}
