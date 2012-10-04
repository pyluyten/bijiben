/*
 * Bijiben
 * Copyright (C) Pierre-Yves Luyten 2012 <py@luyten.fr>
 *
 * Bijiben is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * WebkitWebView is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BIJI_LAZY_DESERIALIZER_H_
#define _BIJI_LAZY_DESERIALIZER_H_

#include "../biji-note-obj.h"

G_BEGIN_DECLS

#define BIJI_TYPE_LAZY_DESERIALIZER              (biji_lazy_deserializer_get_type ())
#define BIJI_LAZY_DESERIALIZER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_LAZY_DESERIALIZER , BijiLazyDeserializer))
#define BIJI_LAZY_DESERIALIZER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_LAZY_DESERIALIZER , BijiLazyDeserializerClass))
#define BIJI_IS_LAZY_DESERIALIZER (obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_LAZY_DESERIALIZER ))
#define BIJI_IS_LAZY_DESERIALIZER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_LAZY_DESERIALIZER ))
#define BIJI_LAZY_DESERIALIZER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_LAZY_DESERIALIZER , BijiLazyDeserializerClass))

typedef struct _BijiLazyDeserializerClass BijiLazyDeserializerClass;
typedef struct _BijiLazyDeserializer BijiLazyDeserializer;

typedef struct _BijiLazyDeserializerPrivate BijiLazyDeserializerPrivate;

struct _BijiLazyDeserializerClass
{
  GObjectClass parent_class;
};

struct _BijiLazyDeserializer
{
  GObject parent_instance;
  BijiLazyDeserializerPrivate *priv;
};

GType biji_lazy_deserializer_get_type (void) G_GNUC_CONST;

gboolean biji_lazy_deserialize (BijiNoteObj *note);

G_END_DECLS

#endif /* _BIJI_LAZY_DESERIALIZER _H_ */

