#ifndef _BIJI_MENU_TOOL_H
#define _BIJI_MENU_TOOL_H

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <gdk/gdk.h>
#include <cairo.h>

#define BIJIBEN_TYPE_MENU_BUTTON (bijiben_menu_button_get_type ())
#define BIJIBEN_MENU_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJIBEN_TYPE_MENU_BUTTON, BijibenMenuButton))
#define BIJIBEN_MENU_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), BIJIBEN_TYPE_MENU_BUTTON, BijibenMenuButtonClass))
#define BIJIBEN_IS_MENU_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJIBEN_TYPE_MENU_BUTTON))
#define BIJIBEN_IS_MENU_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJIBEN_TYPE_MENU_BUTTON))
#define BIJIBEN_MENU_BUTTON_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJIBEN_TYPE_MENU_BUTTON, BijibenMenuButtonClass))

GType bijiben_menu_button_get_type (void) G_GNUC_CONST;

#define BIJIBEN_MENU_BUTTON_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), BIJIBEN_TYPE_MENU_BUTTON, BijibenMenuButtonPrivate))

enum  {
	BIJIBEN_MENU_BUTTON_DUMMY_PROPERTY
};

typedef struct _BijibenMenuButton BijibenMenuButton;
typedef struct _BijibenMenuButtonClass BijibenMenuButtonClass;
typedef struct _BijibenMenuButtonPrivate BijibenMenuButtonPrivate;
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _BijibenMenuButton {
	GtkToggleButton parent_instance;
	BijibenMenuButtonPrivate * priv;
};

struct _BijibenMenuButtonClass {
	GtkToggleButtonClass parent_class;
};


void bijiben_menu_button_set_menu (BijibenMenuButton* self, GtkMenu* menu);
BijibenMenuButton* bijiben_menu_button_new (GtkWidget *image);
 
#endif /* _BIJI_MENU_TOOL_H */