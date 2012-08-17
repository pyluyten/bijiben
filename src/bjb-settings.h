
#ifndef _BJB_SETTINGS_H_
#define _BJB_SETTINGS_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define BJB_TYPE_SETTINGS             (bjb_settings_get_type ())
#define BJB_SETTINGS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BJB_TYPE_SETTINGS, BjbSettings))
#define BJB_SETTINGS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BJB_TYPE_SETTINGS, BjbSettingsClass))
#define BJB_IS_SETTINGS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BJB_TYPE_SETTINGS))
#define BJB_IS_SETTINGS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BJB_TYPE_SETTINGS))
#define BJB_SETTINGS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BJB_TYPE_SETTINGS, BjbSettingsClass))

typedef struct _BjbSettingsClass BjbSettingsClass;
typedef struct _BjbSettings BjbSettings;

typedef struct _BjbSettingsPrivate BjbSettingsPrivate;

struct _BjbSettingsClass
{
  GObjectClass parent_class;
};

struct _BjbSettings
{
  GObject parent_instance;
  BjbSettingsPrivate *priv ;

  // Note edition settings
  gchar *font ;
  gchar *highlight_color ;

  // Other settings
  gboolean case_sensitive_search ;
};

GType bjb_settings_get_type (void) G_GNUC_CONST;

BjbSettings * initialize_settings(void);

void show_bijiben_settings_window(GtkMenuItem* item,GtkWidget *parent_window);

G_END_DECLS

#endif /* _BJB_SETTINGS_H_ */
