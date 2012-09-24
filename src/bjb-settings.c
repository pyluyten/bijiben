
#include <gtk/gtk.h>

#include "bjb-bijiben.h"
#include "bjb-settings.h"

struct _BjbSettingsPrivate
{
  GSettings *settings ;
};

// Properties binded to gsettings.
enum
{
  PROP_0,

  // Note Editor.
  PROP_FONT,
  PROP_HIGHLIGHT_COLOR,

  // Interface / desktop integration
  PROP_CASE_SENSITIVE_SEARCH,           // Hidden setting.

  N_PROPERTIES
};

#define BJB_SETTINGS_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BJB_TYPE_SETTINGS, BjbSettingsPrivate))

G_DEFINE_TYPE (BjbSettings, bjb_settings, G_TYPE_OBJECT);

static void
bjb_settings_init (BjbSettings *object)
{
  g_message("bjb settings init");
    
  object->priv = 
  G_TYPE_INSTANCE_GET_PRIVATE(object,BJB_TYPE_SETTINGS,BjbSettingsPrivate);
}

static void
bjb_settings_finalize (GObject *object)
{
  G_OBJECT_CLASS (bjb_settings_parent_class)->finalize (object);
}

static void
bjb_settings_get_property (GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
  BjbSettings *settings = BJB_SETTINGS (object);

  switch (prop_id)
  {
    case PROP_CASE_SENSITIVE_SEARCH:
      g_value_set_boolean (value,settings->case_sensitive_search);
      break;
            
    case PROP_FONT:
      g_value_set_string (value,settings->font);
      break;

    case PROP_HIGHLIGHT_COLOR:
      g_value_set_string (value,settings->highlight_color);
      break;
                                
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
bjb_settings_set_property (GObject      *object,
			     guint         prop_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
  BjbSettings *settings = BJB_SETTINGS (object);

  switch (prop_id)
  {
    case PROP_CASE_SENSITIVE_SEARCH:
      settings->case_sensitive_search = g_value_get_boolean (value);
      break;

    case PROP_FONT:
      settings->font = g_value_dup_string(value) ; 
      break;

    case PROP_HIGHLIGHT_COLOR:
      settings->highlight_color = g_value_dup_string(value);
      break;
            
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
	}
}

static void
bjb_settings_class_init (BjbSettingsClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (BjbSettingsPrivate));

  object_class->finalize = bjb_settings_finalize;
  object_class->get_property = bjb_settings_get_property;
  object_class->set_property = bjb_settings_set_property;

  g_object_class_install_property (object_class,PROP_CASE_SENSITIVE_SEARCH,
                                   g_param_spec_boolean ("case-sensitive-search",
							                		     "Case Sensitive Search",
                                                         "Case man!",
							                              FALSE,
							                              G_PARAM_READWRITE));
    
  g_object_class_install_property (object_class,PROP_FONT,
                                   g_param_spec_string("font",
                                                       "Font",
                                                       "Font for notes",
                                                       NULL,
                                                       G_PARAM_READWRITE | 
                                                       G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,PROP_HIGHLIGHT_COLOR,
                                   g_param_spec_string("highlight-color",
                                                       "Highlight Color",
                                                       "Color text",
                                                       NULL,
                                                       G_PARAM_READWRITE | 
                                                       G_PARAM_STATIC_STRINGS));
}

// only init from bijiben

BjbSettings *
initialize_settings(void)
{
  BjbSettings *result = g_object_new (BJB_TYPE_SETTINGS,NULL) ;

  result->priv->settings= g_settings_new("org.gnome.bijiben");

  // UI settings

  g_settings_bind  (result->priv->settings, "case-sensitive-search",
                    result,"case_sensitive_search",
                    G_SETTINGS_BIND_DEFAULT);

  // Note editor settings

  g_settings_bind  (result->priv->settings, "font",
                    result,"font",
                    G_SETTINGS_BIND_DEFAULT);

  g_settings_bind  (result->priv->settings, "highlight-color",
                    result,"highlight_color",
                    G_SETTINGS_BIND_DEFAULT);
    
  return result ;
}

static void
on_font_selected(GtkFontButton *widget, BjbSettings *settings)
{
  g_object_set(settings,"font",gtk_font_button_get_font_name(widget),NULL);
}

void
show_bijiben_settings_window(GtkMenuItem* item, GtkWidget *parent_window)
{
  GtkWidget *dialog,*area,*notebook,*page,*frame,*picker;
  BjbSettings *settings = bjb_app_get_settings(g_application_get_default());

  // create dialog
  dialog = gtk_dialog_new_with_buttons("Notes tag",
	                                    GTK_WINDOW(parent_window),
	                                    GTK_DIALOG_MODAL| 
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
	                                    GTK_STOCK_OK,
	                                    GTK_RESPONSE_OK,
	                                    NULL);

  area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_container_set_border_width (GTK_CONTAINER (area), 8);
  notebook = gtk_notebook_new();

  // page 1 = Note Edition -> Font, Highlight-color
  page = gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
  picker = gtk_font_button_new_with_font (settings->font);
  g_signal_connect(picker,"font-set",
                   G_CALLBACK(on_font_selected),settings);
  gtk_box_pack_start (GTK_BOX(page),picker,FALSE,FALSE,2);
  frame = gtk_frame_new("Special Formating");
  gtk_box_pack_start (GTK_BOX(page),frame,FALSE,FALSE,2);
  picker = gtk_color_button_new (); 
  gtk_box_pack_start (GTK_BOX(page),picker,FALSE,FALSE,2);  
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page,
                           gtk_label_new("Note Editor"));

  // pack, show, run, kill
  gtk_box_pack_start (GTK_BOX (area), notebook, TRUE, TRUE,2);
  gtk_widget_show_all(dialog);
  gtk_dialog_run (GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}
