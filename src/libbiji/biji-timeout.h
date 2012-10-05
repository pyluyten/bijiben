/* biji-timeout.h
 * Copyright (C) Pierre-Yves LUYTEN 2012 <py@luyten.fr>
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

#ifndef _BIJI_TIMEOUT_H_
#define _BIJI_TIMEOUT_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define BIJI_TYPE_TIMEOUT             (biji_timeout_get_type ())
#define BIJI_TIMEOUT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_TIMEOUT, BijiTimeout))
#define BIJI_TIMEOUT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_TIMEOUT, BijiTimeoutClass))
#define BIJI_IS_TIMEOUT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_TIMEOUT))
#define BIJI_IS_TIMEOUT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_TIMEOUT))
#define BIJI_TIMEOUT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_TIMEOUT, BijiTimeoutClass))

typedef struct _BijiTimeoutClass BijiTimeoutClass;
typedef struct _BijiTimeout BijiTimeout;
typedef struct _BijiTimeoutPrivate BijiTimeoutPrivate;


struct _BijiTimeoutClass
{
  GObjectClass parent_class;
};

struct _BijiTimeout
{
  GObject parent_instance;

  BijiTimeoutPrivate *priv; 
};

GType biji_timeout_get_type (void) G_GNUC_CONST;

BijiTimeout * biji_timeout_new (void);

void biji_timeout_reset (BijiTimeout *self, guint millisec);

void biji_timeout_cancel (BijiTimeout *self);

void biji_timeout_free (BijiTimeout *self);

G_END_DECLS

#endif /* _BIJI_TIMEOUT_H_ */
