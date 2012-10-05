/* biji-timeout.c
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

/*
 *     Ported from gnote
 */

#include "biji-timeout.h"

/* Signal */
enum {
  BIJI_TIME_OUT,
  BIJI_TIME_SIGNALS
};

static guint biji_time_signals [BIJI_TIME_SIGNALS] = { 0 };

/* Private */
struct _BijiTimeoutPrivate
{
  guint timeout_id;
};

#define BIJI_TIMEOUT_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BIJI_TYPE_TIMEOUT, BijiTimeoutPrivate))

G_DEFINE_TYPE (BijiTimeout, biji_timeout, G_TYPE_OBJECT);

static void
biji_timeout_init (BijiTimeout *self)
{
  BijiTimeoutPrivate *priv = BIJI_TIMEOUT_GET_PRIVATE(self);
  self->priv = priv;

  self->priv->timeout_id = 0;
}

static void
biji_timeout_finalize (GObject *object)
{
  BijiTimeout *self = BIJI_TIMEOUT (object);

  biji_timeout_cancel (self);

  G_OBJECT_CLASS (biji_timeout_parent_class)->finalize (object);
}

static void
biji_timeout_class_init (BijiTimeoutClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = biji_timeout_finalize;

  biji_time_signals[BIJI_TIME_OUT] = g_signal_new ("timeout" ,
                                                  G_OBJECT_CLASS_TYPE (klass),
                                                  G_SIGNAL_RUN_LAST,
                                                  0, 
                                                  NULL, 
                                                  NULL,
                                                  g_cclosure_marshal_VOID__VOID,
                                                  G_TYPE_NONE,
                                                  0);

  g_type_class_add_private (klass, sizeof (BijiTimeoutPrivate));
}

BijiTimeout * biji_timeout_new (void)
{
  return g_object_new (BIJI_TYPE_TIMEOUT,
                       NULL);
}

static gboolean
biji_timeout_expired (BijiTimeout *self)
{
  g_signal_emit (self, biji_time_signals[BIJI_TIME_OUT], 0);
  self->priv->timeout_id = 0;
  return FALSE;
}

static gboolean
biji_timeout_callback (BijiTimeout *self)
{
  if (self)
    return biji_timeout_expired (self);

  return FALSE;
}

void
biji_timeout_cancel (BijiTimeout *self)
{
  if (self->priv->timeout_id != 0)
  {
    g_source_remove (self->priv->timeout_id);
    self->priv->timeout_id = 0;
  }
}

void
biji_timeout_reset (BijiTimeout *self, guint millis)
{
  biji_timeout_cancel (self);

  self->priv->timeout_id = g_timeout_add (
       millis, (GSourceFunc) biji_timeout_callback, self);
}
