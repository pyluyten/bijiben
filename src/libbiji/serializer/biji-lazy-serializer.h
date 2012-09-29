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

/* This serializer is used for developement at first.
 * It combines XML format for metadata
 * and HTML format for note body.
 *
 * This format is NOT compatible with anything as of today.*/

#ifndef _BIJI_LAZY_SERIALIZER_H_
#define _BIJI_LAZY_SERIALIZER_H_

#include "../biji-note-obj.h"

G_BEGIN_DECLS

#define BIJI_TYPE_LAZY_SERIALIZER             (biji_lazy_serializer_get_type ())
#define BIJI_LAZY_SERIALIZER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BIJI_TYPE_LAZY_SERIALIZER, BijiLazySerializer))
#define BIJI_LAZY_SERIALIZER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BIJI_TYPE_LAZY_SERIALIZER, BijiLazySerializerClass))
#define BIJI_IS_LAZY_SERIALIZER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BIJI_TYPE_LAZY_SERIALIZER))
#define BIJI_IS_LAZY_SERIALIZER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BIJI_TYPE_LAZY_SERIALIZER))
#define BIJI_LAZY_SERIALIZER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BIJI_TYPE_LAZY_SERIALIZER, BijiLazySerializerClass))

typedef struct _BijiLazySerializerClass BijiLazySerializerClass;
typedef struct _BijiLazySerializer BijiLazySerializer;

typedef struct _BijiLazySerializerPrivate BijiLazySerializerPrivate;

struct _BijiLazySerializerClass
{
  GObjectClass parent_class;
};

struct _BijiLazySerializer
{
  GObject parent_instance;
  BijiLazySerializerPrivate *priv;
};

GType biji_lazy_serializer_get_type (void) G_GNUC_CONST;

BijiLazySerializer * biji_lazy_serializer_new (BijiNoteObj *note);

gboolean biji_lazy_serialize (BijiLazySerializer *self);

G_END_DECLS

#endif /* _BIJI_LAZY_SERIALIZER_H_ */

