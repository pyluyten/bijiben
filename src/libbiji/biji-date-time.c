/* biji-date-time.c
 * Copyright (C) Pierre-Yves LUYTEN 2011 <py@luyten.fr>
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
 * with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include "biji-date-time.h"

gchar *
biji_get_time_diff_with_time (glong sec_since_epoch)
{
  GTimeVal now;
  glong    diff;

  /* Retrieve the number of days */
  g_get_current_time (&now);
  diff = (now.tv_sec - sec_since_epoch) / 86400 ;

  if (diff < 1)
    return "Today";

  if (diff < 2)
    return "Yesterday";

  if (diff < 7)
    return "This week";

  if (diff < 30)
    return "This month";

  if (diff < 365)
    return "This month";

  return "Unknown";
}
