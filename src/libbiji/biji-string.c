/* biji-string.c
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


#include "biji-string.h"

gchar *
biji_str_replace (gchar *string, gchar *as_is, gchar *to_be)
{
  gchar **array;
  gchar *result = NULL;

  if (!string)
    return NULL;

  if (!as_is)
    return g_strdup (string);

  if (!to_be)
    return g_strdup (string);

  array = g_strsplit( string, as_is, -1);

  if (array)
  {
    result = g_strjoinv (to_be, array);
    g_strfreev (array);
  }

  return result;
}

gchar * biji_str_mass_replace (gchar *string,
                               ...)
{
  va_list args;
  gchar *result = g_strdup (string);
  gchar *tmp;
  gchar *as_is = NULL;
  gchar *to_be = NULL;

  va_start (args, string);
  as_is = va_arg (args, gchar*);

  while (as_is)
  {
    to_be = va_arg (args, gchar*);

    if (to_be)
    {
      tmp = biji_str_replace (result, as_is, to_be);

      if (tmp)
      {
        g_free (result);
        result = tmp;
      }

      as_is = va_arg (args, gchar*);
    }

    else
      as_is = NULL;
  }

  va_end (args);
  return result;
}

