/* biji-zeitgeist.h
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

#ifndef _BIJI_ZEITGEIST_H
#define _BIJI_ZEITGEIST_H

#include <zeitgeist.h>

#include "biji-note-obj.h"

void insert_zeitgeist(BijiNoteObj *note,const char *action) ;

#endif /* _BIJI_ZEITGEIST_H */
